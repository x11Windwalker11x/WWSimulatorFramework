// Copyright Windwalker Productions. All Rights Reserved.

#include "Components/ResourceConsumerComponent.h"
#include "Subsystems/EconomySubsystem.h"
#include "Interfaces/SimulatorFramework/DeviceInterface.h"
#include "Net/UnrealNetwork.h"

UResourceConsumerComponent::UResourceConsumerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UResourceConsumerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Rule #41: Cache subsystem reference
	CachedEconomySubsystem = UEconomySubsystem::Get(this);

	// Cache whether owner implements IDeviceInterface
	if (const AActor* Owner = GetOwner())
	{
		bOwnerHasDeviceInterface = Owner->GetClass()->ImplementsInterface(UDeviceInterface::StaticClass());
	}

	// Set initial active state
	bManualActive = bStartActive;

	// Register with subsystem
	if (CachedEconomySubsystem)
	{
		CachedEconomySubsystem->RegisterConsumer(GetOwner());
	}
}

void UResourceConsumerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from subsystem
	if (CachedEconomySubsystem)
	{
		CachedEconomySubsystem->UnregisterConsumer(GetOwner());
	}

	Super::EndPlay(EndPlayReason);
}

void UResourceConsumerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UResourceConsumerComponent, bManualActive);
}

// ============================================================================
// API
// ============================================================================

void UResourceConsumerComponent::SetActive(bool bActive)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		bManualActive = bActive;
	}
	else
	{
		Server_SetActive(bActive);
	}
}

void UResourceConsumerComponent::SetConsumptionRate(float NewUnitsPerHour, float NewCostPerUnit)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UnitsPerHour = FMath::Max(0.f, NewUnitsPerHour);
		CostPerUnit = FMath::Max(0.f, NewCostPerUnit);
	}
	else
	{
		Server_SetConsumptionRate(NewUnitsPerHour, NewCostPerUnit);
	}
}

// ============================================================================
// SERVER RPCs
// ============================================================================

void UResourceConsumerComponent::Server_SetActive_Implementation(bool bActive)
{
	bManualActive = bActive;
}

bool UResourceConsumerComponent::Server_SetActive_Validate(bool bActive)
{
	return true;
}

void UResourceConsumerComponent::Server_SetConsumptionRate_Implementation(float NewUnitsPerHour, float NewCostPerUnit)
{
	UnitsPerHour = FMath::Max(0.f, NewUnitsPerHour);
	CostPerUnit = FMath::Max(0.f, NewCostPerUnit);
}

bool UResourceConsumerComponent::Server_SetConsumptionRate_Validate(float NewUnitsPerHour, float NewCostPerUnit)
{
	return NewUnitsPerHour >= 0.f && NewCostPerUnit >= 0.f;
}

// ============================================================================
// IEconomyInterface Implementation
// ============================================================================

FGameplayTag UResourceConsumerComponent::GetResourceType_Implementation() const
{
	return ResourceType;
}

float UResourceConsumerComponent::GetResourceConsumptionRate_Implementation() const
{
	return UnitsPerHour;
}

bool UResourceConsumerComponent::IsConsuming_Implementation() const
{
	if (!bManualActive)
	{
		return false;
	}

	// If auto-bind is enabled and owner has IDeviceInterface, gate on power state
	if (bAutoBindToDeviceState && bOwnerHasDeviceInterface)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			return IDeviceInterface::Execute_IsPoweredOn(Owner);
		}
		return false;
	}

	return true;
}

float UResourceConsumerComponent::GetCostPerHour_Implementation() const
{
	return UnitsPerHour * CostPerUnit;
}

void UResourceConsumerComponent::SetConsumptionActive_Implementation(bool bActive)
{
	SetActive(bActive);
}

UActorComponent* UResourceConsumerComponent::GetEconomyComponent_Implementation()
{
	return this;
}
