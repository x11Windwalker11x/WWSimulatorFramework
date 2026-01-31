#include "Actors/SimulatedDeviceBase.h"
#include "Components/DeviceStateComponent.h"
#include "Components/DurabilityComponent.h"
#include "Components/InteractableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

ASimulatedDeviceBase::ASimulatedDeviceBase()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // Root
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    // Mesh
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(Root);

    // Device State
    DeviceStateComponent = CreateDefaultSubobject<UDeviceStateComponent>(TEXT("DeviceStateComponent"));

    // Durability (optional, can be disabled)
    DurabilityComponent = CreateDefaultSubobject<UDurabilityComponent>(TEXT("DurabilityComponent"));

    // Interactable
    InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));
    InteractableComponent->SetupAttachment(Root);
}

void ASimulatedDeviceBase::BeginPlay()
{
    Super::BeginPlay();

    // Bind to device state events
    if (DeviceStateComponent)
    {
        DeviceStateComponent->OnDeviceStateChanged.AddDynamic(this, &ASimulatedDeviceBase::HandleDeviceStateChanged);
    }

    // Bind to durability events
    if (DurabilityComponent)
    {
        DurabilityComponent->OnItemBroken.AddDynamic(this, &ASimulatedDeviceBase::HandleDeviceBroken);
    }

    // Disable durability component if not needed
    if (!bHasDurability && DurabilityComponent)
    {
        DurabilityComponent->DestroyComponent();
        DurabilityComponent = nullptr;
    }
}

// ============================================================================
// IDEVICEINTERFACE IMPLEMENTATION
// ============================================================================

FGameplayTag ASimulatedDeviceBase::GetDeviceState_Implementation() const
{
    if (!DeviceStateComponent) return FWWTagLibrary::Simulator_Device_State_Off();
    return DeviceStateComponent->GetDeviceState();
}

void ASimulatedDeviceBase::SetDeviceState_Implementation(FGameplayTag NewState)
{
    if (DeviceStateComponent)
    {
        DeviceStateComponent->SetDeviceState(NewState);
    }
}

bool ASimulatedDeviceBase::IsUsable_Implementation() const
{
    if (!DeviceStateComponent) return false;
    return DeviceStateComponent->IsUsable();
}

void ASimulatedDeviceBase::TurnOn_Implementation()
{
    if (DeviceStateComponent)
    {
        DeviceStateComponent->TurnOn();
    }
}

void ASimulatedDeviceBase::TurnOff_Implementation()
{
    if (DeviceStateComponent)
    {
        DeviceStateComponent->TurnOff();
    }
}

bool ASimulatedDeviceBase::IsPoweredOn_Implementation() const
{
    if (!DeviceStateComponent) return false;
    return DeviceStateComponent->IsPoweredOn();
}

bool ASimulatedDeviceBase::NeedsMaintenance_Implementation() const
{
    if (!DeviceStateComponent) return false;
    return DeviceStateComponent->NeedsMaintenance();
}

void ASimulatedDeviceBase::PerformMaintenance_Implementation()
{
    if (DeviceStateComponent)
    {
        DeviceStateComponent->PerformMaintenance();
    }
}

UActorComponent* ASimulatedDeviceBase::GetDeviceComponent_Implementation()
{
    return DeviceStateComponent;
}

// ============================================================================
// IINTERACTABLEINTERFACE IMPLEMENTATION
// ============================================================================

void ASimulatedDeviceBase::OnInteract_Implementation(AController* InstigatorController)
{
    if (!InstigatorController) return;
    if (!DeviceStateComponent) return;

    // Can't interact if not usable
    if (!DeviceStateComponent->IsUsable()) return;

    APawn* InstigatorPawn = InstigatorController->GetPawn();
    if (!InstigatorPawn) return;

    // Start using device
    if (DeviceStateComponent->StartUse(InstigatorPawn))
    {
        OnDeviceUsed(InstigatorPawn);
    }
}

bool ASimulatedDeviceBase::IsCurrentlyInteractable_Implementation()
{
    if (!DeviceStateComponent) return false;
    return DeviceStateComponent->IsUsable();
}

void ASimulatedDeviceBase::SetInteractionEnabled_Implementation(bool bEnabled)
{
    if (InteractableComponent)
    {
        InteractableComponent->SetEnabled(bEnabled);
    }
}

bool ASimulatedDeviceBase::GetInteractionEnabled_Implementation()
{
    if (!InteractableComponent) return false;
    return InteractableComponent->GetIsEnabled();
}

// ============================================================================
// INTERNAL BINDINGS
// ============================================================================

void ASimulatedDeviceBase::HandleDeviceStateChanged(FGameplayTag OldState, FGameplayTag NewState)
{
    OnDeviceStateChanged(OldState, NewState);
}

void ASimulatedDeviceBase::HandleDeviceBroken(AActor* Causer)
{
    OnDeviceBroken();
}