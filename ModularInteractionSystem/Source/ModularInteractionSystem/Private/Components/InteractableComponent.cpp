// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InteractableComponent.h"

#include "Components/WidgetComponent.h"
#include "Subsystems/AdvancedWidgetFramework/WidgetManagerBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h" // Needed for GetWorld() checks

// ============================================================================
// SPATIAL HASH INTEGRATION - NEW INCLUDES
// ============================================================================
#include <gsl/pointers>

#include "InteractionSubsystem.h"
#include "../../../../../ModularInventorySystem/Source/ModularInventorySystem/Public/Components/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/InterpToMovementComponent.h"
#include "Interfaces/ModularInteractionSystem/InteractableInterface.h"
#include "Interfaces/ModularInventorySystem/InventoryInterface.h"
#include "Interfaces/SimulatorFramework/DurabilityInterface.h"
#include "Net/UnrealNetwork.h"

// ============================================================================
// CONSTRUCTION
// ============================================================================


UInteractableComponent::UInteractableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;  // Will be enabled only if movement tracking needed
    
    // 🔥 SIMPLEST CONSTRUCTOR: Set ONLY raw defaults. Remove ALL tag assignments.
    InteractableTypeTag = FGameplayTag::RequestGameplayTag(FName("Interactable"));
    InitSphereRadius(InteractionRadius);
    ShapeColor = FColor::Yellow; // Fallback default
    SetHiddenInGame(false);
    
    // ... (collision setup remains here)
    BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BodyInstance.SetObjectType(ECC_WorldDynamic);
    BodyInstance.SetResponseToAllChannels(ECR_Ignore);
    BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Overlap);
    SetGenerateOverlapEvents(true);
    
    // ========================================================================
    // SPATIAL HASH MOVEMENT TRACKING - NEW
    // ========================================================================
    LastKnownLocation = FVector::ZeroVector;
    MovementCheckTimeAccumulator = 0.f;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UInteractableComponent::PostInitProperties()
{
   Super::PostInitProperties();
    
   // This runs after construction but before BeginPlay
   // Tags are loaded, and we can access the CDO-based GetDebugSettingsAsset
   if (!HasAnyFlags(RF_ClassDefaultObject))
   {
      DebugTag_InventoryInteraction = WWDebugTags::Debug_InventoryInteraction();
      DebugTag_Interaction = WWDebugTags::Debug_Interaction();
      SetupInitialShapeColor();
   }
}


