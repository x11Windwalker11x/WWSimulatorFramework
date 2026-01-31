// PawnComponentManager.cpp
#include "Components/PawnComponentManager.h"
#include "Components/InventoryComponent.h"
#include "Components/InteractorComponent.h"
#include "Components/InteractableComponent.h"
#include "Pawns/Components/InputActionMechanicComponent.h"
#include "Camera/Manager/MPC_PlayerCameraManager.h"
#include "Subsystems/UniversalSpawnManager.h"
#include "AIController.h"
#include "GameFramework/PlayerController.h"
#include "Debug/DebugSubsystem.h"
#include "WW_TagLibrary.h"

UPawnComponentManager::UPawnComponentManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    InventoryComponent = nullptr;
    InteractorComponent = nullptr;
    InteractableComponent = nullptr;
    InputActionMechanicComponent = nullptr;
    OwnerPlayerController = nullptr;
    OwnerAIController = nullptr;
    OwnerCameraManager = nullptr;
}

void UPawnComponentManager::BeginPlay()
{
    Super::BeginPlay();
    CacheComponents();
    BindDelegates();
}

void UPawnComponentManager::CacheComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    
    InventoryComponent = Owner->FindComponentByClass<UInventoryComponent>();
    InteractorComponent = Owner->FindComponentByClass<UInteractorComponent>();
    InteractableComponent = Owner->FindComponentByClass<UInteractableComponent>();
    InputActionMechanicComponent = Owner->FindComponentByClass<UInputActionMechanicComponent>();

    UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Pawn(),
        FString::Printf(TEXT("PawnComponentManager cached: Inv=%s, Interactor=%s, Interactable=%s"),
            InventoryComponent ? TEXT("✓") : TEXT("✗"),
            InteractorComponent ? TEXT("✓") : TEXT("✗"),
            InteractableComponent ? TEXT("✓") : TEXT("✗")),
        false, EDebugVerbosity::Info);
}

void UPawnComponentManager::BindDelegates()
{
    if (InventoryComponent)
    {
        InventoryComponent->OnItemDropped.AddDynamic(this, &ThisClass::HandleItemDropped);
        InventoryComponent->OnInventoryChanged.AddDynamic(this, &ThisClass::HandleInventoryChanged);
    }
}

void UPawnComponentManager::OnPawnPossessed(AController* NewController)
{
    if (!NewController) return;

    // Try PlayerController first
    if (APlayerController* PC = Cast<APlayerController>(NewController))
    {
        if (PC->IsLocalPlayerController())
        {
            OwnerPlayerController = PC;
            SetupCameraManager(PC);
            
            // Notify InteractorComponent
            if (InteractorComponent)
            {
                InteractorComponent->OnPossess(Cast<APawn>(GetOwner()));
            }

            UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Pawn(),
                TEXT("✅ PawnComponentManager: Cached local PlayerController"), false, EDebugVerbosity::Info);
        }
        return;
    }

    // Try AIController
    if (AAIController* AIC = Cast<AAIController>(NewController))
    {
        OwnerAIController = AIC;
        
        UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Pawn(),
            TEXT("✅ PawnComponentManager: Cached AIController"), false, EDebugVerbosity::Info);
    }
}

FGameplayTagContainer UPawnComponentManager::GetTagsFromComponent(const FName& CrafterID, UActorComponent* TargetComponent, const UDataTable* DataTable ) const
{
    Super::GetTagsFromComponent();    
}



void UPawnComponentManager::SetupCameraManager(APlayerController* PC)
{
    if (!PC) return;

    OwnerCameraManager = Cast<AMPC_PlayerCameraManager>(PC->PlayerCameraManager);
    
    if (OwnerCameraManager)
    {
        OwnerCameraManager->InitializeCameras(Cast<APawn>(GetOwner()));
        
        UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Camera(),
            TEXT("🎬 PawnComponentManager: Initialized cameras"), false, EDebugVerbosity::Info);
    }
}

//========================================
// CROSS-SYSTEM HANDLERS
//========================================

void UPawnComponentManager::HandleItemDropped(const FInventorySlot& DroppedSlot, FVector DropLocation)
{
    if (USpawnManager* SM = USpawnManager::Get(this))
    {
        SM->SpawnPickup(DropLocation, FRotator::ZeroRotator, DroppedSlot.ItemID, DroppedSlot.Quantity);
        
        UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Inventory(),
            FString::Printf(TEXT("📦 Dropped %s x%d"), *DroppedSlot.ItemID.ToString(), DroppedSlot.Quantity),
            false, EDebugVerbosity::Info);
    }
}

void UPawnComponentManager::HandleInventoryChanged()
{
    // Future: Notify widget system, update UI, etc.
}

