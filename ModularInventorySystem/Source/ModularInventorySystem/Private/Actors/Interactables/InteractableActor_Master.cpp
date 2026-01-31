// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactables/InteractableActor_Master.h"
#include "InteractableLoadSubsystem.h"
#include "ActorFactories/ActorFactory.h"
#include "Engine/AssetManager.h"
#include "Interface/InteractionSystem/InteractorInterface.h"
#include "Kismet/GameplayStatics.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

AInteractableActor_Master::AInteractableActor_Master()
{
	InteractableInstigator = nullptr;
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bHasInventory = false;

	// Collision component
	// CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	// CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	// CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	// CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	// CollisionComponent->SetGenerateOverlapEvents(true);
	// CollisionComponent->InitSphereRadius(25.f);
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;
	
	// Optional inventory component
	if (bHasInventory)
	{
		InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	}
	//Durability Component
	DurabilityComponent = CreateDefaultSubobject<UDurabilityComponent>(TEXT("DurabilityComponent"));
	//InteractableComponent
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));
	InteractableComponent->SetupAttachment(RootComponent);
	//OutlineComponent
	OutlineComponent = CreateDefaultSubobject<UOutlineComponent>(TEXT("OutlineComponent"));
	InteractableComponent->SetupAttachment(RootComponent);
	// Visual components
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetIsReplicated(false);
	StaticMeshComponent->SetupAttachment(RootComponent);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComponent->SetIsReplicated(false);
	SkeletalMeshComponent->SetupAttachment(RootComponent);

	// Editor preview meshes
	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_PreviewMesh"));
	PreviewMesh->SetupAttachment(RootComponent);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetIsReplicated(false);
	PreviewMesh->SetupAttachment(StaticMeshComponent);
	
	PreviewSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_PreviewMesh"));
	PreviewSkeletalMesh->SetupAttachment(RootComponent);
	PreviewSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewSkeletalMesh->SetIsReplicated(false);
	PreviewSkeletalMesh->SetupAttachment(SkeletalMeshComponent);
	// ✅ DEBUG: Verify attachment
// #if WITH_EDITOR
// 	UE_LOG(LogActorFactory, Log, TEXT("🔧 CONSTRUCTOR: Root=%s, RootComponent=%s"), 
// 		Root ? TEXT("Valid") : TEXT("NULL"),
// 		RootComponent ? TEXT("Valid") : TEXT("NULL"));
// 	UE_LOG(LogActorFactory, Log, TEXT("🔧 PreviewMesh AttachParent: %s"), 
// 		PreviewMesh->GetAttachParent() ? *PreviewMesh->GetAttachParent()->GetName() : TEXT("NULL"));
// 	UE_LOG(LogActorFactory, Log, TEXT("🔧 PreviewSkeletalMesh AttachParent: %s"), 
// 		PreviewSkeletalMesh->GetAttachParent() ? *PreviewSkeletalMesh->GetAttachParent()->GetName() : TEXT("NULL"));
// #endif

}

// ============================================================================
// LIFECYCLE
// ============================================================================

void AInteractableActor_Master::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	InteractableInstigator = Cast<APlayerController>(InteractableComponent->InteractableInstigator);
	if (InteractableInstigator)
	{
		UWidgetManager* WidgetManager = UWidgetManager::Get(InteractableInstigator);
	}
}

void AInteractableActor_Master::BeginPlay()
{
	Super::BeginPlay();

	if (!InteractableComponent)
	{
		SetupFallbackInteraction();
	}

	// Set replication
	SetReplicateMovement(true);
	
	// Sync ID to PickupData
	PickupData.ID = ID;

	// Initialize debug
	DebugTag_InventoryInteraction = FGameplayTag::RequestGameplayTag(FName("Debug.InventoryInteraction"));
	DebugSubsystem = GetDebugSubsystem();
	
	// ========================================================================
	// ✅ FIX: EXPLICITLY UNLOAD RUNTIME MESHES ON BEGIN PLAY
	// ========================================================================
	// Ensures meshes start unloaded, even if set in Blueprint defaults
	// They will load when player approaches via OnPlayerNearbyInit
	// ========================================================================
	
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
		StaticMeshComponent->SetVisibility(false);
	}
	
	if (SkeletalMeshComponent)
	{
		SkeletalMeshComponent->SetSkeletalMeshAsset(nullptr);
		SkeletalMeshComponent->SetVisibility(false);
	}
	
	// Hide editor preview meshes during runtime
	if (PreviewMesh)
	{
		PreviewMesh->SetStaticMesh(nullptr);
		PreviewMesh->SetVisibility(false);
	}
	
	if (PreviewSkeletalMesh)
	{
		PreviewSkeletalMesh->SetSkeletalMeshAsset(nullptr);
		PreviewSkeletalMesh->SetVisibility(false);
	}
	
	// Ensure asset package is in unloaded state
	AssetPackage.Reset();
	bIsLoadingAssets = false;
	
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("💤 %s: Starting UNLOADED (will load on proximity)"), 
			*GetName()), false, EDebugVerbosity::Info);
	
	// ========================================================================
	// STANDARD INITIALIZATION
	// ========================================================================
	
	// Register with InteractionSubsystem
	if (UWorld* World = GetWorld())
	{
		if (UInteractionSubsystem* Sub = World->GetGameInstance()->GetSubsystem<UInteractionSubsystem>())
		{
			Sub->RegisterInteractable(this);
		}
	}
	
	// Cache basic item info without loading assets
	CacheBasicItemInfo();
	
	// Check for existing overlaps after brief delay
	FTimerHandle DelayedCheckTimer;
	GetWorld()->GetTimerManager().SetTimer(
		DelayedCheckTimer, 
		this, 
		&AInteractableActor_Master::CheckExistingOverlaps, 
		0.1f, 
		false
	);
}



