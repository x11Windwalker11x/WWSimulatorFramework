// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/MPC_Pawn_Master.h"

// Sets default values
AMPC_Pawn_Master::AMPC_Pawn_Master()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// vars:
		//true by default...
	bCanInteract = true;
	//Components
	PawnComponentManager = CreateDefaultSubobject<UPawnComponentManager>(TEXT("PawnComponentManager"));
	InRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SK_GFX = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_GFX"));
	SM_GFX = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_GFX"));
	InputActionMechanicComponent = CreateDefaultSubobject<UInputActionMechanicComponent>(TEXT("InputActionMechanic"));
	InventoryCompopent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("InteractorComponent"));
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	OwnerPlayerController = nullptr;
	OwnerAIController = nullptr;

	//SetupAttachments
	SetRootComponent(InRootComponent);
	SM_GFX->SetupAttachment(RootComponent);
	SK_GFX->SetupAttachment(RootComponent);
	
	CameraBoom->SetupAttachment(RootComponent);
	CameraComponent->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	

	//Setup TPS

	CameraBoom->bUsePawnControlRotation = true;
	CameraComponent->bUsePawnControlRotation = false;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

// Called when the game starts or when spawned
void AMPC_Pawn_Master::BeginPlay()
{
	Super::BeginPlay();
	Tags.Add("Interactor");
	
}

// Called every frame
void AMPC_Pawn_Master::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMPC_Pawn_Master::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMPC_Pawn_Master::OnJumpActionPressed_Implementation()
{
	//HandBrake implementation here
	UE_LOG(LogTemp, Log, TEXT("Jump Action Pressed"));
}

UActorComponent* AMPC_Pawn_Master::GetInventoryComponentActorComponent_Implementation(
	TSubclassOf<UActorComponent> ComponentClass)
{
	return InventoryCompopent;
}

UInventoryComponent* AMPC_Pawn_Master::GetInventoryComponent_Implementation()
{
	return InventoryCompopent;
}

UActorComponent* AMPC_Pawn_Master::GetInteractorComponent_Implementation()
{
	return InteractorComponent;
}

UInteractableComponent* AMPC_Pawn_Master::GetInteractableComponent_Implementation()
{
	return InteractableComponent;
}

UActorComponent* AMPC_Pawn_Master::GetInteractableComponentAsActorComponent_Implementation(
	TSubclassOf<UActorComponent> ComponentClass)
{
	return InteractableComponent;
}

bool AMPC_Pawn_Master::GetbCanInteract_Implementation()
{
	return bCanInteract;
}

void AMPC_Pawn_Master::OnCameraToggle_Implementation(APawn* ControlledPawn)
{
	if (!ControlledPawn || !PawnComponentManager) return;

	if (AMPC_PlayerCameraManager* CamManager = PawnComponentManager->GetOwnerCameraManager())
	{
		CamManager->ToggleCamera(ControlledPawn);
	}
}

void AMPC_Pawn_Master::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
    
	if (PawnComponentManager)
	{
		PawnComponentManager->OnPawnPossessed(NewController);
	}


}


UClass* AMPC_Pawn_Master::GetInteractorClass_Implementation()
{
	return GetClass();
}

AController* AMPC_Pawn_Master::GetInteractorController_Implementation()
{
	return GetController();
}

APlayerController* AMPC_Pawn_Master::GetInteractorPlayerController_Implementation()
{
	return GetController<APlayerController>();
}

APawn* AMPC_Pawn_Master::GetInteractorPawn_Implementation()
{
	return this;
}



UActorComponent* AMPC_Pawn_Master::GetInteractorComponentAsActorComponent_Implementation(
	TSubclassOf<UActorComponent> ComponentClass)
{
	return InteractorComponent;
}

// ========================================
// IInteractableInterface Implementations
// ========================================

void AMPC_Pawn_Master::OnInteract_Implementation(AController* InstigatorController)
{
	// Delegate to InteractableComponent if available
	if (InteractableComponent)
	{
		InteractableComponent->OnInteract(this);
	}
}

void AMPC_Pawn_Master::OnPreInteractionEntered_Implementation(APawn* OtherPawn)
{
	// Delegate to InteractableComponent if available
	if (InteractorComponent)
	{
		// InteractorComponent->Preinter(OtherPawn);
	}
}

void AMPC_Pawn_Master::OnFullInteractionEntered_Implementation(APawn* OtherPawn)
{
	// Delegate to InteractableComponent if available
	if (InteractableComponent)
	{
		// InteractableComponent->;
		//No function to deletegate for the comp handles internally
	}
}

void AMPC_Pawn_Master::OnPreInteractionExited_Implementation(APawn* OtherPawn)
{
	// Delegate to InteractableComponent if available
	if (InteractableComponent)
	{
		// InteractableComponent->OnPreInteractionExited(OtherPawn);
		//No function to deletegate for the comp handles internally

	}
}

void AMPC_Pawn_Master::OnFullInteractionExited_Implementation(APawn* OtherPawn)
{
	// Delegate to InteractableComponent if available
	if (InteractableComponent)
	{
		// InteractableComponent->OnFullInteractionExited(OtherPawn);
		//No function to deletegate for the comp handles internally

	}
	
}

bool AMPC_Pawn_Master::IsCurrentlyInteractable_Implementation()
{
	// Delegate to InteractableComponent if available
	if (InteractableComponent)
	{
		//return InteractableComponent->IsCurrentlyInteractable();
		//TODO: Make Is CUrrently interactoble in interactable component 
	}
	return bCanInteract;
}