void UInteractableComponent::BeginPlay()
{
   Super::BeginPlay();
    
   UE_LOG(LogDebugSystem, Warning, TEXT("🔥 BeginPlay START for %s"), *GetName());
    
   FGameplayTag ItemTag = FGameplayTag::RequestGameplayTag(FName("Interactable.Item"), false);
   UDebugSettings* DebugSettings = UDebugSubsystem::GetDebugSettingsAsset();
    
   if (DebugSettings)
   {
      UE_LOG(LogDebugSystem, Warning, TEXT("✅ DebugSettings LOADED in BeginPlay"));
        
      if (InteractableTypeTag.MatchesTag(ItemTag))
      {
         LayerSettings = DebugSettings->GetLayerSettings(DebugTag_InventoryInteraction);
         UE_LOG(LogDebugSystem, Warning, TEXT("🔵 Using InventoryInteraction tag"));
      }
      else
      {
         LayerSettings = DebugSettings->GetLayerSettings(DebugTag_Interaction);
         UE_LOG(LogDebugSystem, Warning, TEXT("🟡 Using Interaction tag"));
      }
        
      if (LayerSettings)
      {
         UE_LOG(LogDebugSystem, Warning, TEXT("✅ LayerSettings VALID: Color R=%d G=%d B=%d, bHidden=%s"), 
             LayerSettings->TraceColor.R, 
             LayerSettings->TraceColor.G, 
             LayerSettings->TraceColor.B,
             LayerSettings->bShowCollisionShapes ? TEXT("FALSE") : TEXT("TRUE"));
            
         bHiddenInGame = !LayerSettings->bShowCollisionShapes;
         ShapeColor = LayerSettings->TraceColor;
      }
      else
      {
         UE_LOG(LogDebugSystem, Error, TEXT("❌ LayerSettings NULL!"));
         bHiddenInGame = false;
         ShapeColor = FColor::Yellow;
      }
   }
   else
   {
      UE_LOG(LogDebugSystem, Error, TEXT("❌ DebugSettings NULL in BeginPlay!"));
   }
    
   UE_LOG(LogDebugSystem, Warning, TEXT("🔥 BeginPlay END: FinalColor R=%d G=%d B=%d, bHiddenInGame=%s"), 
       ShapeColor.R, ShapeColor.G, ShapeColor.B, bHiddenInGame ? TEXT("TRUE") : TEXT("FALSE"));

   // if (InteractableTypeTag.MatchesTag(ItemTag))
   // {
   //    bHiddenInGame = !LayerSettings->bShowCollisionShapes;
   //    ShapeColor = LayerSettings->TraceColor;
   //    UDebugSubsystem::PrintDebug(this, DebugTag, TEXT("Interactable Component: SetupCollisionVisualization complete in beginplay."));
   // }
   // else
   // {
   //    bHiddenInGame = !LayerSettings->bShowCollisionShapes;
   //    ShapeColor = LayerSettings->TraceColor;
   //    UDebugSubsystem::PrintDebug(this, DebugTag, TEXT("Interactable Component: SetupCollisionVisualization complete in beginplay."));
   //
   // }

   // Bind overlap events
   OnComponentBeginOverlap.AddDynamic(this, &UInteractableComponent::OnOverlapBegin);
   OnComponentEndOverlap.AddDynamic(this, &UInteractableComponent::OnOverlapEnd);
	
   // Update sphere radius from config
   SetSphereRadius(InteractionRadius);
	
   // Register the prompt widget with WidgetManager if configured
   if (InteractionPromptClass)
   {
      UWidgetManagerBase* WidgetManager = GetWidgetManager();
      if (WidgetManager)
      {
         // Register as pooled widget (PoolSize=1, not allowing multiple)
         if (!WidgetManager->IsWidgetRegistered(InteractionPromptClass))
         {
            WidgetManager->RegisterWidget(
               InteractionPromptClass,
               1,              // PoolSize - single instance
               PromptZOrder,   // ZOrder
               false,          // bAutoShow - don't show on registration
               false           // bAllowMultiple - only one at a time
            );
         }
      }
   }
	
   // IMPORTANT: InteractableInstigator is null by default (initialized in header)
   // It will be set in OnOverlapBegin
   
   // ========================================================================
   // SPATIAL HASH MOVEMENT TRACKING - NEW
   // ========================================================================
   
   // Store initial location and register with spatial grid
   if (AActor* Owner = GetOwner())
   {
      LastKnownLocation = Owner->GetActorLocation();
      
      // Register actor with spatial hash grid
      if (UWorld* World = GetWorld())
      {
         if (UInteractionSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UInteractionSubsystem>())
         {
            Subsystem->RegisterInteractable(Owner);
         }
      }
      
      // Auto-detect movement if not manually configured
      if (!bTrackMovementForSpatialHash)
      {
         AutoDetectMovement();
      }
      
      // Enable tick only if movement tracking is needed
      if (bTrackMovementForSpatialHash)
      {
         PrimaryComponentTick.bCanEverTick = true;
         SetComponentTickEnabled(true);
      }
   }
}

// ============================================================================
// DEBUG VISUALIZATION (Simplified Logic)
// ============================================================================

