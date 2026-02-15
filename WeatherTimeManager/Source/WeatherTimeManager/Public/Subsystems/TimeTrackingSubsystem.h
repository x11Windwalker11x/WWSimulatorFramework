// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/WeatherTimeManager/TimeWeatherDelegates.h"
#include "Delegates/WeatherTimeManager/SleepDelegates.h"
#include "Lib/Data/WeatherTimeManager/TimeWeatherData.h"
#include "Lib/Data/WeatherTimeManager/SleepData.h"
#include "Interfaces/ModularSaveGameSystem/SaveableInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "TimeTrackingSubsystem.generated.h"

class ASleepManagerAuthority;
class UDaySummarySubsystem;
class UDaySummaryWidget_Base;

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
class WEATHERTIMEMANAGER_API UTimeTrackingSubsystem : public UGameInstanceSubsystem, public ISaveableInterface
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
	// SLEEP DELEGATES
	// ============================================================================

	/** Fires when a player requests sleep */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Sleep|Delegates")
	FOnSleepRequested OnSleepRequested;

	/** Fires when sleep begins (time speed changed) */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Sleep|Delegates")
	FOnSleepStarted OnSleepStarted;

	/** Fires when sleep completes (target hour reached) */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Sleep|Delegates")
	FOnSleepCompleted OnSleepCompleted;

	/** Fires when sleep is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Sleep|Delegates")
	FOnSleepCancelled OnSleepCancelled;

	/** Fires when MP vote state changes */
	UPROPERTY(BlueprintAssignable, Category = "TimeWeather|Sleep|Delegates")
	FOnSleepVoteChanged OnSleepVoteChanged;

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

	// ============================================================================
	// SLEEP API
	// ============================================================================

	/**
	 * Request sleep until target hour.
	 * SP: immediate start. MP: initiates vote via ASleepManagerAuthority.
	 * @param TargetWakeHour Target hour to wake (0-24)
	 * @param SpeedMultiplier Time speed during sleep (default 60x)
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Sleep")
	void RequestSleep(float TargetWakeHour, float SpeedMultiplier = 60.0f);

	/**
	 * Cancel active sleep. Time keeps what it advanced (Stardew model).
	 * @param CancellingPlayer Player who cancelled (nullptr for SP)
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Sleep")
	void CancelSleep(APlayerState* CancellingPlayer = nullptr);

	/** Is a sleep operation currently active? */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Sleep")
	bool IsSleeping() const;

	/** Get sleep progress (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Sleep")
	float GetSleepProgress() const;

	/** Get current sleep request data */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Sleep")
	const FSleepRequest& GetSleepRequest() const { return SleepRequest; }

	/** Get current sleep vote state */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Sleep")
	const FSleepVoteState& GetSleepVoteState() const { return SleepVoteState; }

	/** Cast a vote for/against sleep (MP) */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|Sleep")
	void CastSleepVote(int32 PlayerID, bool bApprove);

	/** Called by ASleepManagerAuthority to begin sleep after vote approval */
	void BeginSleep();

	/** Get cached sleep manager authority reference */
	ASleepManagerAuthority* GetSleepAuthority() const { return SleepAuthority.Get(); }

	/** Set sleep manager authority reference (called by authority on spawn) */
	void SetSleepAuthority(ASleepManagerAuthority* Authority) { SleepAuthority = Authority; }

	// ============================================================================
	// DAY SUMMARY API
	// ============================================================================

	/**
	 * Show the day-end summary screen.
	 * Triggers UDaySummarySubsystem::RequestDaySummary, creates widget, submits sleep entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeWeather|DaySummary")
	void ShowDaySummary();

	/** Widget class to spawn for day summary (set in Blueprint) */
	UPROPERTY(EditDefaultsOnly, Category = "TimeWeather|DaySummary")
	TSubclassOf<UDaySummaryWidget_Base> DaySummaryWidgetClass;

private:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	/** Current time state */
	UPROPERTY(SaveGame)
	FTimeOfDayState TimeState;

	/** Current weather state */
	UPROPERTY(SaveGame)
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
	UPROPERTY(SaveGame)
	float WeatherTransitionDuration = 5.0f;

	/** Weather transition elapsed time */
	UPROPERTY(SaveGame)
	float WeatherTransitionElapsed = 0.0f;

	/** Target intensity for weather transition */
	UPROPERTY(SaveGame)
	float WeatherTargetIntensity = 1.0f;

	/** Cached previous integer hour (for OnHourChanged detection) */
	int32 CachedHour = -1;

	// ============================================================================
	// SLEEP STATE
	// ============================================================================

	/** Active sleep request state */
	FSleepRequest SleepRequest;

	/** MP vote state */
	FSleepVoteState SleepVoteState;

	/** Weak ref to server-spawned sleep authority actor */
	TWeakObjectPtr<ASleepManagerAuthority> SleepAuthority;

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
	// SLEEP HELPERS
	// ============================================================================

	/** Complete sleep (target hour reached) */
	void CompleteSleep();

	/** Handle sleep tick (called from OnTimerTick) */
	void HandleSleepTick();

	/** Evaluate vote and act on result */
	void EvaluateSleepVote();

	/** Check if target wake hour is reached (handles midnight wrap) */
	bool HasReachedTargetHour() const;

	/** Get whether this is a multiplayer session */
	bool IsMultiplayerSession() const;

	/** Spawn or get sleep authority for MP */
	ASleepManagerAuthority* EnsureSleepAuthority();

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

	/** Console command: WW.Sleep <Hour> */
	static void CmdSleep(const TArray<FString>& Args, UWorld* World);

	/** Console command: WW.CancelSleep */
	static void CmdCancelSleep(const TArray<FString>& Args, UWorld* World);

	/** Console command: WW.ShowSummary - Force show day summary for testing */
	static void CmdShowSummary(const TArray<FString>& Args, UWorld* World);

	/** Hours slept from last CompleteSleep (for summary) */
	UPROPERTY(SaveGame)
	float LastHoursSlept = 0.0f;

	// ============================================================================
	// SAVE SYSTEM (ISaveableInterface)
	// ============================================================================

	/** Dirty flag for save system (Rule #40) */
	bool bSaveDirty = false;

	/** Mark this subsystem as having unsaved changes */
	void MarkSaveDirty();

	// ISaveableInterface _Implementation methods
	virtual FString GetSaveID_Implementation() const override;
	virtual int32 GetSavePriority_Implementation() const override;
	virtual FGameplayTag GetSaveType_Implementation() const override;
	virtual bool SaveState_Implementation(FSaveRecord& OutRecord) override;
	virtual bool LoadState_Implementation(const FSaveRecord& InRecord) override;
	virtual bool IsDirty_Implementation() const override;
	virtual void ClearDirty_Implementation() override;
	virtual void OnSaveDataLoaded_Implementation() override;
};