void AInteractableActor_Master::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogTemp, Warning, TEXT("🏁 EndPlay called on %s (Reason: %d)"), 
		*GetName(), (int32)EndPlayReason);
	
	// ✅ CRITICAL: Clean up all widgets for this actor BEFORE calling Super::EndPlay
	// This ensures widgets are removed even if the actor is destroyed unexpectedly
	if (UWorld* World = GetWorld())
	{
		// ✅ FIX: Check if InteractableInstigator is valid before accessing it
		if (InteractableInstigator && InteractableInstigator->GetNetOwningPlayer())
		{
			FString WarningText = FString::Printf(TEXT("🧹 Cleaning up widgets of %s for Player %s"), 
				*GetName(), *InteractableInstigator->GetNetOwningPlayer()->GetName());
			UE_LOG(LogTemp, Verbose, TEXT("%s"), *WarningText);
			
			UWidgetManager* WidgetManager = UWidgetManager::Get(InteractableInstigator);
			
			if (WidgetManager)
			{
				WidgetManager->HideAllWidgets();
			}
			else
			{
				UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction, TEXT("NO widget manager found"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Verbose, TEXT("🧹 Cleaning up widgets of %s (No instigator)"), *GetName());
		}
		
		
		// Unregister from InteractionSubsystem
		if (UInteractionSubsystem* Sub = World->GetGameInstance()->GetSubsystem<UInteractionSubsystem>())
		{
			UE_LOG(LogTemp, Verbose, TEXT("   📤 Unregistering from InteractionSubsystem"));
			Sub->UnregisterInteractable(this);
		}
	}
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction, TEXT("✅ EndPlay completed"));
	Super::EndPlay(EndPlayReason);
}


void AInteractableActor_Master::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void AInteractableActor_Master::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(AInteractableActor_Master, bIsBeingHeld);
	DOREPLIFETIME(AInteractableActor_Master, HoldingActor);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

UDebugSubsystem* AInteractableActor_Master::GetDebugSubsystem()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UDebugSubsystem>();
		}
	}
	return nullptr;
}

void AInteractableActor_Master::CheckExistingOverlaps()
{
	// if (!CollisionComponent) 
	// {
	// 	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
	// 		TEXT("CollisionComponent is NULL!"), false);
	// 	return;
	// }

	// Debug collision component setup
	// UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
	// 	FString::Printf(TEXT("=== COLLISION DEBUG for %s ==="), *GetName()), false);
	// UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
	// 	FString::Printf(TEXT("Collision Enabled: %d"), (int32)CollisionComponent->GetCollisionEnabled()), false);
	// UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
	// 	FString::Printf(TEXT("Generate Overlap Events: %s"), CollisionComponent->GetGenerateOverlapEvents() ? TEXT("true") : TEXT("false")), false);
	
	// if (USphereComponent* SphereComp = Cast<USphereComponent>(CollisionComponent))
	// {
	// 	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
	// 		FString::Printf(TEXT("Sphere Radius: %f"), SphereComp->GetScaledSphereRadius()), false);
	// }
    
	// Get overlapping components
	TArray<UPrimitiveComponent*> OverlappingComponents;
	// CollisionComponent->GetOverlappingComponents(OverlappingComponents);

	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Found %d overlapping components"), OverlappingComponents.Num()), false);
	
	for (UPrimitiveComponent* Comp : OverlappingComponents)
	{
		if (Comp && Comp->GetOwner())
		{
			UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
				FString::Printf(TEXT("Overlapping Component: %s (Owner: %s, Channel: %d)"), 
					*Comp->GetName(), 
					*Comp->GetOwner()->GetName(),
					(int32)Comp->GetCollisionObjectType()), false);
		}
	}
    
	// Check overlapping actors
	TArray<AActor*> OverlappingActors;
	// CollisionComponent->GetOverlappingActors(OverlappingActors);

	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Found %d overlapping actors"), OverlappingActors.Num()), false);
	
	for (AActor* Actor : OverlappingActors)
	{
		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
				FString::Printf(TEXT("Found Pawn: %s, IsPlayerControlled: %s"), 
					*Pawn->GetName(), 
					Pawn->IsPlayerControlled() ? TEXT("true") : TEXT("false")), false);
                   
			if (IsRelevantPlayer(Pawn))
			{
				UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
					FString::Printf(TEXT("Found relevant overlapping player: %s"), *Pawn->GetName()), false);
				OnPlayerNearbyInit_Implementation(Pawn, true);
				break;
			}
		}
	}
    
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		TEXT("=== END COLLISION DEBUG ==="), false);
}

bool AInteractableActor_Master::IsRelevantPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn || !PlayerPawn->IsPlayerControlled())
	{
		return false;
	}
    
	ENetMode NetMode = GetWorld()->GetNetMode();
    
	switch (NetMode)
	{
	case NM_Standalone:
	case NM_DedicatedServer:
	case NM_ListenServer:
		return true;
            
	case NM_Client:
		return PlayerPawn->IsLocallyControlled();
            
	default:
		return PlayerPawn->IsLocallyControlled();
	}
}

// ============================================================================
// GETTERS
// ============================================================================

float AInteractableActor_Master::GetPreInteractionDistance() const
{
	if (UWorld* World = GetWorld())
	{
		if (UInteractionSubsystem* Subsystem = UInteractionSubsystem::Get(World))
		{
			return Subsystem->GetPreInteractionDistance();
		}
	}
	
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		TEXT("GetPreInteractionDistance failed, using default"), false);
	return DefaultPreInteractionDistance;
}

float AInteractableActor_Master::GetFullInteractionDistance() const
{
	if (UWorld* World = GetWorld())
	{
		if (UInteractionSubsystem* Subsystem = UInteractionSubsystem::Get(World))
		{
			return Subsystem->GetFullInteractionDistance();
		}
	}
	
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		TEXT("GetFullInteractionDistance failed, using default"), false);
	return DefaultFullInteractionDistance;
}

float AInteractableActor_Master::GetUIUpdateRate() const
{
	if (UWorld* World = GetWorld())
	{
		if (UInteractionSubsystem* Subsystem = UInteractionSubsystem::Get(World))
		{
			return Subsystem->GetUIUpdateRate();
		}
	}
	
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		TEXT("GetUIUpdateRate failed, using default"), false);
	return DefaultUIUpdateRate;
}

