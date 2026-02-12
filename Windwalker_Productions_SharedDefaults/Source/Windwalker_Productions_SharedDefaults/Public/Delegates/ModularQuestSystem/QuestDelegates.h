// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestDelegates.generated.h"

/** Fires when a quest state changes for any player (Old/New as FGameplayTag, no enums) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnQuestStateChanged,
	FName, QuestID,
	const FGameplayTag&, OldState,
	const FGameplayTag&, NewState,
	AActor*, Player);

/** Fires when a quest is accepted */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnQuestAccepted,
	FName, QuestID,
	AActor*, Player);

/** Fires when a quest is completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnQuestCompleted,
	FName, QuestID,
	bool, bBonusAchieved,
	AActor*, Player);

/** Fires when a quest is failed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnQuestFailed,
	FName, QuestID,
	AActor*, Player);

/** Fires when quest rewards are ready to be claimed (turn-in) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnQuestTurnedIn,
	FName, QuestID,
	AActor*, Player);

/** Fires when a quest becomes available (prerequisites met) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnQuestAvailable,
	FName, QuestID,
	AActor*, Player);

/** Fires when a quest objective progresses (forwarded from ObjectiveTracker) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnQuestObjectiveProgress,
	FName, QuestID,
	const FGameplayTag&, ObjectiveTag,
	float, NewValue,
	AActor*, Player);