void AMPC_Pawn_Master::SetInteractionEnabled_Implementation(bool bIsEnabled)
{
	bCanInteract = bIsEnabled;
	
	// Delegate to InteractableComponent if available
	if (InteractableComponent)
	{
		//InteractableComponent->SetInteractionEnabled(bIsEnabled);
	}
}

bool AMPC_Pawn_Master::GetInteractionEnabled_Implementation()
{
	// Delegate to InteractableComponent if available
	if (InteractableComponent)
	{
		//return InteractableComponent->GetInteractionEnabled();
	}
	return bCanInteract;
}

void AMPC_Pawn_Master::SetInteractionNotifications_Implementation()
{
	// if (UWorld* World = GetWorld())
	// {
	// 	if (UInteractionSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UInteractionSubsystem>())
	// 	{
	// 		bEnableInteractionNotifications = Subsystem->bEnableGlobalInteractionNotifications;
	// 		UDebugSubsystem::PrintDebug(this, DebugTag_InventoryInteraction,
	// 			FString::Printf(TEXT("Interactable %s has interaction notifications enabled: %s"), 
	// 				*GetName(), bEnableInteractionNotifications ? TEXT("true") : TEXT("false")), false);
	// 	}
	// }
}

bool AMPC_Pawn_Master::GetbInteractionNotificationsAllowManualHandling_Implementation()
{
	// Delegate to InteractableComponent if available
	// if (InteractableComponent)
	// {
	// 	return InteractableComponent->GetbInteractionNotificationsAllowManualHandling();
	// }
	return false;
}

void AMPC_Pawn_Master::LoadAssets_Implementation()
{
	// Delegate to InteractableComponent if available
	// if (InteractableComponent)
	// {
	// 	InteractableComponent->LoadAssets();
	// }
}

void AMPC_Pawn_Master::UnloadAssets_Implementation()
{
	// Delegate to InteractableComponent if available
	// if (InteractableComponent)
	// {
	// 	InteractableComponent->UnloadAssets();
	// }
}

FGameplayTagContainer AMPC_Pawn_Master::GetGameplayTags_Implementation()
{
	// Delegate to InteractableComponent if available
	// if (InteractableComponent)
	// {
	// 	return InteractableComponent->GetGameplayTags();
	// }
	
	// Return empty container if no component
	return FGameplayTagContainer();
}

void AMPC_Pawn_Master::OnPlayerNearbyInit_Implementation(APawn* PlayerPawn, const bool bIsNearby)
{
	// Delegate to InteractableComponent if available
	// if (InteractableComponent)
	// {
	// 	InteractableComponent->OnPlayerNearbyInit(PlayerPawn, bIsNearby);
	// }
}

void AMPC_Pawn_Master::SavePawnState(const FString& PlayerID)
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>())
            {
                // Save Character Transform
                if (UCharacterSaveModule* CharModule = SaveSubsystem->GetCharacterModule())
                {
                    CharModule->SaveCharacterTransform(PlayerID, GetActorTransform(), World->GetMapName());
                    
                    FCharacterStatsData Stats;
                    // You'd populate stats from your pawn here
                    CharModule->SaveCharacterStats(PlayerID, Stats);
                }
                
                // Save Inventory
                if (UInventorySaveModule* InvModule = SaveSubsystem->GetInventoryModule())
                {
                    if (InventoryCompopent)
                    {
                        FPlayerInventoryData& InvData = InvModule->GetOrCreatePlayerInventory(PlayerID);
                        // Clear existing
                        InvData.InventoryItems.Empty();
                        
                        // Save items from inventory component
                        // You'd iterate your inventory items here
                        // Example:
                        // for (const auto& Item : InventoryCompopent->Items)
                        // {
                        //     FItemSaveData ItemSave(Item.ItemID, Item.Quantity, Item.SlotIndex);
                        //     InvData.InventoryItems.Add(ItemSave);
                        // }
                    }
                }
                
                // Trigger save
                SaveSubsystem->QuickSave();
                
                UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Pawn(), 
                    FString::Printf(TEXT("Saved pawn state for player: %s"), *PlayerID), false);
            }
        }
    }
}

void AMPC_Pawn_Master::LoadPawnState(const FString& PlayerID)
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>())
            {
                // Load Character Transform
                if (UCharacterSaveModule* CharModule = SaveSubsystem->GetCharacterModule())
                {
                    FTransform LoadedTransform;
                    FString LoadedLevel;
                    
                    if (CharModule->LoadCharacterTransform(PlayerID, LoadedTransform, LoadedLevel))
                    {
                        SetActorTransform(LoadedTransform);
                        
                        FCharacterStatsData LoadedStats;
                        CharModule->LoadCharacterStats(PlayerID, LoadedStats);
                        // Apply stats to your pawn here
                    }
                }
                
                // Load Inventory
                if (UInventorySaveModule* InvModule = SaveSubsystem->GetInventoryModule())
                {
                    if (InventoryCompopent && InvModule->HasPlayerData(PlayerID))
                    {
                        FPlayerInventoryData& InvData = InvModule->GetOrCreatePlayerInventory(PlayerID);
                        
                        // Load items into inventory component
                        // Example:
                        // InventoryCompopent->ClearInventory();
                        // for (const FItemSaveData& SavedItem : InvData.InventoryItems)
                        // {
                        //     InventoryCompopent->AddItem(SavedItem.ItemID, SavedItem.Quantity);
                        // }
                    }
                }
                
                UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Pawn(), 
                    FString::Printf(TEXT("Loaded pawn state for player: %s"), *PlayerID), false);
            }
        }
    }
}