float AInteractableActor_Master::GetGlobalLoadDistance() const
{
	if (UWorld* World = GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (const UInteractionSubsystem* Subsystem = GameInstance->GetSubsystem<UInteractionSubsystem>())
			{
				return Subsystem->GlobalLoadingDistance_Items;
			}
		}
	}
	
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		TEXT("Failed to get global load distance, using default"), false);
	return DefaultLoadDistance;
}

void AInteractableActor_Master::LoadItemAssets()
{
	if (bIsLoadingAssets || AssetPackage.bIsLoaded)
	{
		return;
	}
    
	StartProximityAssetLoading();
}

// ============================================================================
// INTERFACE IMPLEMENTATIONS
// ============================================================================

void AInteractableActor_Master::OnInteract_Implementation(AController* InstigatorController)
{
	// ... (Existing logging and authority checks) ...

	// ✅ Authority check - Only server should process interactions
	if (!HasAuthority())
	{
		// ... (Existing client log and return) ...
		return;
	}

	// ✅ Server processes the interaction
	if (InteractableComponent)
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			FString::Printf(TEXT("✅ SERVER: Calling InteractableComponent->OnInteract() for %s"), *GetName()),
			true, EDebugVerbosity::Error);
        
		InteractableComponent->OnInteract(InstigatorController);
	}
	else
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			TEXT("❌ No InteractableComponent found!"),
			true, EDebugVerbosity::Error);
	}
    
	// ⭐ CRITICAL FIX: Destroy the actor on the server after interaction is handled.
	// This will replicate the destruction to all clients, triggering EndPlay.
	UE_LOG(LogTemp, Warning, TEXT("🗑️ SERVER: Successfully executed interaction. Destroying Actor %s."), *GetName());
	Destroy(); 
}

FItemData_Dynamic AInteractableActor_Master::GetPickupData_Implementation() const
{
	FItemData_Dynamic Data = PickupData;
	Data.ID = ID;  // Ensure ID is synced
	return Data;

}

void AInteractableActor_Master::SetPickupData_Implementation(FItemData_Dynamic& InPickupData)
{
	PickupData.ID = ID;
	PickupData.Quality = InPickupData.Quality;
	PickupData.bIsStolen = InPickupData.bIsStolen;
	PickupData.Durability = InPickupData.Durability;
	PickupData.Quantity = InPickupData.Quantity;
	PickupData.Weight = InPickupData.Weight;
}


bool AInteractableActor_Master::IsCurrentlyInteractable_Implementation()
{
	return bIsCurrentlyInteractable;
}

void AInteractableActor_Master::OnPlayerNearbyInit_Implementation(APawn* PlayerPawn, const bool bIsNearby)
{
	// Early exit if not relevant player
	if (!IsRelevantPlayer(PlayerPawn))
	{
		return;
	}

	APlayerController* L_PC = Cast<APlayerController>(PlayerPawn->GetController());
	if (!L_PC || !L_PC->IsLocalController()) return; 
		
	bIsPlayerNearby = bIsNearby;

	if (bIsNearby)
	{
		// Register with subsystem for priority management
		if (UWorld* World = GetWorld())
		{
			if (UInteractionSubsystem* Sub = World->GetGameInstance()->GetSubsystem<UInteractionSubsystem>())
			{
				Sub->RegisterNearbyItembyPawn(this, PlayerPawn);
			}
		}
		
		// Calculate distances and update UI
		float PreInteractionDistance = GetPreInteractionDistance();
		float FullInteractionDistance = GetFullInteractionDistance();
		float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), GetActorLocation());

		// Update UI based on distance
		bool bNewPreInteraction = Distance <= PreInteractionDistance;
		bool bNewFullInteraction = Distance <= FullInteractionDistance;

		if (bNewPreInteraction != bShowPreInteraction || bNewFullInteraction != bShowFullInteraction)
		{
			bShowPreInteraction = bNewPreInteraction;
			bShowFullInteraction = bNewFullInteraction;
			OnInteractionStateChanged();
		}
		
		// Load assets if not already loaded
		if (AssetPackage.bIsLoaded)
		{
			ApplySoftMeshRefs();
			UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
				FString::Printf(TEXT("Item %s: Meshes applied (already loaded)"), *GetName()), false);
		}
		else if (!bIsLoadingAssets)
		{
			StartProximityAssetLoading();
			UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
				FString::Printf(TEXT("Item %s: Started proximity loading"), *GetName()), false);
		}
	}
	else
	{
		// Player left - clear UI
		if (bShowPreInteraction || bShowFullInteraction)
		{
			bShowPreInteraction = false;
			bShowFullInteraction = false;
			OnInteractionStateChanged();
		}
		
		// ✅ ONLY FIX NEEDED: Don't unload if currently loading
		if (!bIsLoadingAssets)
		{
			UnloadAssetsAndMeshes();
		}
	}
}




void AInteractableActor_Master::SetInteractionEnabled_Implementation(bool bIsEnabled)
{
	if (bInteractionEnabled == bIsEnabled)
	{
		return;
	}
	
	bInteractionEnabled = bIsEnabled;
	
	// Update collision
	// if (CollisionComponent)
	// {
	// 	CollisionComponent->SetCollisionEnabled(bIsEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	// }
	
	// Clear UI state
	bShowPreInteraction = false;
	bShowFullInteraction = false;
	bIsPlayerNearby = false;
	OnInteractionStateChanged();
	
	// Unload assets to save memory
	UnloadAssetsAndMeshes();

	// Broadcast notification
	if (bEnableInteractionNotifications)
	{
		InteractionStateChanged.Broadcast(bIsEnabled);
	}
}

bool AInteractableActor_Master::GetInteractionEnabled_Implementation()
{
	return bInteractionEnabled;
}

void AInteractableActor_Master::SetInteractionNotifications_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (UInteractionSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UInteractionSubsystem>())
		{
			bEnableInteractionNotifications = Subsystem->bEnableGlobalInteractionNotifications;
			UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
				FString::Printf(TEXT("Interactable %s has interaction notifications enabled: %s"), 
					*GetName(), bEnableInteractionNotifications ? TEXT("true") : TEXT("false")), false);
		}
	}
}

bool AInteractableActor_Master::GetbInteractionNotificationsAllowManualHandling_Implementation()
{
	return bInteractionNotificationsAllowManualHandling;
}