void UInteractableComponent::SetupInitialShapeColor()
{
   UDebugSettings* DebugSettings = UDebugSubsystem::GetDebugSettingsAsset();

   // ✅ PROOF IT'S LOADED
   if (DebugSettings)
   {
      FString ProofMsg = FString::Printf(TEXT("✅ DebugSettings LOADED! Has %d layers"), DebugSettings->DebugLayerSettings.Num());
      UE_LOG(LogDebugSystem, Warning, TEXT("%s"), *ProofMsg);
        
      FGameplayTag ItemTag = FGameplayTag::RequestGameplayTag(FName("Interactable.Item"), false);
        
      if (InteractableTypeTag.MatchesTag(ItemTag))
      {
         LayerSettings = DebugSettings->GetLayerSettings(DebugTag_InventoryInteraction);
         if (LayerSettings)
         {
            FString ColorMsg = FString::Printf(TEXT("✅ LayerSettings found! Color: R=%d G=%d B=%d, bShowCollisionShapes=%s"), 
                LayerSettings->TraceColor.R, LayerSettings->TraceColor.G, LayerSettings->TraceColor.B,
                LayerSettings->bShowCollisionShapes ? TEXT("TRUE") : TEXT("FALSE"));
            UE_LOG(LogDebugSystem, Warning, TEXT("%s"), *ColorMsg);
                
            bHiddenInGame = !LayerSettings->bShowCollisionShapes;
            ShapeColor = LayerSettings->TraceColor;
         }
         else
         {
            UE_LOG(LogDebugSystem, Error, TEXT("❌ LayerSettings NULL for InventoryInteraction!"));
         }
      }
      // ... rest of your code
   }
   else
   {
      UE_LOG(LogDebugSystem, Error, TEXT("❌ DebugSettings is NULL!"));
      // ... fallback
   }
}




void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from spatial hash grid
    if (AActor* Owner = GetOwner())
    {
       if (UWorld* World = GetWorld())
       {
          if (UInteractionSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UInteractionSubsystem>())
          {
             Subsystem->UnregisterInteractable(Owner);
          }
       }
    }
    
    // Clean up - hide prompt if showing
    if (InteractableInstigator != nullptr)
    {
       HidePrompt();
       InteractableInstigator = nullptr;
    }
    
    Super::EndPlay(EndPlayReason);
}

// ============================================================================
// TICK COMPONENT - NEW (ONLY FOR MOVEMENT TRACKING)
// ============================================================================

void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Only tick if movement tracking is enabled
	if (!bTrackMovementForSpatialHash)
		return;
	
	// Rate limit movement checks
	MovementCheckTimeAccumulator += DeltaTime;
	if (MovementCheckTimeAccumulator < MovementCheckInterval)
		return;
	
	MovementCheckTimeAccumulator = 0.f;
	
	// Check if actor moved significantly
	UpdateSpatialGridPosition();
}

// ============================================================================
// SPATIAL HASH MOVEMENT TRACKING IMPLEMENTATIONS - NEW
// ============================================================================

void UInteractableComponent::UpdateSpatialGridPosition()
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return;
	
	FVector CurrentLocation = Owner->GetActorLocation();
	float DistanceMoved = FVector::Dist(LastKnownLocation, CurrentLocation);
	
	// Check if moved beyond threshold
	if (DistanceMoved >= MovementThreshold)
	{
		// Notify spatial grid of position change
		if (UInteractionSubsystem* System = UInteractionSubsystem::Get(Owner->GetWorld()))
		{
			System->NotifyActorMoved(Owner, LastKnownLocation);
			

		}
		
		// Update last known location
		LastKnownLocation = CurrentLocation;
	}
}

void UInteractableComponent::AutoDetectMovement()
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return;
	
	// Check if actor has movement components
	bool bHasMovement = HasMovementComponents();
	
	// Check if actor is set to movable
	USceneComponent* RootComp = Owner->GetRootComponent();
	bool bIsMovable = RootComp && RootComp->Mobility == EComponentMobility::Movable;
	
	// Auto-enable movement tracking if we detect movement capability
	if (bHasMovement || bIsMovable)
	{
		bTrackMovementForSpatialHash = true;
		

	}
}

bool UInteractableComponent::HasMovementComponents() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return false;
	
	// Check for common movement components
	if (Owner->FindComponentByClass<UCharacterMovementComponent>())
		return true;
	
	if (Owner->FindComponentByClass<UFloatingPawnMovement>())
		return true;
	
	if (Owner->FindComponentByClass<URotatingMovementComponent>())
		return true;
	
	if (Owner->FindComponentByClass<UProjectileMovementComponent>())
		return true;
	
	if (Owner->FindComponentByClass<UInterpToMovementComponent>())
		return true;
	
	// Check if owner is a Character (has movement by default)
	if (Cast<ACharacter>(Owner))
		return true;
	
	return false;
}

