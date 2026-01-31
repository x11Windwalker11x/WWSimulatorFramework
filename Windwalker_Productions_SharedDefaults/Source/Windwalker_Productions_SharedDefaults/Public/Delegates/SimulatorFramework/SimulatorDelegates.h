#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SimulatorDelegates.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnDurabilityChanged,
	AActor*, Device,
	float, OldValue,
	float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnDeviceStateChanged,
	AActor*, Device,
	FGameplayTag, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnManipulationStarted,
	APawn*, Manipulator,
	AActor*, Target);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnManipulatiodEnded,
	APawn*, Manipulator,
	AActor*, Target);