void AInteractableActor_Master::OnPreInteractionEntered_Implementation(APawn* OtherPawn)
{
	if (!IsRelevantPlayer(OtherPawn))
	{
		return;
	}
	
	bShowPreInteraction = true;
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Pawn %s triggered pre-interaction on item %s"), *OtherPawn->GetName(), *GetName()), false);
	OnInteractionStateChanged();
}

void AInteractableActor_Master::OnFullInteractionEntered_Implementation(APawn* OtherPawn)
{
	if (!IsRelevantPlayer(OtherPawn))
	{
		return;
	}
	
	bShowFullInteraction = true;
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Pawn %s triggered full interaction on item %s"), *OtherPawn->GetName(), *GetName()), false);
	OnInteractionStateChanged();
}

void AInteractableActor_Master::OnPreInteractionExited_Implementation(APawn* OtherPawn)
{
	if (!IsRelevantPlayer(OtherPawn))
	{
		return;
	}
	
	bShowPreInteraction = false;
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Pawn %s exited pre-interaction on item %s"), *OtherPawn->GetName(), *GetName()), false);
	OnInteractionStateChanged();
}

void AInteractableActor_Master::OnFullInteractionExited_Implementation(APawn* OtherPawn)
{
	if (!IsRelevantPlayer(OtherPawn))
	{
		return;
	}
	
	bShowFullInteraction = false;
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Pawn %s exited full interaction on item %s"), *OtherPawn->GetName(), *GetName()), false);
	OnInteractionStateChanged();
}

void AInteractableActor_Master::LoadAssets_Implementation()
{
	LoadItemAssets();
}

void AInteractableActor_Master::UnloadAssets_Implementation()
{
	UnloadAssetsAndMeshes();
}

UActorComponent* AInteractableActor_Master::GetOutlineComponent_Implementation_Implementation()
{
	return OutlineComponent;
}


// ============================================================================
// ASSET LOADING SYSTEM
// ============================================================================

void AInteractableActor_Master::CacheBasicItemInfo()
{
	UDataTable* CurrentDataTable = GetActiveDataTable();
    
	if (!CurrentDataTable || ID.IsNone())
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			FString::Printf(TEXT("Cannot cache item info for %s - missing DataTable or ID"), *GetName()), false);
		return;
	}
    
	const FItemData* ItemData = CurrentDataTable->FindRow<FItemData>(ID, TEXT("CacheBasicItemInfo"));
	if (!ItemData)
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			FString::Printf(TEXT("ID '%s' not found for %s"), *ID.ToString(), *GetName()), false);
		return;
	}
    
	// Cache basic info and soft references (no actual loading)
	AssetPackage.ID = ItemData->ID;
	AssetPackage.DisplayName = ItemData->DisplayName;
	AssetPackage.Description = ItemData->Description;
	AssetPackage.Weight = ItemData->Weight;
	AssetPackage.MaxStackSize = ItemData->MaxStackSize;
    
	// Copy soft references
	AssetPackage.Soft_SM_Mesh = ItemData->Soft_SM_Mesh;
	AssetPackage.Soft_SK_Mesh = ItemData->Soft_SK_Mesh;
	AssetPackage.PickupMontage = ItemData->PickupMontage;
	AssetPackage.DropMontage = ItemData->DropMontage;
	AssetPackage.UseMontage = ItemData->UseMontage;
	AssetPackage.PickupSound = ItemData->PickupSound;
	AssetPackage.DropSound = ItemData->DropSound;
	AssetPackage.UseSound = ItemData->UseSound;
}

void AInteractableActor_Master::StartProximityAssetLoading()
{
	if (bIsLoadingAssets || AssetPackage.bIsLoaded)
	{
		return;
	}
    
	bIsLoadingAssets = true;
	TotalAssetsToLoad = 0;
	AssetsLoadedCount = 0;
	StreamableHandles.Empty();

	//Debug which assets are loading
	//UE_LOG(LogDebugSystem, Error, TEXT("Loading Asset: %s"), *GetName());
    
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
    
	// Helper lambda for loading assets with proximity validation
	auto LoadSoftAsset = [&](const auto& SoftObjectPtr, const FString& AssetType)
	{
		if (SoftObjectPtr.ToSoftObjectPath().IsValid())
		{
			TotalAssetsToLoad++;
			// Capture the path string for robust failure logging
			const FString AssetPathString = SoftObjectPtr.ToSoftObjectPath().GetAssetPathString();
			
			TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
				SoftObjectPtr.ToSoftObjectPath(),
				FStreamableDelegate::CreateWeakLambda(this, [this, AssetType, AssetPathString]()
				{
					if (IsValid(this) && bIsPlayerNearby)
					{
						// ✅ FIX 1: Check if the asset was actually resolved (successfully loaded)
							// This ensures the completion delegate doesn't fire and silently fail
							// because the underlying asset was invalid or missing.
							const FSoftObjectPath ResolvedPath(AssetPathString);
						if (ResolvedPath.ResolveObject())
						{
							// Success path
								OnSingleAssetLoaded(AssetType);
						}
					else if (IsValid(this) && !ResolvedPath.ResolveObject())
					{
						// ❌ FAILURE PATH - LOG THE BUG
								UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
									FString::Printf(TEXT("❌ ASSET LOAD FAILED/INVALID for %s (Type: %s). Path: %s"), 
										*GetName(), *AssetType, *AssetPathString), true, EDebugVerbosity::Error);
						// Continue the counter to avoid deadlock, but we now have visibility on the failure.
								OnSingleAssetLoaded(AssetType);
					}
						else
						{
							// Player left during loading - cancel
							AssetsLoadedCount++;
							if (AssetsLoadedCount >= TotalAssetsToLoad)
							{
								OnAssetLoadingCancelled();
							}
						}
						
					}
				})
			);
            
			// 🚩 FIX 1: Log the result of the RequestAsyncLoad call (Immediate rejection check)
			if (Handle.IsValid())
			{
				// Log the memory address of the handle as a unique identifier for the request
				// If this fires for all 99, the issue is internal to the AssetManager's delegate queue.
				UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
					FString::Printf(TEXT("✅ REQUEST SENT: %s (Handle: 0x%p, Path: %s)"), 
						*GetName(), Handle.Get(), *AssetPathString), false);

				StreamableHandles.Add(Handle);
			}
			else
			{
				// ❌ CRITICAL: This is the desired log if the request is immediately rejected (e.g., path is bad, or manager is overloaded/shut down)
				UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
					FString::Printf(TEXT("❌ CRITICAL IMMEDIATE FAILURE: RequestAsyncLoad FAILED immediately for %s (Type: %s, Path: %s)!"), 
						*GetName(), *AssetType, *AssetPathString), true, EDebugVerbosity::Error);
				// We must still increment the count to prevent a deadlock!
				AssetsLoadedCount++; 
				if (AssetsLoadedCount >= TotalAssetsToLoad)
				{
					OnAssetLoadingComplete(); // This will hit the post-load validation check (Fix 3)
				}
			}
		}
	};
    
	// Load all asset types
	LoadSoftAsset(AssetPackage.Soft_SM_Mesh, TEXT("StaticMesh"));
	LoadSoftAsset(AssetPackage.Soft_SK_Mesh, TEXT("SkeletalMesh"));
	LoadSoftAsset(AssetPackage.PickupMontage, TEXT("PickupMontage"));
	LoadSoftAsset(AssetPackage.DropMontage, TEXT("DropMontage"));
	LoadSoftAsset(AssetPackage.UseMontage, TEXT("UseMontage"));
	LoadSoftAsset(AssetPackage.PickupSound, TEXT("PickupSound"));
	LoadSoftAsset(AssetPackage.DropSound, TEXT("DropSound"));
	LoadSoftAsset(AssetPackage.UseSound, TEXT("UseSound"));
    
	if (TotalAssetsToLoad == 0)
	{
		OnAssetLoadingComplete();
	}
	else
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			FString::Printf(TEXT("Started proximity loading %d assets for %s"), TotalAssetsToLoad, *GetName()), false);
	}
}

