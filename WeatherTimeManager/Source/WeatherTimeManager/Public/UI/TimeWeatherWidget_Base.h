// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "Lib/Data/WeatherTimeManager/TimeWeatherData.h"
#include "GameplayTagContainer.h"
#include "TimeWeatherWidget_Base.generated.h"

class UTimeTrackingSubsystem;

/**
 * Abstract base class for all time/weather HUD widgets.
 * Provides auto-binding to TimeTrackingSubsystem delegates,
 * cached subsystem ref (Rule #41), and BlueprintNativeEvent callbacks.
 * Extends UManagedWidget_Master (Rule #46).
 * Subclasses: ClockWidget_AnalogBase, ClockWidget_DigitalBase, DateTimeWidget_Base.
 */
UCLASS(Abstract, Blueprintable)
class WEATHERTIMEMANAGER_API UTimeWeatherWidget_Base : public UManagedWidget_Master
{
	GENERATED_BODY()

public:
	UTimeWeatherWidget_Base(const FObjectInitializer& ObjectInitializer);

	// ============================================================================
	// STATE QUERIES (BlueprintPure)
	// ============================================================================

	/** Get current hour (0.0 - 24.0) */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget")
	float GetCurrentHour() const;

	/** Get current day number */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget")
	int32 GetCurrentDay() const;

	/** Get current time-of-day period tag */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget")
	FGameplayTag GetTimeOfDayTag() const;

	/** Get current weather tag */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget")
	FGameplayTag GetCurrentWeatherTag() const;

	/** Is time paused? */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget")
	bool IsTimePaused() const;

	/** Get time speed multiplier */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget")
	float GetTimeSpeed() const;

protected:
	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ============================================================================
	// BLUEPRINT NATIVE EVENTS
	// ============================================================================

	/** Called when the integer hour changes */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|Events")
	void OnHourChanged(int32 OldHour, int32 NewHour);

	/** Called when the day number increments */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|Events")
	void OnDayChanged(int32 OldDay, int32 NewDay);

	/** Called when the time-of-day period changes */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|Events")
	void OnTimeOfDayPeriodChanged(const FGameplayTag& OldPeriod, const FGameplayTag& NewPeriod);

	/** Called when weather changes */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|Events")
	void OnWeatherStateChanged(const FGameplayTag& OldWeather, const FGameplayTag& NewWeather);

	/** Called when time is paused */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|Events")
	void OnTimePausedEvent();

	/** Called when time is resumed */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|Events")
	void OnTimeResumedEvent();

	// ============================================================================
	// CACHED STATE (Rule #41)
	// ============================================================================

	/** Cached reference to TimeTrackingSubsystem */
	UPROPERTY(BlueprintReadOnly, Category = "TimeWeather|Widget|Cache")
	TWeakObjectPtr<UTimeTrackingSubsystem> CachedSubsystem;

private:
	// ============================================================================
	// DELEGATE HANDLERS
	// ============================================================================

	UFUNCTION()
	void HandleHourChanged(int32 OldHour, int32 NewHour);

	UFUNCTION()
	void HandleDayChanged(int32 OldDay, int32 NewDay);

	UFUNCTION()
	void HandleTimeOfDayChanged(const FGameplayTag& OldPeriod, const FGameplayTag& NewPeriod);

	UFUNCTION()
	void HandleWeatherChanged(const FGameplayTag& OldWeather, const FGameplayTag& NewWeather);

	UFUNCTION()
	void HandleTimePaused();

	UFUNCTION()
	void HandleTimeResumed();

	/** Bind to subsystem delegates */
	void BindDelegates();

	/** Unbind from subsystem delegates */
	void UnbindDelegates();
};