void UInteractableComponent::ForceUpdateSpatialGrid()
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return;
	
	FVector OldLocation = LastKnownLocation;
	FVector NewLocation = Owner->GetActorLocation();
	
	if (UInteractionSubsystem* System = UInteractionSubsystem::Get(Owner->GetWorld()))
	{
		System->NotifyActorMoved(Owner, OldLocation);
		LastKnownLocation = NewLocation;
		

	}
}

// ============================================================================
// OVERLAP EVENTS
// ============================================================================

void UInteractableComponent::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // Ignore if not enabled
    if (!bIsEnabled)
    {
       return;
    }
    
    // Only respond to actors that can interact (e.g., player pawns)
    if (!CanActorInteract(OtherActor))
    {
       return;
    }
    
    // Store the instigator
    InteractableInstigator = OtherActor;
    
    // Show the prompt
    ShowPrompt();
    
    FString Message = FString::Printf(TEXT("InteractableComponent: %s - Overlap BEGIN with %s"), 
       *GetOwner()->GetName(), *OtherActor->GetName());
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
}

void UInteractableComponent::OnOverlapEnd(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    // Only process if this was the current instigator
    if (InteractableInstigator != OtherActor)
    {
       return;
    }
    
    // Clear the instigator
    InteractableInstigator = nullptr;
    
    // Hide the prompt
    HidePrompt();
    
    FString Message = FString::Printf(TEXT("InteractableComponent: %s - Overlap END with %s"), 
       *GetOwner()->GetName(), *OtherActor->GetName());
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
}

// ============================================================================
// INTERACTION
// ============================================================================

bool UInteractableComponent::Interact(AActor* Instigator)
{
    if (!bIsEnabled)
    {
       FString Message = FString::Printf(TEXT("InteractableComponent: %s - Cannot interact (disabled)"), 
          *GetOwner()->GetName());
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
       return false;
    }
    
    if (!IsInstigatorInRange())
    {
       FString Message = FString::Printf(TEXT("InteractableComponent: %s - Cannot interact (no one in range)"), 
          *GetOwner()->GetName());
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
       return false;
    }
	
    // Call the blueprint implementable event
    OnInteract(Instigator);
    
    FString Message = FString::Printf(TEXT("InteractableComponent: %s - Interaction SUCCESS with %s"), 
       *GetOwner()->GetName(), *Instigator->GetName());
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
    
    return true;
}

void UInteractableComponent::OnInteract_Implementation(AActor* Instigator)
{
    // Default implementation - override in blueprints
	//Update DurabilityComponet if exists
	/*
	 *Wrong approach... use inventorycomponent centric approach instead...
	 */
	if (!Instigator || !Instigator->HasAuthority()) return;
	
	AActor* Owner = GetOwner();

	if (!Instigator || !Owner || !Owner->HasAuthority())
	{
		return;
	}

	if (!Owner->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		return;
	}

	// Get inventory from INSTIGATOR (the player), not Owner
	if (!Instigator->GetClass()->ImplementsInterface(UInventoryInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Instigator does not implement IInventoryInterface"));
		return;
	}

	// Get durability from OWNER (the pickup item)
	float Durability = 1.0f;
	
	if (Owner->GetClass()->ImplementsInterface(UDurabilityInterface::StaticClass()))
	{
		if (IDurabilityInterface::Execute_HasDurability(Owner))
		{
			Durability = IDurabilityInterface::Execute_GetDurability(Owner);
		}
	}

	// Get and update pickup data.
	FItemData_Dynamic PickupData = IInteractableInterface::Execute_GetPickupData(Owner);
	PickupData.Durability = Durability;
	IInteractableInterface::Execute_SetPickupData(Owner, PickupData);

	IInventoryInterface::Execute_AddItem(Instigator, PickupData.ID, PickupData.Quantity, PickupData.Quality, PickupData.Durability, PickupData.bIsStolen);


    FString Message = FString::Printf(TEXT("InteractableComponent: %s - Default OnInteract called"), 
       *GetOwner()->GetName());
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
}

void UInteractableComponent::SetEnabled(bool bEnabled)
{
	if (!GetOwner()->HasAuthority())
	{
		return; // Only server can change enabled state
	}

    bIsEnabled = bEnabled;
    
    // If disabled while someone is in range, hide the prompt
    if (!bIsEnabled && InteractableInstigator != nullptr)
    {
       HidePrompt();
    }
    // If enabled and someone is in range, show the prompt
    else if (bIsEnabled && InteractableInstigator != nullptr)
    {
       ShowPrompt();
    }
    
    FString Message = FString::Printf(TEXT("InteractableComponent: %s - Enabled set to %s"), 
       *GetOwner()->GetName(), bIsEnabled ? TEXT("TRUE") : TEXT("FALSE"));
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
}

// ============================================================================
// WIDGET MANAGEMENT
// ============================================================================

void UInteractableComponent::ShowPrompt()
{
    if (!InteractionPromptClass)
    {
       FString Message = FString::Printf(TEXT("InteractableComponent: %s - No InteractionPromptClass set!"), *GetOwner()->GetName());
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Warning);
       return;
    }
    
    UWidgetManagerBase* WidgetManager = GetWidgetManager();
    if (!WidgetManager)
    {
       FString Message = FString::Printf(TEXT("InteractableComponent: %s - No WidgetManager found!"), *GetOwner()->GetName());
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Error);
       return;
    }
    
    // Show the widget through WidgetManager
    // This will automatically hide any previous prompt (bAllowMultiple = false)
    PromptWidgetInstance = WidgetManager->ShowWidget(InteractionPromptClass);
    
    if (PromptWidgetInstance)
    {
       // Update the prompt text if it has a text property
       UpdatePromptText(InteractionText);
       
       FString Message = FString::Printf(TEXT("InteractableComponent: %s - Prompt shown"), *GetOwner()->GetName());
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
    }
}

