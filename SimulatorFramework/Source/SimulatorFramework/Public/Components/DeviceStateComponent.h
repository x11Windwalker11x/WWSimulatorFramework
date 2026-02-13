#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SimulatorFramework/DeviceInterface.h"
#include "Interfaces/ModularSaveGameSystem/SaveableInterface.h"
#include "DeviceStateComponent.generated.h"

class UDurabilityComponent;

// === DELEGATES ===

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeviceStateChanged, FGameplayTag, OldState, FGameplayTag, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDevicePowerChanged, bool, bIsPoweredOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeviceMaintenanceRequired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeviceMaintenanceComplete);

UCLASS(ClassGroup=(SimulatorFramework), meta=(BlueprintSpawnableComponent))
class SIMULATORFRAMEWORK_API UDeviceStateComponent : public UActorComponent, public IDeviceInterface, public ISaveableInterface
{
    GENERATED_BODY()

public:
    UDeviceStateComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Config")
    FGameplayTag DefaultState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Config")
    bool bRequiresPower = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Config", meta = (ClampMin = "0.0"))
    float MaintenanceInterval = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DurabilityLossPerUse = 0.0f;

    // ============================================================================
    // PUBLIC API
    // ============================================================================

    UFUNCTION(BlueprintCallable, Category = "Device")
    void TurnOn();

    UFUNCTION(BlueprintCallable, Category = "Device")
    void TurnOff();

    UFUNCTION(BlueprintCallable, Category = "Device")
    void SetDeviceState(FGameplayTag NewState);

    UFUNCTION(BlueprintCallable, Category = "Device")
    bool StartUse(AActor* User);

    UFUNCTION(BlueprintCallable, Category = "Device")
    void StopUse(AActor* User);

    UFUNCTION(BlueprintCallable, Category = "Device")
    void PerformMaintenance();

    // ============================================================================
    // QUERIES
    // ============================================================================

    UFUNCTION(BlueprintPure, Category = "Device")
    FGameplayTag GetDeviceState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Device")
    bool IsPoweredOn() const { return bIsPoweredOn; }

    UFUNCTION(BlueprintPure, Category = "Device")
    bool IsUsable() const;

    UFUNCTION(BlueprintPure, Category = "Device")
    bool IsInUse() const;

    UFUNCTION(BlueprintPure, Category = "Device")
    bool IsBroken() const;

    UFUNCTION(BlueprintPure, Category = "Device")
    bool NeedsMaintenance() const;

    UFUNCTION(BlueprintPure, Category = "Device")
    AActor* GetCurrentUser() const { return CurrentUser; }

    // ============================================================================
    // INTERFACE IMPLEMENTATIONS
    // ============================================================================

    virtual FGameplayTag GetDeviceState_Implementation() const override;
    virtual void SetDeviceState_Implementation(FGameplayTag NewState) override;
    virtual bool IsUsable_Implementation() const override;
    virtual void TurnOn_Implementation() override;
    virtual void TurnOff_Implementation() override;
    virtual bool IsPoweredOn_Implementation() const override;
    virtual bool NeedsMaintenance_Implementation() const override;
    virtual void PerformMaintenance_Implementation() override;
    virtual UActorComponent* GetDeviceComponent_Implementation() override;

    // ============================================================================
    // EVENTS
    // ============================================================================

    UPROPERTY(BlueprintAssignable, Category = "Device|Events")
    FOnDeviceStateChanged OnDeviceStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Device|Events")
    FOnDevicePowerChanged OnDevicePowerChanged;

    UPROPERTY(BlueprintAssignable, Category = "Device|Events")
    FOnDeviceMaintenanceRequired OnMaintenanceRequired;

    UPROPERTY(BlueprintAssignable, Category = "Device|Events")
    FOnDeviceMaintenanceComplete OnMaintenanceComplete;

    // ============================================================================
    // SERVER RPCs
    // ============================================================================

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TurnOn();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TurnOff();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetState(FGameplayTag NewState);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_StartUse(AActor* User);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_StopUse(AActor* User);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_PerformMaintenance();

protected:
    // ============================================================================
    // REPLICATED STATE
    // ============================================================================

    UPROPERTY(SaveGame, ReplicatedUsing = OnRep_CurrentState)
    FGameplayTag CurrentState;

    UPROPERTY(SaveGame, ReplicatedUsing = OnRep_IsPoweredOn)
    bool bIsPoweredOn = false;

    UPROPERTY(Replicated)
    AActor* CurrentUser = nullptr;

    UPROPERTY(SaveGame, Replicated)
    float TimeSinceLastMaintenance = 0.0f;

    UPROPERTY()
    bool bMaintenanceRequired = false;

    UFUNCTION()
    void OnRep_CurrentState(FGameplayTag OldState);

    UFUNCTION()
    void OnRep_IsPoweredOn();

    // ============================================================================
    // OPTIONAL COMPONENTS
    // ============================================================================

    UPROPERTY()
    UDurabilityComponent* DurabilityComponent = nullptr;

    // ============================================================================
    // INTERNALS
    // ============================================================================

    void Internal_SetState(FGameplayTag NewState);
    void Internal_TurnOn();
    void Internal_TurnOff();
    bool Internal_StartUse(AActor* User);
    void Internal_StopUse(AActor* User);
    void Internal_PerformMaintenance();

    void CheckDurabilityComponent();
    
    UFUNCTION()
    void OnDurabilityBroken(AActor* Causer);
    
    void UpdateMaintenanceTimer(float DeltaTime);

    // ============================================================================
    // SAVE SYSTEM (ISaveableInterface)
    // ============================================================================

    virtual FString GetSaveID_Implementation() const override;
    virtual int32 GetSavePriority_Implementation() const override;
    virtual FGameplayTag GetSaveType_Implementation() const override;
    virtual bool SaveState_Implementation(FSaveRecord& OutRecord) override;
    virtual bool LoadState_Implementation(const FSaveRecord& InRecord) override;
    virtual bool IsDirty_Implementation() const override;
    virtual void ClearDirty_Implementation() override;
    virtual void OnSaveDataLoaded_Implementation() override;

    bool bSaveDirty = false;
    void MarkSaveDirty();
};