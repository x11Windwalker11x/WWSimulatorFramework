// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "SleepWidget_Base.generated.h"

class UTimeTrackingSubsystem;

/**
 * USleepWidget_Base
 *
 * Base widget for sleep progress overlay (Rule #46: extends UManagedWidget_Master).
 * Lives in WeatherTimeManager/UI/ (Rule #47: widget in owning plugin).
 * Blueprint-extendable for visual customization.
 */
UCLASS(Abstract)
class WEATHERTIMEMANAGER_API USleepWidget_Base : public UManagedWidget_Master
{
	GENERATED_BODY()

public:
	// ============================================================================
	// BLUEPRINT PURE GETTERS
	// ============================================================================

	/** Get current sleep progress (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Sleep|Widget")
	float GetSleepProgress() const;

	/** Get target wake hour */
	UFUNCTION(BlueprintPure, Category = "Sleep|Widget")
	float GetTargetWakeHour() const;

	/** Is the player currently sleeping? */
	UFUNCTION(BlueprintPure, Category = "Sleep|Widget")
	bool IsSleeping() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ============================================================================
	// BLUEPRINT NATIVE EVENTS
	// ============================================================================

	/** Called when sleep progress updates (from polling timer) */
	UFUNCTION(BlueprintNativeEvent, Category = "Sleep|Widget")
	void OnSleepProgressUpdated(float Progress);

	/** Called when sleep starts */
	UFUNCTION(BlueprintNativeEvent, Category = "Sleep|Widget")
	void OnSleepStartedEvent(float TargetWakeHour);

	/** Called when sleep completes */
	UFUNCTION(BlueprintNativeEvent, Category = "Sleep|Widget")
	void OnSleepCompletedEvent(float HoursSlept);

	/** Called when sleep is cancelled */
	UFUNCTION(BlueprintNativeEvent, Category = "Sleep|Widget")
	void OnSleepCancelledEvent(APlayerState* CancellingPlayer);

private:
	/** Cached subsystem ref (Rule #41) */
	UPROPERTY()
	TWeakObjectPtr<UTimeTrackingSubsystem> CachedSubsystem;

	/** Timer handle for progress polling during sleep */
	FTimerHandle ProgressPollHandle;

	/** Progress poll interval (4Hz for UI smoothness) */
	static constexpr float ProgressPollInterval = 0.25f;

	/** Bind to subsystem delegates */
	void BindDelegates();

	/** Unbind from subsystem delegates */
	void UnbindDelegates();

	/** Called by polling timer to update progress */
	void PollSleepProgress();

	/** Get the time tracking subsystem */
	UTimeTrackingSubsystem* GetTimeSubsystem();

	// Delegate handlers
	UFUNCTION()
	void HandleSleepStarted(float TargetWakeHour);

	UFUNCTION()
	void HandleSleepCompleted(float HoursSlept);

	UFUNCTION()
	void HandleSleepCancelled(APlayerState* CancellingPlayer);
};
