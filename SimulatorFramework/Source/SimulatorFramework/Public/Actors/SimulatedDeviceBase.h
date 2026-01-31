#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SimulatorFramework/DeviceInterface.h"
#include "Interfaces/ModularInteractionSystem/InteractableInterface.h"
#include "SimulatedDeviceBase.generated.h"

class UDeviceStateComponent;
class UDurabilityComponent;
class UInteractableComponent;
class UStaticMeshComponent;

/**
 * Base actor for all simulated devices (PCs, stations, machines).
 * Thin orchestration layer - routes to components.
 * Extend in C++ or Blueprint for specific devices.
 */
UCLASS(Abstract, Blueprintable)
class SIMULATORFRAMEWORK_API ASimulatedDeviceBase : public AActor, public IDeviceInterface, public IInteractableInterface
{
    GENERATED_BODY()

public:
    ASimulatedDeviceBase();

    virtual void BeginPlay() override;

    // ============================================================================
    // COMPONENTS
    // ============================================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDeviceStateComponent* DeviceStateComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDurabilityComponent* DurabilityComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInteractableComponent* InteractableComponent;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Config")
    FName DeviceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Config")
    FText DeviceDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Config")
    bool bHasDurability = true;

    // ============================================================================
    // IDEVICEINTERFACE IMPLEMENTATION
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
    // IINTERACTABLEINTERFACE IMPLEMENTATION
    // ============================================================================

    virtual void OnInteract_Implementation(AController* InstigatorController) override;
    virtual bool IsCurrentlyInteractable_Implementation() override;
    virtual void SetInteractionEnabled_Implementation(bool bEnabled) override;
    virtual bool GetInteractionEnabled_Implementation() override;

protected:
    // ============================================================================
    // BLUEPRINT EVENTS
    // ============================================================================

    /** Called when device is used via interaction */
    UFUNCTION(BlueprintImplementableEvent, Category = "Device")
    void OnDeviceUsed(AActor* User);

    /** Called when device state changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Device")
    void OnDeviceStateChanged(FGameplayTag OldState, FGameplayTag NewState);

    /** Called when device breaks */
    UFUNCTION(BlueprintImplementableEvent, Category = "Device")
    void OnDeviceBroken();

    // ============================================================================
    // INTERNAL BINDINGS
    // ============================================================================

    UFUNCTION()
    void HandleDeviceStateChanged(FGameplayTag OldState, FGameplayTag NewState);

    UFUNCTION()
    void HandleDeviceBroken(AActor* Causer);
};