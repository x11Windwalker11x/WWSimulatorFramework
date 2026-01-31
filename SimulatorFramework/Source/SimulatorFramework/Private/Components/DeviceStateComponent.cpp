#include "Components/DeviceStateComponent.h"
#include "Components/DurabilityComponent.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Net/UnrealNetwork.h"

UDeviceStateComponent::UDeviceStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    
    DefaultState = FWWTagLibrary::Simulator_Device_State_Off();
}

void UDeviceStateComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize state
    if (!DefaultState.IsValid())
    {
        DefaultState = FWWTagLibrary::Simulator_Device_State_Off();
    }
    CurrentState = DefaultState;
    
    // Find optional durability component
    CheckDurabilityComponent();
    
    // Bind to durability broken event if exists
    if (DurabilityComponent)
    {
        DurabilityComponent->OnItemBroken.AddDynamic(this, &UDeviceStateComponent::OnDurabilityBroken);
    }
    
    // Enable tick only if maintenance tracking needed
    if (MaintenanceInterval > 0.0f)
    {
        PrimaryComponentTick.bCanEverTick = true;
        SetComponentTickEnabled(true);
    }
}

void UDeviceStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (GetOwner()->HasAuthority())
    {
        UpdateMaintenanceTimer(DeltaTime);
    }
}

void UDeviceStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UDeviceStateComponent, CurrentState);
    DOREPLIFETIME(UDeviceStateComponent, bIsPoweredOn);
    DOREPLIFETIME(UDeviceStateComponent, CurrentUser);
    DOREPLIFETIME(UDeviceStateComponent, TimeSinceLastMaintenance);
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UDeviceStateComponent::TurnOn()
{
    if (!GetOwner()->HasAuthority())
    {
        Server_TurnOn();
        return;
    }
    Internal_TurnOn();
}

void UDeviceStateComponent::TurnOff()
{
    if (!GetOwner()->HasAuthority())
    {
        Server_TurnOff();
        return;
    }
    Internal_TurnOff();
}

void UDeviceStateComponent::SetDeviceState(FGameplayTag NewState)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_SetState(NewState);
        return;
    }
    Internal_SetState(NewState);
}

bool UDeviceStateComponent::StartUse(AActor* User)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_StartUse(User);
        return true; // Optimistic
    }
    return Internal_StartUse(User);
}

void UDeviceStateComponent::StopUse(AActor* User)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_StopUse(User);
        return;
    }
    Internal_StopUse(User);
}

void UDeviceStateComponent::PerformMaintenance()
{
    if (!GetOwner()->HasAuthority())
    {
        Server_PerformMaintenance();
        return;
    }
    Internal_PerformMaintenance();
}

// ============================================================================
// QUERIES
// ============================================================================

bool UDeviceStateComponent::IsUsable() const
{
    // Can't use if broken
    if (CurrentState == FWWTagLibrary::Simulator_Device_State_Broken()) return false;
    
    // Can't use if requires power and not powered
    if (bRequiresPower && !bIsPoweredOn) return false;
    
    // Can't use if under maintenance
    if (CurrentState == FWWTagLibrary::Simulator_Device_State_Maintenance()) return false;
    
    // Can't use if already in use
    if (CurrentState == FWWTagLibrary::Simulator_Device_State_InUse()) return false;
    
    // Can't use if no power state
    if (CurrentState == FWWTagLibrary::Simulator_Device_State_NoPower()) return false;
    
    // Check durability if exists
    if (DurabilityComponent)
    {
        if (DurabilityComponent->GetClass()->ImplementsInterface(UDurabilityInterface::StaticClass()))
        {
            if (!IDurabilityInterface::Execute_IsUsable(DurabilityComponent))
            {
                return false;
            }
        }
    }
    
    return true;
}

bool UDeviceStateComponent::IsInUse() const
{
    return CurrentState == FWWTagLibrary::Simulator_Device_State_InUse();
}

bool UDeviceStateComponent::IsBroken() const
{
    return CurrentState == FWWTagLibrary::Simulator_Device_State_Broken();
}

bool UDeviceStateComponent::NeedsMaintenance() const
{
    if (MaintenanceInterval <= 0.0f) return false;
    return TimeSinceLastMaintenance >= MaintenanceInterval;
}

// ============================================================================
// INTERFACE IMPLEMENTATIONS
// ============================================================================

FGameplayTag UDeviceStateComponent::GetDeviceState_Implementation() const
{
    return CurrentState;
}

void UDeviceStateComponent::SetDeviceState_Implementation(FGameplayTag NewState)
{
    SetDeviceState(NewState);
}

bool UDeviceStateComponent::IsUsable_Implementation() const
{
    return IsUsable();
}

void UDeviceStateComponent::TurnOn_Implementation()
{
    TurnOn();
}

void UDeviceStateComponent::TurnOff_Implementation()
{
    TurnOff();
}

bool UDeviceStateComponent::IsPoweredOn_Implementation() const
{
    return bIsPoweredOn;
}

bool UDeviceStateComponent::NeedsMaintenance_Implementation() const
{
    return NeedsMaintenance();
}

void UDeviceStateComponent::PerformMaintenance_Implementation()
{
    PerformMaintenance();
}

UActorComponent* UDeviceStateComponent::GetDeviceComponent_Implementation()
{
    return this;
}

// ============================================================================
// SERVER RPCs
// ============================================================================

bool UDeviceStateComponent::Server_TurnOn_Validate()
{
    return true;
}

void UDeviceStateComponent::Server_TurnOn_Implementation()
{
    Internal_TurnOn();
}

bool UDeviceStateComponent::Server_TurnOff_Validate()
{
    return true;
}

