// Copyright notice...

#include "Pawns/MPC_Character_Master.h"
#include "InteractionSubsystem.h"
#include "ModularPlayerController_Master.h"
#include "Components/PhysicsGrabComponent.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Debug/DebugSubsystem.h"
#include "Editor/WidgetCompilerLog.h"

AMPC_Character_Master::AMPC_Character_Master()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	OwnerPlayerController = nullptr;
	OwnerAIController = nullptr;

	bCanInteract = true;

	PawnComponentManager = CreateDefaultSubobject<UPawnComponentManager>(TEXT("PawnComponentManager"));
	InputActionMechanicComponent = CreateDefaultSubobject<UInputActionMechanicComponent>("InputActionMechanicComponent");
	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("InteractorComponent"));
	PhysicsGrabComponent = CreateDefaultSubobject<UPhysicsGrabComponent>(TEXT("PhysicsGrabComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}

void AMPC_Character_Master::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
    
	if (PawnComponentManager)
	{
		PawnComponentManager->OnPawnPossessed(NewController);
	}

}

void AMPC_Character_Master::OnRep_Controller()
{
	Super::OnRep_Controller();
    
	if (PawnComponentManager)
	{
		PawnComponentManager->OnPawnPossessed(GetController());
	}

}

void AMPC_Character_Master::BeginPlay()
{
	Super::BeginPlay();
	Tags.Add(FName("Interactor"));
	
	MovementGameplayTags.AddTag(FWWTagLibrary::Movement_Stance_Standing());

	FString BeginPlayMsg = FString::Printf(TEXT("✅ Character %s BeginPlay completed"), *GetName());
	UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Character(), BeginPlayMsg, false, EDebugVerbosity::Info);

	
}

void AMPC_Character_Master::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMPC_Character_Master::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UActorComponent* AMPC_Character_Master::GetCrafterComponent_Implementation()
{
	return CrafterComponent;
}

UActorComponent* AMPC_Character_Master::GetInteractorComponentAsActorComponent_Implementation(
	TSubclassOf<UActorComponent> ComponentClass)
{
	return InteractorComponent;
}

bool AMPC_Character_Master::GetbCanInteract_Implementation()
{
	return bCanInteract;
}

void AMPC_Character_Master::OnCameraToggle_Implementation(APawn* ControlledPawn)
{
	if (!ControlledPawn || !PawnComponentManager) return;

	if (AMPC_PlayerCameraManager* CamManager = PawnComponentManager->GetOwnerCameraManager())
	{
		CamManager->ToggleCamera(ControlledPawn);
	}

}

UInventoryComponent* AMPC_Character_Master::GetInventoryComponent_Implementation()
{
	return InventoryComponent;
}

UActorComponent* AMPC_Character_Master::GetInventoryComponentAsActorComponent_Implementation(
	TSubclassOf<UActorComponent> ComponentClass)
{
	return InventoryComponent;
}

AAIController* AMPC_Character_Master::GetInteractorAIController_Implementation()
{
	return PawnComponentManager ? PawnComponentManager->GetOwnerAIController() : nullptr;

}

UActorComponent* AMPC_Character_Master::GetInteractorComponent_Implementation()
{
	return InteractorComponent;
}



// FName AMPC_Character_Master::GetCrafterID_Implementation()
// {
// 	return CrafterComponent->GetCrafterTags();
// }

FGameplayTagContainer AMPC_Character_Master::GetCrafterTags_Implementation()
{
	return CrafterComponent->GetCrafterTags();
}

UPhysicsGrabComponent* AMPC_Character_Master::GetPhysicsGrabComponent_Implementation()
{
	return PhysicsGrabComponent;
}

UActorComponent* AMPC_Character_Master::GetPhysicsGrabComponentAsActorComponent_Implementation()
{
	return PhysicsGrabComponent;
}

void AMPC_Character_Master::OnJumpActionPressed_Implementation()
{
	Super::Jump();
	UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Movement(), TEXT("JumpActionPressed"), false, EDebugVerbosity::Info);
}

void AMPC_Character_Master::OnCrouchActionPressed_Implementation()
{
	if (!MovementGameplayTags.HasTag(FWWTagLibrary::Movement_Stance_Crouch()))
	{
		if (CanCrouch())
		{
			Crouch();
			MovementGameplayTags.RemoveTag(FWWTagLibrary::Movement_Stance_Standing());
			MovementGameplayTags.AddTag(FWWTagLibrary::Movement_Stance_Crouch());
			UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Movement(), TEXT("✅ Started Crouch - Tag Added"), false, EDebugVerbosity::Info);
		}
	}
}

void AMPC_Character_Master::OnCrouchActionReleased_Implementation()
{
	if (MovementGameplayTags.HasTag(FWWTagLibrary::Movement_Stance_Crouch()))
	{
		UnCrouch();
		MovementGameplayTags.RemoveTag(FWWTagLibrary::Movement_Stance_Crouch());
		MovementGameplayTags.AddTag(FWWTagLibrary::Movement_Stance_Standing());
		UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Movement(), TEXT("✅ Stopped Crouch - Tag Removed"), false, EDebugVerbosity::Info);
	}
}

void AMPC_Character_Master::OnInventoryToggle_Implementation()
{
	UDebugSubsystem::PrintDebug(this, WWDebugTags::Debug_Inventory(), TEXT("InventoryToggle from interface called"), false, EDebugVerbosity::Info);
	if (InventoryComponent && GetController()->IsLocalPlayerController())
	{
		InventoryComponent->ToggleInventory();
	}
}