void UInteractableComponent::HidePrompt()
{
    if (!InteractionPromptClass)
    {
       return;
    }
    
    UWidgetManagerBase* WidgetManager = GetWidgetManager();
    if (!WidgetManager)
    {
       return;
    }
    
    // Hide the widget through WidgetManager
    // This will return it to the pool
    WidgetManager->HideWidget(InteractionPromptClass);
    PromptWidgetInstance = nullptr;
    
    FString Message = FString::Printf(TEXT("InteractableComponent: %s - Prompt hidden"), *GetOwner()->GetName());
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
}

void UInteractableComponent::UpdatePromptText(const FText& NewText)
{
    InteractionText = NewText;
    
    if (!PromptWidgetInstance)
    {
       return;
    }
    
    // Try to find and update a text property on the widget
    // This is a common pattern - look for "InteractionText" or "PromptText" property
    UFunction* SetTextFunction = PromptWidgetInstance->FindFunction(FName("SetInteractionText"));
    if (!SetTextFunction)
    {
       SetTextFunction = PromptWidgetInstance->FindFunction(FName("SetPromptText"));
    }
    
    if (SetTextFunction)
    {
       struct FSetTextParams
       {
          FText Text;
       };
       
       FSetTextParams Params;
       Params.Text = NewText;
       PromptWidgetInstance->ProcessEvent(SetTextFunction, &Params);
    }
    else
    {
       // No function found - widget should implement SetInteractionText or SetPromptText
       FString Message = FString::Printf(TEXT("InteractableComponent: %s - Widget has no SetInteractionText/SetPromptText function"), 
          *GetOwner()->GetName());
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, Message, true, EDebugVerbosity::Verbose);
    }
}

// ============================================================================
// HELPERS
// ============================================================================

UWidgetManagerBase* UInteractableComponent::GetWidgetManager() const
{
    // Return cached if available
    if (CachedWidgetManager)
    {
       return CachedWidgetManager;
    }
    
    // Get the owning actor's player controller
    AActor* Owner = GetOwner();
    if (!Owner)
    {
       return nullptr;
    }
    
    // Try to get player controller
    APlayerController* PC = Cast<APlayerController>(Owner);
    if (!PC)
    {
       // If owner is not a controller, try to get the first player controller
       PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    }
    
    if (!PC)
    {
       return nullptr;
    }
    
    // Get WidgetManager subsystem from local player
    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
       return nullptr;
    }
    
    // Cache it for future use
    UInteractableComponent* MutableThis = const_cast<UInteractableComponent*>(this);
    MutableThis->CachedWidgetManager = LocalPlayer->GetSubsystem<UWidgetManagerBase>();
    
    return CachedWidgetManager;
}

