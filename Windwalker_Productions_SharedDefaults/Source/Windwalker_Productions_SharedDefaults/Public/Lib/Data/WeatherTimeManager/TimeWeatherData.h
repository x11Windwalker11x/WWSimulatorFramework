// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "TimeWeatherData.generated.h"

/**
 * Current time-of-day state snapshot
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FTimeOfDayState
{
	GENERATED_BODY()

	/** Current hour as float (0.0 - 24.0), e.g. 13.5 = 1:30 PM */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	float CurrentHour = 6.0f;

	/** Current day number (starts at 1) */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	int32 DayNumber = 1;

	/** Speed multiplier for time progression (1.0 = real-time ratio) */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0"))
	float TimeSpeedMultiplier = 1.0f;

	/** Whether time progression is paused */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	bool bTimePaused = false;

	/** Current time-of-day period tag (Time.State.Dawn, Time.State.Morning, etc.) */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	FGameplayTag TimeOfDayTag;

	bool IsValid() const { return CurrentHour >= 0.0f && CurrentHour < 24.0f && DayNumber > 0; }
};

/**
 * Weather type configuration -- designed for DataTable loading
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWeatherConfig : public FTableRowBase
{
	GENERATED_BODY()

	/** Weather type tag (Weather.Type.Clear, Weather.Type.Rain, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FGameplayTag WeatherTag;

	/** Duration in seconds for transitioning TO this weather */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0"))
	float TransitionDuration = 5.0f;

	/** Base intensity (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Intensity = 1.0f;

	/** Tags of weather types this can transition to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FGameplayTagContainer AllowedTransitions;

	bool IsValid() const { return WeatherTag.IsValid(); }
};

/**
 * Runtime weather state snapshot
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWeatherState
{
	GENERATED_BODY()

	/** Current active weather tag */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FGameplayTag CurrentWeatherTag;

	/** Target weather tag during transition */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FGameplayTag TargetWeatherTag;

	/** Transition progress (0.0 = current, 1.0 = target reached) */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TransitionAlpha = 0.0f;

	/** Whether a weather transition is currently in progress */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather")
	bool bTransitioning = false;

	/** Current weather intensity (0.0 - 1.0) */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CurrentIntensity = 1.0f;

	bool IsValid() const { return CurrentWeatherTag.IsValid(); }
};

/**
 * Maps a time-of-day tag to an hour range
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FTimeThreshold
{
	GENERATED_BODY()

	/** Time period tag (Time.State.Dawn, Time.State.Morning, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	FGameplayTag TimeOfDayTag;

	/** Start hour (inclusive, 0.0 - 24.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float StartHour = 0.0f;

	/** End hour (exclusive, 0.0 - 24.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float EndHour = 0.0f;

	bool IsValid() const { return TimeOfDayTag.IsValid(); }
};