void AInteractableActor_Master::OnSingleAssetLoaded(const FString& AssetType)
{
	AssetsLoadedCount++;
    
	if (AssetsLoadedCount >= TotalAssetsToLoad)
	{
		OnAssetLoadingComplete();
	}
}

void AInteractableActor_Master::OnAssetLoadingComplete()
{
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("=== ASSET LOADING COMPLETE for %s ==="), *GetName()), false);
    
	if (!IsValid(this))
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			TEXT("Actor is no longer valid!"), false);
		return;
	}
    
	if (!bIsPlayerNearby)
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			TEXT("Player left during loading - cancelling"), false);
		OnAssetLoadingCancelled();
		return;
	}
    
	AssetPackage.bIsLoaded = true;
	bIsLoadingAssets = false;
    
	ApplyLoadedMeshes();

	// 🚩 FIX 3: Ensure the validation failure log is clear
	bool bAnyMeshLoaded = AssetPackage.Soft_SM_Mesh.IsValid() || AssetPackage.Soft_SK_Mesh.IsValid();
    
	if (!bAnyMeshLoaded)
	{
		// All mesh loads failed or were invalid. Log a critical failure and clean up.
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			FString::Printf(TEXT("❌ CRITICAL: Post-Load Validation Failed for %s. No meshes loaded."), 
				*GetName()), true, EDebugVerbosity::Error);
        
		OnAssetLoadingCancelled(); // This call now logs via Fix 2
		return;
	}
    
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Proximity asset loading complete for %s"), *GetName()), false);
	StreamableHandles.Empty();
}

void AInteractableActor_Master::OnAssetLoadingCancelled()
{
	// 🚩 FIX 2: Add explicit logging for cancellation
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("⚠️ ASSET LOADING CANCELED for %s. Cleaning up handles."), *GetName()), 
		false, EDebugVerbosity::Warning);
	
	bIsLoadingAssets = false;
	AssetPackage.bIsLoaded = false;
    
	// Release streamable handles
	StreamableHandles.Empty();
    
	// Reset asset references
	AssetPackage.Reset();
    
	// Recache basic info
	CacheBasicItemInfo();
    
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Asset loading cancelled for %s (player left)"), *GetName()), false);
}

// ============================================================================
// MESH APPLICATION
// ============================================================================

void AInteractableActor_Master::ApplyLoadedMeshes()
{
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("=== MESH LOADING DEBUG for %s ==="), *GetName()), false);
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Actor Transform: %s"), *GetActorTransform().ToString()), false);
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("RootComponent Location: %s"), *RootComponent->GetComponentLocation().ToString()), false);
    
	if (AssetPackage.Soft_SM_Mesh.IsValid() && StaticMeshComponent)
	{
		EnsureComponentAttachment(StaticMeshComponent, TEXT("StaticMeshComponent"));        
		StaticMeshComponent->SetStaticMesh(AssetPackage.Soft_SM_Mesh.Get());
		StaticMeshComponent->SetVisibility(true);
        
		if (SkeletalMeshComponent)
		{
			SkeletalMeshComponent->SetVisibility(false);
		}
	}
	else if (AssetPackage.Soft_SK_Mesh.IsValid() && SkeletalMeshComponent)
	{
		EnsureComponentAttachment(SkeletalMeshComponent, TEXT("SkeletalMeshComponent"));
		SkeletalMeshComponent->SetSkeletalMesh(AssetPackage.Soft_SK_Mesh.Get());
		SkeletalMeshComponent->SetVisibility(true);
        
		if (StaticMeshComponent)
		{
			StaticMeshComponent->SetVisibility(false);
		}
	}
	
	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		TEXT("=== END MESH LOADING DEBUG ==="), false);
}

void AInteractableActor_Master::ApplySoftMeshRefs()
{
	if (!AssetPackage.bIsLoaded)
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			FString::Printf(TEXT("Assets not ready for %s"), *GetName()), false);
		return;
	}
    
	ApplyLoadedMeshes();
}