void AMPC_Character_Master::OnCrouchToggle_Implementation()
{
	if (MovementGameplayTags.HasTag(FWWTagLibrary::Movement_Stance_Crouch()))
	{
		UnCrouch();
		MovementGameplayTags.RemoveTag(FWWTagLibrary::Movement_Stance_Crouch());
		MovementGameplayTags.AddTag(FWWTagLibrary::Movement_Stance_Standing());
		UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Movement(), TEXT("✅ Toggle OFF - Tag Removed"), false, EDebugVerbosity::Info);
	}
	else
	{
		if (CanCrouch())
		{
			Crouch();
			MovementGameplayTags.RemoveTag(FWWTagLibrary::Movement_Stance_Standing());
			MovementGameplayTags.AddTag(FWWTagLibrary::Movement_Stance_Crouch());
			UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Movement(), TEXT("✅ Toggle ON - Tag Added"), false, EDebugVerbosity::Info);
		}
	}
}

void AMPC_Character_Master::ToggleCrouch()
{
	OnCrouchToggle_Implementation();
}

void AMPC_Character_Master::StartCrouch()
{
	OnCrouchActionPressed_Implementation();
}

void AMPC_Character_Master::StopCrouch()
{
	OnCrouchActionReleased_Implementation();
}

bool AMPC_Character_Master::IsCrouching() const
{
	return MovementGameplayTags.HasTag(FWWTagLibrary::Movement_Stance_Crouch());
}

UClass* AMPC_Character_Master::GetInteractorClass_Implementation()
{
	return GetClass();
}

AController* AMPC_Character_Master::GetInteractorController_Implementation()
{
	return GetController();
}

APlayerController* AMPC_Character_Master::GetInteractorPlayerController_Implementation()
{
	return GetController<APlayerController>();
}

APawn* AMPC_Character_Master::GetInteractorPawn_Implementation()
{
	return this;
}

ACharacter* AMPC_Character_Master::GetInteractorCharacter_Implementation()
{
	return this;
}

void AMPC_Character_Master::SaveCharacterState(const FString& PlayerID)
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>())
            {
                // Save Character Transform & Stats
                if (UCharacterSaveModule* CharModule = SaveSubsystem->GetCharacterModule())
                {
                    CharModule->SaveCharacterTransform(PlayerID, GetActorTransform(), World->GetMapName());
                    
                    FCharacterStatsData Stats;
                    // Populate from your character
                    // Stats.Health = CurrentHealth;
                    // Stats.MaxHealth = MaxHealth;
                    // etc...
                    
                    CharModule->SaveCharacterStats(PlayerID, Stats);
                }
                
                // Save Inventory
                if (UInventorySaveModule* InvModule = SaveSubsystem->GetInventoryModule())
                {
                    if (InventoryComponent)
                    {
                        FPlayerInventoryData& InvData = InvModule->GetOrCreatePlayerInventory(PlayerID);
                        InvData.InventoryItems.Empty();
                        
                        // Save items
                        // Implement based on your inventory system
                    }
                }
                
                // Save Abilities
                if (UAbilitiesSaveModule* AbilityModule = SaveSubsystem->GetAbilitiesModule())
                {
                    FPlayerAbilitiesData& AbilityData = AbilityModule->GetOrCreatePlayerAbilities(PlayerID);
                    // Save abilities here
                }
                
                SaveSubsystem->QuickSave();
                
                UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Character(), 
                    FString::Printf(TEXT("Saved character state for: %s"), *PlayerID), false);
            }
        }
    }
}

void AMPC_Character_Master::LoadCharacterState(const FString& PlayerID)
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>())
            {
                // Load Character Transform & Stats
                if (UCharacterSaveModule* CharModule = SaveSubsystem->GetCharacterModule())
                {
                    FTransform LoadedTransform;
                    FString LoadedLevel;
                    
                    if (CharModule->LoadCharacterTransform(PlayerID, LoadedTransform, LoadedLevel))
                    {
                        SetActorTransform(LoadedTransform);
                        
                        FCharacterStatsData LoadedStats;
                        if (CharModule->LoadCharacterStats(PlayerID, LoadedStats))
                        {
                            // Apply stats
                            // CurrentHealth = LoadedStats.Health;
                            // MaxHealth = LoadedStats.MaxHealth;
                            // etc...
                        }
                    }
                }
                
                // Load Inventory
                if (UInventorySaveModule* InvModule = SaveSubsystem->GetInventoryModule())
                {
                    if (InventoryComponent && InvModule->HasPlayerData(PlayerID))
                    {
                        FPlayerInventoryData& InvData = InvModule->GetOrCreatePlayerInventory(PlayerID);
                        // Load items into inventory
                    }
                }
                
                // Load Abilities
                if (UAbilitiesSaveModule* AbilityModule = SaveSubsystem->GetAbilitiesModule())
                {
                    if (AbilityModule->HasPlayerData(PlayerID))
                    {
                        FPlayerAbilitiesData& AbilityData = AbilityModule->GetOrCreatePlayerAbilities(PlayerID);
                        // Load abilities
                    }
                }
                
                UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Character(), 
                    FString::Printf(TEXT("Loaded character state for: %s"), *PlayerID), false);
            }
        }
    }
}


