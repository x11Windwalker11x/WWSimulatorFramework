#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InteractionDelegates.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInteractionStarted,
	AActor*, Interactor,
	AActor*, Ineteractable);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnInteractionEnded,
	AActor*, Interactor,
	AActor*, Interactable,
	bool, bCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFocusChanged,
	FGameplayTag, OldState,
	FGameplayTag, Newtate);

//Hold Interaction Tick, range : 0.0 - 1.0
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInteractionProgress,
	AActor*, Interactable,
	float, Progress);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnInteractableStateChanged,
	AActor*, Interactable,
	FGameplayTag, OldInteractionState,
	FGameplayTag, NewInteractionState,
	);