void AInteractableActor_Master::UnloadAssetsAndMeshes()
{
	// Safety check - log if we're force-cancelling a load
	if (bIsLoadingAssets)
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			FString::Printf(TEXT("⚠️ FORCE CANCEL loading for %s (shouldn't happen with Fix #2!)"), 
				*GetName()), 
			true, EDebugVerbosity::Error);
		
		StreamableHandles.Empty();
		bIsLoadingAssets = false;
	}
    
	// Clear mesh components
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
		StaticMeshComponent->SetVisibility(false);
	}
    
	if (SkeletalMeshComponent)
	{
		SkeletalMeshComponent->SetSkeletalMesh(nullptr);
		SkeletalMeshComponent->SetVisibility(false);
	}
    
	// Reset asset package
	AssetPackage.Reset();
    
	// Recache basic info
	CacheBasicItemInfo();
}





void AInteractableActor_Master::EnsureComponentAttachment(USceneComponent* Component, const FString& ComponentName)
{
	if (!Component || !RootComponent)
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			TEXT("Cannot ensure attachment - Component or RootComponent is null"), false);
		return;
	}
    
	if (Component->GetAttachParent() != RootComponent)
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			FString::Printf(TEXT("Fixing broken attachment for %s on actor %s"), *ComponentName, *GetName()), false);
		Component->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

// ============================================================================
// EDITOR-ONLY FUNCTIONS
// ============================================================================

#if WITH_EDITOR

void AInteractableActor_Master::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AInteractableActor_Master, ItemDataTable) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AInteractableActor_Master, ID))
	{
		RefreshEditorMesh();
	}
}

void AInteractableActor_Master::SetID_Implementation(FName NewID)
{
	ID = NewID;
}

void AInteractableActor_Master::RefreshEditorMesh()
{
	if (!PreviewMesh || !PreviewSkeletalMesh)
	{
		return;
	}

	// Clear both meshes first
	PreviewMesh->SetStaticMesh(nullptr);
	PreviewSkeletalMesh->SetSkeletalMesh(nullptr);
	PreviewMesh->SetVisibility(false);
	PreviewSkeletalMesh->SetVisibility(false);

	// Get active DataTable
	UDataTable* CurrentDataTable = GetActiveDataTable();
    
	if (!CurrentDataTable)
	{
		if (!ID.IsNone())
		{
			UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
				FString::Printf(TEXT("No DataTable available for actor %s"), *GetName()), false);
		}
		return;
	}
	
	if (ID.IsNone())
	{
		return; // Silent - empty ID is normal during setup
	}

	const FItemData* Row = CurrentDataTable->FindRow<FItemData>(ID, TEXT("PickupActor Editor Refresh"));
	if (!Row)
	{
		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
			FString::Printf(TEXT("Row %s not found in DataTable %s"), 
				*ID.ToString(), *CurrentDataTable->GetPathName()), false);
		return;
	}

	// Check if we have any mesh references
	bool bHasAnyMeshRef = Row->EditorPreviewMesh_Soft.ToSoftObjectPath().IsValid() ||
						  Row->Soft_SM_Mesh.ToSoftObjectPath().IsValid() ||
						  Row->EditorPreviewSKMesh_Soft.ToSoftObjectPath().IsValid() ||
						  Row->Soft_SK_Mesh.ToSoftObjectPath().IsValid();

	if (!bHasAnyMeshRef)
	{
		return; // No mesh references - normal for some items
	}

	// Get Asset Manager for async loading
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	
	// Try loading static mesh
	if (TryLoadStaticMeshPreview(Row, StreamableManager))
	{
		return;
	}
	
	// Fallback to skeletal mesh
	if (TryLoadSkeletalMeshPreview(Row, StreamableManager))
	{
		return;
	}

	UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
		FString::Printf(TEXT("Failed to start loading any preview mesh for ID %s"), *ID.ToString()), false);
}

UDataTable* AInteractableActor_Master::GetActiveDataTable()
{
	// Check actor's own DataTable first
	if (ItemDataTable)
	{
		return ItemDataTable;
	}
    
	// Fallback to subsystem's DataTable
	if (UInteractableLoadSubsystem* LoadSubsystem = GEngine->GetEngineSubsystem<UInteractableLoadSubsystem>())
	{
		if (UDataTable* SubsystemTable = LoadSubsystem->GetDataTable())
		{
			return SubsystemTable;
		}
	}
    
	return nullptr;
}

bool AInteractableActor_Master::TryLoadStaticMeshPreview(const FItemData* Row, FStreamableManager& StreamableManager)
{
	if (!Row)
	{
		return false;
	}

	// Try EditorPreviewMesh_Soft first
	if (Row->EditorPreviewMesh_Soft.ToSoftObjectPath().IsValid())
	{
		return LoadStaticMeshAsync(Row->EditorPreviewMesh_Soft, StreamableManager, TEXT("Editor Static"));
	}
    
	// Fallback to runtime static mesh
	if (Row->Soft_SM_Mesh.ToSoftObjectPath().IsValid())
	{
		return LoadStaticMeshAsync(Row->Soft_SM_Mesh, StreamableManager, TEXT("Runtime Static"));
	}
    
	return false;
}

bool AInteractableActor_Master::TryLoadSkeletalMeshPreview(const FItemData* Row, FStreamableManager& StreamableManager)
{
	if (!Row)
	{
		return false;
	}

	// Try EditorPreviewSKMesh_Soft first
	if (Row->EditorPreviewSKMesh_Soft.ToSoftObjectPath().IsValid())
	{
		return LoadSkeletalMeshAsync(Row->EditorPreviewSKMesh_Soft, StreamableManager, TEXT("Editor Skeletal"));
	}
    
	// Fallback to runtime skeletal mesh
	if (Row->Soft_SK_Mesh.ToSoftObjectPath().IsValid())
	{
		return LoadSkeletalMeshAsync(Row->Soft_SK_Mesh, StreamableManager, TEXT("Runtime Skeletal"));
	}
    
	return false;
}

