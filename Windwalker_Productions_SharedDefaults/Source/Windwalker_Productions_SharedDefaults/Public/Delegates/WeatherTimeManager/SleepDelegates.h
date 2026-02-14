// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SleepDelegates.generated.h"

class APlayerState;

/** Fired when a player requests sleep */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSleepRequested,
	APlayerState*, RequestingPlayer,
	float, TargetWakeHour);

/** Fired when sleep actually begins (time speed changed) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSleepStarted,
	float, TargetWakeHour);

/** Fired when sleep completes (target hour reached) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSleepCompleted,
	float, HoursSlept);

/** Fired when sleep is cancelled */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSleepCancelled,
	APlayerState*, CancellingPlayer);

/** Fired when MP vote state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnSleepVoteChanged,
	int32, VotesFor,
	int32, VotesAgainst,
	int32, TotalPlayers);
