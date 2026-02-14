// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TimeWeatherDelegates.generated.h"

/** Fires when the integer hour changes (e.g. 5 -> 6) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHourChanged,
	int32, OldHour,
	int32, NewHour);

/** Fires when the day number increments */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnDayChanged,
	int32, OldDay,
	int32, NewDay);

/** Fires when the time-of-day period changes (e.g. Dawn -> Morning) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnTimeOfDayChanged,
	const FGameplayTag&, OldPeriod,
	const FGameplayTag&, NewPeriod);

/** Fires when weather changes (after transition completes) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWeatherChanged,
	const FGameplayTag&, OldWeather,
	const FGameplayTag&, NewWeather);

/** Fires when a weather transition begins */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWeatherTransitionStarted,
	const FGameplayTag&, FromWeather,
	const FGameplayTag&, ToWeather);

/** Fires when a weather transition completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnWeatherTransitionComplete,
	const FGameplayTag&, NewWeather);

/** Fires when time is paused */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimePaused);

/** Fires when time is resumed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeResumed);