bool AInteractableActor_Master::LoadStaticMeshAsync(
	const TSoftObjectPtr<UStaticMesh>& MeshRef, 
	FStreamableManager& StreamableManager, 
	const FString& MeshType)
{
	// Check if already loaded
	if (MeshRef.IsValid())
	{
		PreviewMesh->SetStaticMesh(MeshRef.Get());
		PreviewMesh->SetVisibility(true);
		return true;
	}
    
	const FSoftObjectPath& ObjectPath = MeshRef.ToSoftObjectPath();
    
	// Check if asset finished loading
	if (StreamableManager.IsAsyncLoadComplete(ObjectPath))
	{
		if (UStaticMesh* LoadedMesh = MeshRef.Get())
		{
			PreviewMesh->SetStaticMesh(LoadedMesh);
			PreviewMesh->SetVisibility(true);
		}
		return true;
	}
    
	// Start async load
	StreamableManager.RequestAsyncLoad(ObjectPath, 
		FStreamableDelegate::CreateWeakLambda(this, [this, ID = ID, MeshType]()
		{
			if (!IsValid(this) || this->ID != ID)
			{
				return;
			}

			UDataTable* CurrentDataTable = GetActiveDataTable();
			if (!CurrentDataTable)
			{
				return;
			}

			const FItemData* CurrentRow = CurrentDataTable->FindRow<FItemData>(ID, TEXT(""));
			if (!CurrentRow)
			{
				return;
			}

			UStaticMesh* LoadedMesh = nullptr;
			if (MeshType.Contains(TEXT("Editor")) && CurrentRow->EditorPreviewMesh_Soft.IsValid())
			{
				LoadedMesh = CurrentRow->EditorPreviewMesh_Soft.Get();
			}
			else if (MeshType.Contains(TEXT("Runtime")) && CurrentRow->Soft_SM_Mesh.IsValid())
			{
				LoadedMesh = CurrentRow->Soft_SM_Mesh.Get();
			}
			
			if (LoadedMesh && PreviewMesh)
			{
				PreviewMesh->SetStaticMesh(LoadedMesh);
				PreviewMesh->SetVisibility(true);
			}
		})
	);
    
	return true;
}

bool AInteractableActor_Master::LoadSkeletalMeshAsync(
	const TSoftObjectPtr<USkeletalMesh>& MeshRef, 
	FStreamableManager& StreamableManager, 
	const FString& MeshType)
{
	// Check if already loaded
	if (MeshRef.IsValid())
	{
		PreviewSkeletalMesh->SetSkeletalMesh(MeshRef.Get());
		PreviewSkeletalMesh->SetVisibility(true);
		return true;
	}
    
	const FSoftObjectPath& ObjectPath = MeshRef.ToSoftObjectPath();
    
	// Check if asset finished loading
	if (StreamableManager.IsAsyncLoadComplete(ObjectPath))
	{
		if (USkeletalMesh* LoadedMesh = MeshRef.Get())
		{
			PreviewSkeletalMesh->SetSkeletalMesh(LoadedMesh);
			PreviewSkeletalMesh->SetVisibility(true);
		}
		return true;
	}
    
	// Start async load
	StreamableManager.RequestAsyncLoad(ObjectPath, 
		FStreamableDelegate::CreateWeakLambda(this, [this, ID = ID, MeshType]()
		{
			if (!IsValid(this) || this->ID != ID)
			{
				return;
			}

			UDataTable* CurrentDataTable = GetActiveDataTable();
			if (!CurrentDataTable)
			{
				return;
			}

			const FItemData* CurrentRow = CurrentDataTable->FindRow<FItemData>(ID, TEXT(""));
			if (!CurrentRow)
			{
				return;
			}

			USkeletalMesh* LoadedMesh = nullptr;
			if (MeshType.Contains(TEXT("Editor")) && CurrentRow->EditorPreviewSKMesh_Soft.IsValid())
			{
				LoadedMesh = CurrentRow->EditorPreviewSKMesh_Soft.Get();
			}
			else if (MeshType.Contains(TEXT("Runtime")) && CurrentRow->Soft_SK_Mesh.IsValid())
			{
				LoadedMesh = CurrentRow->Soft_SK_Mesh.Get();
			}
			
			if (LoadedMesh && PreviewSkeletalMesh)
			{
				PreviewSkeletalMesh->SetSkeletalMesh(LoadedMesh);
				PreviewSkeletalMesh->SetVisibility(true);
			}
		})
	);
    
	return true;
}

#endif // WITH_EDITOR

void AInteractableActor_Master::SetOutlineActive_Implementation_Implementation(bool bActivate)
{
	OutlineComponent->SetOutlineActive(bActivate);
}

void AInteractableActor_Master::OnInteractPredicted_Implementation(AController* InstigatorController)
{
    UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
        FString::Printf(TEXT("🔮 PREDICTED interaction with %s"), *GetName()),
        false, EDebugVerbosity::Info);
    
    // ========================================================================
    // PLAY CLIENT-SIDE EFFECTS ONLY (NO STATE CHANGES!)
    // ========================================================================
    
    // 1. Play animation (if you have one)
    // Example: if (InteractionMontage)
    // {
    //     if (USkeletalMeshComponent* Mesh = GetSkeletalMeshComponent())
    //     {
    //         if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
    //         {
    //             AnimInstance->Montage_Play(InteractionMontage);
    //         }
    //     }
    // }
    
    // 2. Play sound (local only - won't replicate)
    // Example: if (InteractionSound)
    // {
    //     UGameplayStatics::PlaySoundAtLocation(this, InteractionSound, GetActorLocation());
    // }
    
    // 3. Play particle effects (local only)
    // Example: if (InteractionParticle)
    // {
    //     UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), InteractionParticle, GetActorLocation());
    // }
    
    // 4. Update UI cosmetically (show preview, etc.)
    // NOTE: Don't actually add items to inventory - server will do that!
    // Just show a preview/indication that item will be picked up
    
    UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
        TEXT("✅ Predicted effects played"),
        false, EDebugVerbosity::Info);
}

void AInteractableActor_Master::CachePredictionState_Implementation(FInteractableState& OutState)
{
    // Cache any custom state you need to rollback
    // This is called BEFORE prediction executes
    
    // Example: Cache interactable flags
    OutState.CustomData.Add("bIsCurrentlyInteractable", 
        bIsCurrentlyInteractable ? TEXT("true") : TEXT("false"));
    
    OutState.CustomData.Add("bInteractionEnabled", 
        bInteractionEnabled ? TEXT("true") : TEXT("false"));
    
    // Example: Cache item data (if this is a pickup)
    if (ID != NAME_None)
    {
        OutState.CustomData.Add("ID", ID.ToString());
    }
    
    // Add any other state you need to restore on rollback
}