void UDeviceStateComponent::Server_TurnOff_Implementation()
{
    Internal_TurnOff();
}

bool UDeviceStateComponent::Server_SetState_Validate(FGameplayTag NewState)
{
    return NewState.IsValid();
}

void UDeviceStateComponent::Server_SetState_Implementation(FGameplayTag NewState)
{
    Internal_SetState(NewState);
}

bool UDeviceStateComponent::Server_StartUse_Validate(AActor* User)
{
    return User != nullptr;
}

void UDeviceStateComponent::Server_StartUse_Implementation(AActor* User)
{
    Internal_StartUse(User);
}

bool UDeviceStateComponent::Server_StopUse_Validate(AActor* User)
{
    return User != nullptr;
}

void UDeviceStateComponent::Server_StopUse_Implementation(AActor* User)
{
    Internal_StopUse(User);
}

bool UDeviceStateComponent::Server_PerformMaintenance_Validate()
{
    return true;
}

void UDeviceStateComponent::Server_PerformMaintenance_Implementation()
{
    Internal_PerformMaintenance();
}

// ============================================================================
// INTERNALS
// ============================================================================

void UDeviceStateComponent::Internal_SetState(FGameplayTag NewState)
{
    if (CurrentState == NewState) return;
    
    FGameplayTag OldState = CurrentState;
    CurrentState = NewState;
    
    OnDeviceStateChanged.Broadcast(OldState, NewState);
}

void UDeviceStateComponent::Internal_TurnOn()
{
    if (IsBroken()) return;
    
    bool bWasPoweredOn = bIsPoweredOn;
    bIsPoweredOn = true;
    
    if (bWasPoweredOn != bIsPoweredOn)
    {
        OnDevicePowerChanged.Broadcast(bIsPoweredOn);
    }
    
    // Transition to idle if was off
    if (CurrentState == FWWTagLibrary::Simulator_Device_State_Off() ||
        CurrentState == FWWTagLibrary::Simulator_Device_State_NoPower())
    {
        Internal_SetState(FWWTagLibrary::Simulator_Device_State_Idle());
    }
}

void UDeviceStateComponent::Internal_TurnOff()
{
    // Stop use first if in use
    if (IsInUse() && CurrentUser)
    {
        Internal_StopUse(CurrentUser);
    }
    
    bool bWasPoweredOn = bIsPoweredOn;
    bIsPoweredOn = false;
    
    if (bWasPoweredOn != bIsPoweredOn)
    {
        OnDevicePowerChanged.Broadcast(bIsPoweredOn);
    }
    
    Internal_SetState(FWWTagLibrary::Simulator_Device_State_Off());
}

bool UDeviceStateComponent::Internal_StartUse(AActor* User)
{
    if (!IsUsable()) return false;
    if (!User) return false;
    
    CurrentUser = User;
    Internal_SetState(FWWTagLibrary::Simulator_Device_State_InUse());
    
    // Apply durability loss if configured
    if (DurabilityComponent && DurabilityLossPerUse > 0.0f)
    {
        IDurabilityInterface::Execute_ApplyDurabilityLoss(DurabilityComponent, DurabilityLossPerUse, User);
    }
    
    return true;
}

void UDeviceStateComponent::Internal_StopUse(AActor* User)
{
    if (!IsInUse()) return;
    if (CurrentUser != User) return;
    
    CurrentUser = nullptr;
    
    // Return to idle if powered, off if not
    if (bIsPoweredOn)
    {
        Internal_SetState(FWWTagLibrary::Simulator_Device_State_Idle());
    }
    else
    {
        Internal_SetState(FWWTagLibrary::Simulator_Device_State_Off());
    }
}

void UDeviceStateComponent::Internal_PerformMaintenance()
{
    TimeSinceLastMaintenance = 0.0f;
    bMaintenanceRequired = false;
    
    // If was in maintenance state, return to idle
    if (CurrentState == FWWTagLibrary::Simulator_Device_State_Maintenance())
    {
        if (bIsPoweredOn)
        {
            Internal_SetState(FWWTagLibrary::Simulator_Device_State_Idle());
        }
        else
        {
            Internal_SetState(FWWTagLibrary::Simulator_Device_State_Off());
        }
    }
    
    OnMaintenanceComplete.Broadcast();
}

void UDeviceStateComponent::CheckDurabilityComponent()
{
    if (!GetOwner()) return;
    DurabilityComponent = GetOwner()->FindComponentByClass<UDurabilityComponent>();
}

void UDeviceStateComponent::OnDurabilityBroken(AActor* Causer)
{
    // Stop use if in use
    if (IsInUse() && CurrentUser)
    {
        Internal_StopUse(CurrentUser);
    }
    
    Internal_SetState(FWWTagLibrary::Simulator_Device_State_Broken());
}

void UDeviceStateComponent::UpdateMaintenanceTimer(float DeltaTime)
{
    if (MaintenanceInterval <= 0.0f) return;
    if (!bIsPoweredOn) return;
    
    TimeSinceLastMaintenance += DeltaTime;
    
    if (!bMaintenanceRequired && TimeSinceLastMaintenance >= MaintenanceInterval)
    {
        bMaintenanceRequired = true;
        OnMaintenanceRequired.Broadcast();
    }
}

// ============================================================================
// ONREPS
// ============================================================================

void UDeviceStateComponent::OnRep_CurrentState(FGameplayTag OldState)
{
    OnDeviceStateChanged.Broadcast(OldState, CurrentState);
}

void UDeviceStateComponent::OnRep_IsPoweredOn()
{
    OnDevicePowerChanged.Broadcast(bIsPoweredOn);
}