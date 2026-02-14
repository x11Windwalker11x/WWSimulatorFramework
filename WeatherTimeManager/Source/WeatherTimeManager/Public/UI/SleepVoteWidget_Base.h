// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "SleepVoteWidget_Base.generated.h"

class UTimeTrackingSubsystem;

/**
 * USleepVoteWidget_Base
 *
 * Base widget for MP sleep vote prompt (Rule #46: extends UManagedWidget_Master).
 * Modal category tag for ESC priority. Blueprint-extendable.
 */
UCLASS(Abstract)
class WEATHERTIMEMANAGER_API USleepVoteWidget_Base : public UManagedWidget_Master
{
	GENERATED_BODY()

public:
	// ============================================================================
	// BLUEPRINT PURE GETTERS
	// ============================================================================

	/** Get remaining vote time in seconds */
	UFUNCTION(BlueprintPure, Category = "Sleep|Vote|Widget")
	float GetVoteTimeRemaining() const;

	/** Get vote progress (VotesFor / TotalPlayers) */
	UFUNCTION(BlueprintPure, Category = "Sleep|Vote|Widget")
	float GetVoteProgress() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ============================================================================
	// BLUEPRINT NATIVE EVENTS
	// ============================================================================

	/** Called when vote tallies change */
	UFUNCTION(BlueprintNativeEvent, Category = "Sleep|Vote|Widget")
	void OnVoteUpdated(int32 VotesFor, int32 VotesAgainst, int32 TotalPlayers);

	/** Called when vote times out */
	UFUNCTION(BlueprintNativeEvent, Category = "Sleep|Vote|Widget")
	void OnVoteTimedOut();

	/** Called when vote is approved */
	UFUNCTION(BlueprintNativeEvent, Category = "Sleep|Vote|Widget")
	void OnVoteApproved();

	/** Called when vote is rejected */
	UFUNCTION(BlueprintNativeEvent, Category = "Sleep|Vote|Widget")
	void OnVoteRejected();

private:
	/** Cached subsystem ref (Rule #41) */
	UPROPERTY()
	TWeakObjectPtr<UTimeTrackingSubsystem> CachedSubsystem;

	/** Timer handle for countdown */
	FTimerHandle CountdownHandle;

	/** Countdown interval (1Hz) */
	static constexpr float CountdownInterval = 1.0f;

	/** Bind to subsystem delegates */
	void BindDelegates();

	/** Unbind from subsystem delegates */
	void UnbindDelegates();

	/** Countdown tick */
	void TickCountdown();

	/** Get the time tracking subsystem */
	UTimeTrackingSubsystem* GetTimeSubsystem();

	// Delegate handlers
	UFUNCTION()
	void HandleVoteChanged(int32 VotesFor, int32 VotesAgainst, int32 TotalPlayers);

	UFUNCTION()
	void HandleSleepStarted(float TargetWakeHour);

	UFUNCTION()
	void HandleSleepCancelled(APlayerState* CancellingPlayer);
};