void AInteractableActor_Master::RestorePredictionState_Implementation(const FInteractableState& State)
{
    // Restore custom state on rollback
    // This is called if server rejects the prediction
    
    // Example: Restore interactable flags
    if (const FString* bInteractableStr = State.CustomData.Find("bIsCurrentlyInteractable"))
    {
        bIsCurrentlyInteractable = (*bInteractableStr == TEXT("true"));
    }
    
    if (const FString* bEnabledStr = State.CustomData.Find("bInteractionEnabled"))
    {
        bInteractionEnabled = (*bEnabledStr == TEXT("true"));
    }
    
    // Reverse any visual changes (if you did any in OnInteractPredicted)
    // Example: Stop animations, hide preview UI, etc.
}




UActorComponent* AInteractableActor_Master::GetInventoryComponentAsActorComponent_Implementation()
{
	if (bHasInventory && InventoryComponent)
	{
		return InventoryComponent;
	}
	return nullptr;
}

UActorComponent* AInteractableActor_Master::GetDurabilityComponentAsActorComponent_Implementation()
{
	return DurabilityComponent;
}

void AInteractableActor_Master::SetupFallbackInteraction()
{
	bUsingFallbackInteraction = true;
    
	// Create fallback sphere
	FallbackInteractionSphere = NewObject<USphereComponent>(this, TEXT("FallbackInteractionSphere"));
	if (FallbackInteractionSphere)
	{
		FallbackInteractionSphere->SetupAttachment(RootComponent);
		FallbackInteractionSphere->InitSphereRadius(FallbackInteractionRadius);
		FallbackInteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		FallbackInteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		FallbackInteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		FallbackInteractionSphere->SetGenerateOverlapEvents(true);
		FallbackInteractionSphere->RegisterComponent();
        
		FallbackInteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AInteractableActor_Master::OnFallbackOverlapBegin);
		FallbackInteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AInteractableActor_Master::OnFallbackOverlapEnd);
        
		UE_LOG(LogTemp, Log, TEXT("%s: Using fallback interaction (no InteractableComponent)"), *GetName());
	}
}

void AInteractableActor_Master::OnFallbackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bUsingFallbackInteraction || !bInteractionEnabled) return;
    
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;

	// Track that player is nearby and can interact
	InteractableInstigator = Cast<APlayerController>(Pawn->GetController());
	bIsPlayerNearby = true;

	// Auto-interact on overlap (simple pickup behavior)
	AController* Controller = Pawn->GetController();
	if (Controller)
	{
		OnInteract_Implementation(Controller);
	}
}

void AInteractableActor_Master::OnFallbackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bUsingFallbackInteraction) return;
    
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;
    
	// Clear instigator when player leaves
	if (InteractableInstigator && InteractableInstigator == Pawn->GetController())
	{
		InteractableInstigator = nullptr;
	}
    
	bIsPlayerNearby = false;
}


// === INTERFACE IMPLEMENTATIONS ===

bool AInteractableActor_Master::CanBeGrabbed_Implementation() const
{
	return bCanBeGrabbed && !bIsBeingHeld;
}

void AInteractableActor_Master::OnGrabbed_Implementation(AActor* GrabbingActor)
{
	if (!HasAuthority())
	{
		Server_Grab(GrabbingActor);
		return;
	}
    
	Internal_Grab(GrabbingActor);
}

void AInteractableActor_Master::OnReleased_Implementation(AActor* ReleasingActor)
{
	if (!HasAuthority())
	{
		Server_Release(ReleasingActor);
		return;
	}
    
	Internal_Release(ReleasingActor);
}

FVector AInteractableActor_Master::GetGrabOffset_Implementation() const
{
	return GrabOffset;
}

UPrimitiveComponent* AInteractableActor_Master::GetGrabComponent_Implementation() const
{
	return StaticMeshComponent ? Cast<UPrimitiveComponent>(StaticMeshComponent) 
							   : Cast<UPrimitiveComponent>(SkeletalMeshComponent);
}

float AInteractableActor_Master::GetGrabMass_Implementation() const
{
	UPrimitiveComponent* Comp = GetGrabComponent_Implementation();
	if (Comp && Comp->IsSimulatingPhysics())
	{
		return Comp->GetMass();
	}
	return 1.0f;
}

bool AInteractableActor_Master::IsBeingHeld_Implementation() const
{
	return bIsBeingHeld;
}

bool AInteractableActor_Master::Server_Grab_Validate(AActor* GrabbingActor)
{
	return GrabbingActor != nullptr && bCanBeGrabbed && !bIsBeingHeld;
}

void AInteractableActor_Master::Server_Grab_Implementation(AActor* GrabbingActor)
{
	Internal_Grab(GrabbingActor);
}

bool AInteractableActor_Master::Server_Release_Validate(AActor* ReleasingActor)
{
	return bIsBeingHeld && HoldingActor.Get() == ReleasingActor;
}

void AInteractableActor_Master::Server_Release_Implementation(AActor* ReleasingActor)
{
	Internal_Release(ReleasingActor);
}

// === INTERNALS ===

bool AInteractableActor_Master::Internal_Grab(AActor* GrabbingActor)
{
	if (!bCanBeGrabbed || bIsBeingHeld) return false;
    
	bIsBeingHeld = true;
	HoldingActor = GrabbingActor;
    
	SetInteractionEnabled_Implementation(false);
    
	return true;
}

bool AInteractableActor_Master::Internal_Release(AActor* ReleasingActor)
{
	if (!bIsBeingHeld) return false;
    
	bIsBeingHeld = false;
	HoldingActor = nullptr;
    
	SetInteractionEnabled_Implementation(true);
    
	return true;
}

// === ONREPS ===

void AInteractableActor_Master::OnRep_IsBeingHeld()
{
	// Update interaction state on clients
	SetInteractionEnabled_Implementation(!bIsBeingHeld);
}

void AInteractableActor_Master::OnRep_HoldingActor()
{
	// Visual feedback if needed
}
