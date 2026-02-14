// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/WeatherTimeManager/TimeWeatherDelegates.h"
#include "Lib/Data/WeatherTimeManager/TimeWeatherData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "TimeTrackingSubsystem.generated.h"

/**
 * Time Tracking Subsystem
 *
 * Central time-of-day and weather manager. Handles:
 * - Time progression with configurable speed multiplier
 * - Day/night cycle with 6 time-of-day periods
 * - Weather state with smooth transitions
 * - Sky provider registration for visual sync
 * - Console commands for debug/cheat
 *
 * Uses FTimerHandle at 10Hz (no tick). Server-authoritative.
 */
UCLASS()
class WEATHERTIMEMANAGER_API UTimeTrackingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UTimeTrackingSubsystem();

	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Static accessor */
	UFUNCTION(BlueprintPure, Category = "TimeWeather", meta = (WorldContext = "WorldContextObject"))
	static UTimeTrackingSubsystem* Get(const UObject* WorldContextObject);

	// ============================================================================
	// DELEGATES
	// ============================================================================

	/** Fires when the integer hour changes */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Delegates")
	FOnHourChanged OnHourChanged;

	/** Fires when the day number increments */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Delegates")
	FOnDayChanged OnDayChanged;

	/** Fires when the time-of-day period changes */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Delegates")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	/** Fires when weather changes (after transition completes) */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Delegates")
	FOnWeatherChanged OnWeatherChanged;

	/** Fires when a weather transition begins */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Delegates")
	FOnWeatherTransitionStarted OnWeatherTransitionStarted;

	/** Fires when a weather transition completes */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Delegates")
	FOnWeatherTransitionComplete OnWeatherTransitionComplete;

	/** Fires when time is paused */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Delegates")
	FOnTimePaused OnTimePaused;

	/** Fires when time is resumed */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Delegates")
	FOnTimeResumed OnTimeResumed;

	// ============================================================================
	// TIME API
	// ============================================================================

	/** Get full time state snapshot */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Time")
	FTimeOfDayState GetTimeState() const { return TimeState; }

	/** Get current hour (0.0 - 24.0) */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Time")
	float GetCurrentHour() const { return TimeState.CurrentHour; }

	/** Get current day number */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Time")
	int32 GetCurrentDay() const { return TimeState.DayNumber; }

	/** Get current time-of-day tag */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Time")
	FGameplayTag GetTimeOfDayTag() const { return TimeState.TimeOfDayTag; }

	/** Set time-of-day directly (0.0 - 24.0) */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Time")
	void SetTimeOfDay(float Hour);

	/** Set time speed multiplier */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Time")
	void SetTimeSpeed(float Multiplier);

	/** Get time speed multiplier */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Time")
	float GetTimeSpeed() const { return TimeState.TimeSpeedMultiplier; }

	/** Pause time progression */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Time")
	void PauseTime();

	/** Resume time progression */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Time")
	void ResumeTime();

	/** Is time currently paused? */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Time")
	bool IsTimePaused() const { return TimeState.bTimePaused; }

	/** Start automatic time progression */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Time")
	void StartTimeProgression();

	/** Stop automatic time progression */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Time")
	void StopTimeProgression();

	/** Is time progression running? */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Time")
	bool IsTimeProgressionActive() const { return bTimeProgressionActive; }

	// ============================================================================
	// WEATHER API
	// ============================================================================

	/** Get full weather state snapshot */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Weather")
	FWeatherState GetWeatherState() const { return WeatherState; }

	/** Get current weather tag */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Weather")
	FGameplayTag GetCurrentWeatherTag() const { return WeatherState.CurrentWeatherTag; }

	/** Is a weather transition in progress? */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Weather")
	bool IsWeatherTransitioning() const { return WeatherState.bTransitioning; }

	/** Set weather immediately (no transition) */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Weather")
	void SetWeatherImmediate(FGameplayTag WeatherTag, float Intensity = 1.0f);

	/** Start a smooth weather transition */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Weather")
	void TransitionToWeather(FGameplayTag TargetWeather, float Duration = 5.0f, float TargetIntensity = 1.0f);

	/** Cancel current weather transition */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Weather")
	void CancelWeatherTransition();

	// ============================================================================
	// PROVIDER API
	// ============================================================================

	/** Register a sky/atmosphere provider actor */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Provider")
	void RegisterSkyProvider(AActor* Provider);

	/** Unregister a sky provider */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Provider")
	void UnregisterSkyProvider(AActor* Provider);

	/** Auto-discover all actors implementing ITimeWeatherProviderInterface */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Provider")
	void DiscoverSkyProviders();

	// ============================================================================
	// THRESHOLDS
	// ============================================================================

	/** Get all time thresholds (editable via Blueprint) */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Time")
	const TArray<FTimeThreshold>& GetTimeThresholds() const { return TimeThresholds; }

private:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	/** Current time state */
	FTimeOfDayState TimeState;

	/** Current weather state */
	FWeatherState WeatherState;

	/** Time-of-day period thresholds */
	UPROPERTY()
	TArray<FTimeThreshold> TimeThresholds;

	/** Registered sky provider actors */
	TArray<TWeakObjectPtr<AActor>> SkyProviders;

	/** Whether time progression is active */
	bool bTimeProgressionActive = false;

	/** Timer handle for time progression (10Hz) */
	FTimerHandle TimeProgressionHandle;

	/** Weather transition duration in seconds */
	float WeatherTransitionDuration = 5.0f;

	/** Weather transition elapsed time */
	float WeatherTransitionElapsed = 0.0f;

	/** Target intensity for weather transition */
	float WeatherTargetIntensity = 1.0f;

	/** Cached previous integer hour (for OnHourChanged detection) */
	int32 CachedHour = -1;

	/** Timer tick interval (10Hz = 0.1s) */
	static constexpr float TickInterval = 0.1f;

	/** Default game-hours per real-minute (1 game-hour = 1 real-minute by default) */
	static constexpr float DefaultGameHoursPerRealMinute = 1.0f;

	// ============================================================================
	// INTERNAL HELPERS
	// ============================================================================

	/** Called by timer every 0.1s */
	void OnTimerTick();

	/** Advance time by delta seconds */
	void AdvanceTime(float DeltaSeconds);

	/** Update weather transition state */
	void UpdateWeatherTransition(float DeltaSeconds);

	/** Evaluate time thresholds and fire period change */
	void EvaluateTimeOfDayPeriod();

	/** Push current state to all registered sky providers */
	void PushStateToProviders();

	/** Initialize default 6-period thresholds */
	void InitDefaultThresholds();

	/** Clean stale provider references */
	void CleanStaleProviders();

	/** Get world for timer access */
	UWorld* GetWorldForTimers() const;

	// ============================================================================
	// CONSOLE COMMANDS
	// ============================================================================

	/** Console command: WW.SetTime <Hour> */
	static void CmdSetTime(const TArray<FString>& Args, UWorld* World);

	/** Console command: WW.SetSpeed <Multiplier> */
	static void CmdSetSpeed(const TArray<FString>& Args, UWorld* World);

	/** Console command: WW.PauseTime */
	static void CmdPauseTime(const TArray<FString>& Args, UWorld* World);

	/** Console command: WW.SetWeather <WeatherTag> */
	static void CmdSetWeather(const TArray<FString>& Args, UWorld* World);
};