bool UInteractableComponent::CanActorInteract(AActor* Actor) const
{
    if (!Actor)
    {
       return false;
    }
    
    // Default implementation: Check if it's a player controlled pawn
    APawn* Pawn = Cast<APawn>(Actor);
    if (!Pawn)
    {
       return false;
    }
    
    // Must be player controlled
    return Pawn->IsPlayerControlled();
}

FGameplayTag UInteractableComponent::GetInteractableDebugTag()
{
    // WARNING: SearchQuery_Tag is undefined in this context, assuming a simple tag check
    // if (InteractableTypeTag.IsValid() && InteractableTypeTag.MatchesAny(SearchQuery_Tag))
    //    return DebugTag_InventoryInteraction;
    
    FGameplayTag ItemTag = FGameplayTag::RequestGameplayTag(FName("Interactable.Item"), false);
    if (InteractableTypeTag.MatchesTag(ItemTag))
       return DebugTag_InventoryInteraction;
    
    return DebugTag_Interaction;   
}

void UInteractableComponent::OnInteractableTypeTagNotFound()
{
    if (!InteractableTypeTag.IsValid())
    {
       FString Message = FString::Printf(TEXT("RequestedGameplayTag %s can not be found in DefaultGameplayTags.ini plase check the file: InteractableComponent: %s, Owner: %s"), *InteractableTypeTag.ToString(), *GetName(), *GetOwner()->GetName());
       UDebugSubsystem::PrintDebug(this, WWDebugTags::Debug(), Message, true, EDebugVerbosity::Error );
    }
}


bool UInteractableComponent::GetIsEnabled() const
{
	return bIsEnabled;
}

void UInteractableComponent::SetDebugTagsOnAnyVisualisation()
{
    // SAFE: Direct struct access (non-virtual)
    //(DEPRECIATED) ShapeColor = DebugDefaultsConstants::DEBUG_LAYER_INTERACTION_LINEARCOLOR.ToFColor(true);
    // DebugSubsystem will handle coloring and visualisation
    //Change Collision Visualisation Based on InterctableTypeTag
    if (!InteractableTypeTag.IsValid())
    {
       FString ErrorText = FString::Printf(TEXT("InteractableTypeTag not valid, setting DebugTag to DebugTag_Interactable component: %s owner: %s"), *GetName(), *GetOwner()->GetName());
       UDebugSubsystem::PrintDebug(this, DebugTag, ErrorText, true, EDebugVerbosity::Error);
    }

    else if (InteractableTypeTag == FGameplayTag::RequestGameplayTag(FName("Interactable.Item"), false))
    {
       UDebugSubsystem::SetupCollisionVisualization(this, this, DebugTag_InventoryInteraction);
    }
    else if (InteractableTypeTag == FGameplayTag::RequestGameplayTag(FName("Interactable"), false))
    {
       UDebugSubsystem::SetupCollisionVisualization(this, this, DebugTag_Interaction);
    }
}

void UInteractableComponent::SetupInteractableTypeTag()
{
    //SetupCollisionVisualisation..
    // WARNING: SearchQuery_Tag is undefined in this context, assuming a simple tag check
    /*
    if (InteractableTypeTag.IsValid() && InteractableTypeTag.MatchesAny(SearchQuery_Tag))
       UDebugSubsystem::SetupCollisionVisualization(this, this, DebugTag_InventoryInteraction);
    else
       UDebugSubsystem::SetupCollisionVisualization(this, this, DebugTag_Interaction);
    */
    FGameplayTag ItemTag = FGameplayTag::RequestGameplayTag(FName("Interactable.Item"), false);
    if (InteractableTypeTag.MatchesTag(ItemTag))
       UDebugSubsystem::SetupCollisionVisualization(this, this, DebugTag_InventoryInteraction);
    else
       UDebugSubsystem::SetupCollisionVisualization(this, this, DebugTag_Interaction);
}


void UInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInteractableComponent, bIsEnabled);
}

void UInteractableComponent::OnRep_IsEnabled()
{
	// Hide prompt if disabled while someone is in range
	if (!bIsEnabled && PromptWidgetInstance)
	{
		HidePrompt();
	}
}
