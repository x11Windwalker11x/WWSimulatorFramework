// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/ModularEconomyPlugin/EconomyInterface.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "ResourceConsumerComponent.generated.h"

class UEconomySubsystem;

/**
 * Resource Consumer Component
 *
 * Attach to any actor to make it a resource consumer.
 * Implements IEconomyInterface so the EconomySubsystem can poll consumption.
 *
 * DeviceState integration (poll-based, no L2 dep):
 * If bAutoBindToDeviceState is true and the owner implements IDeviceInterface,
 * IsConsuming returns true only when BOTH bManualActive AND IsPoweredOn.
 *
 * Rule #13: Replicated. Rule #41: Caches subsystem ref.
 */
UCLASS(ClassGroup = (Economy), meta = (BlueprintSpawnableComponent))
class MODULARECONOMYPLUGIN_API UResourceConsumerComponent : public UActorComponent, public IEconomyInterface
{
	GENERATED_BODY()

public:
	UResourceConsumerComponent();

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Resource type this consumer uses (Economy.Resource.*) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy|Resource", meta = (Categories = "Economy.Resource"))
	FGameplayTag ResourceType;

	/** Units consumed per game-hour */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy|Resource", meta = (ClampMin = "0.0"))
	float UnitsPerHour = 1.f;

	/** Cost per unit consumed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy|Resource", meta = (ClampMin = "0.0"))
	float CostPerUnit = 1.f;

	/** If true, queries IDeviceInterface::IsPoweredOn on owner to gate consumption */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy|Resource")
	bool bAutoBindToDeviceState = true;

	/** Whether consumption starts active on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy|Resource")
	bool bStartActive = false;

	// ============================================================================
	// API
	// ============================================================================

	/** Set manual active state (server-authoritative) */
	UFUNCTION(BlueprintCallable, Category = "Economy|Resource")
	void SetActive(bool bActive);

	/** Get manual active state */
	UFUNCTION(BlueprintPure, Category = "Economy|Resource")
	bool IsActive() const { return bManualActive; }

	/** Set consumption rate at runtime */
	UFUNCTION(BlueprintCallable, Category = "Economy|Resource")
	void SetConsumptionRate(float NewUnitsPerHour, float NewCostPerUnit);

	// ============================================================================
	// SERVER RPCs (Rule #13-14)
	// ============================================================================

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetActive(bool bActive);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetConsumptionRate(float NewUnitsPerHour, float NewCostPerUnit);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================================
	// IEconomyInterface Implementation
	// ============================================================================

	virtual FGameplayTag GetResourceType_Implementation() const override;
	virtual float GetResourceConsumptionRate_Implementation() const override;
	virtual bool IsConsuming_Implementation() const override;
	virtual float GetCostPerHour_Implementation() const override;
	virtual void SetConsumptionActive_Implementation(bool bActive) override;
	virtual UActorComponent* GetEconomyComponent_Implementation() override;

private:
	/** Manual active state (replicated) */
	UPROPERTY(Replicated)
	bool bManualActive = false;

	/** Cached subsystem reference (Rule #41) */
	UPROPERTY()
	TObjectPtr<UEconomySubsystem> CachedEconomySubsystem;

	/** Whether the owner implements IDeviceInterface (cached on BeginPlay) */
	bool bOwnerHasDeviceInterface = false;
};
