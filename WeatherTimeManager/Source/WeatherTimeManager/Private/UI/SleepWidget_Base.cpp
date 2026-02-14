// Copyright Windwalker Productions. All Rights Reserved.

#include "UI/SleepWidget_Base.h"
#include "Subsystems/TimeTrackingSubsystem.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void USleepWidget_Base::NativeConstruct()
{
	Super::NativeConstruct();
	BindDelegates();
}

void USleepWidget_Base::NativeDestruct()
{
	UnbindDelegates();

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProgressPollHandle);
	}

	Super::NativeDestruct();
}

// ============================================================================
// BLUEPRINT PURE GETTERS
// ============================================================================

float USleepWidget_Base::GetSleepProgress() const
{
	if (UTimeTrackingSubsystem* Sub = const_cast<USleepWidget_Base*>(this)->GetTimeSubsystem())
	{
		return Sub->GetSleepProgress();
	}
	return 0.0f;
}

float USleepWidget_Base::GetTargetWakeHour() const
{
	if (UTimeTrackingSubsystem* Sub = const_cast<USleepWidget_Base*>(this)->GetTimeSubsystem())
	{
		return Sub->GetSleepRequest().TargetWakeHour;
	}
	return 0.0f;
}

bool USleepWidget_Base::IsSleeping() const
{
	if (UTimeTrackingSubsystem* Sub = const_cast<USleepWidget_Base*>(this)->GetTimeSubsystem())
	{
		return Sub->IsSleeping();
	}
	return false;
}

// ============================================================================
// BLUEPRINT NATIVE EVENT DEFAULTS
// ============================================================================

void USleepWidget_Base::OnSleepProgressUpdated_Implementation(float Progress)
{
	// BP override point
}

void USleepWidget_Base::OnSleepStartedEvent_Implementation(float TargetWakeHour)
{
	// BP override point
}

void USleepWidget_Base::OnSleepCompletedEvent_Implementation(float HoursSlept)
{
	// BP override point
}

void USleepWidget_Base::OnSleepCancelledEvent_Implementation(APlayerState* CancellingPlayer)
{
	// BP override point
}

// ============================================================================
// DELEGATE BINDING
// ============================================================================

void USleepWidget_Base::BindDelegates()
{
	if (UTimeTrackingSubsystem* Sub = GetTimeSubsystem())
	{
		Sub->OnSleepStarted.AddDynamic(this, &USleepWidget_Base::HandleSleepStarted);
		Sub->OnSleepCompleted.AddDynamic(this, &USleepWidget_Base::HandleSleepCompleted);
		Sub->OnSleepCancelled.AddDynamic(this, &USleepWidget_Base::HandleSleepCancelled);
	}
}

void USleepWidget_Base::UnbindDelegates()
{
	if (UTimeTrackingSubsystem* Sub = GetTimeSubsystem())
	{
		Sub->OnSleepStarted.RemoveDynamic(this, &USleepWidget_Base::HandleSleepStarted);
		Sub->OnSleepCompleted.RemoveDynamic(this, &USleepWidget_Base::HandleSleepCompleted);
		Sub->OnSleepCancelled.RemoveDynamic(this, &USleepWidget_Base::HandleSleepCancelled);
	}
}

// ============================================================================
// DELEGATE HANDLERS
// ============================================================================

void USleepWidget_Base::HandleSleepStarted(float TargetWakeHour)
{
	OnSleepStartedEvent(TargetWakeHour);

	// Start progress polling timer
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ProgressPollHandle,
			this,
			&USleepWidget_Base::PollSleepProgress,
			ProgressPollInterval,
			true
		);
	}
}

void USleepWidget_Base::HandleSleepCompleted(float HoursSlept)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProgressPollHandle);
	}

	OnSleepCompletedEvent(HoursSlept);
}

void USleepWidget_Base::HandleSleepCancelled(APlayerState* CancellingPlayer)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProgressPollHandle);
	}

	OnSleepCancelledEvent(CancellingPlayer);
}

void USleepWidget_Base::PollSleepProgress()
{
	if (UTimeTrackingSubsystem* Sub = GetTimeSubsystem())
	{
		OnSleepProgressUpdated(Sub->GetSleepProgress());
	}
}

UTimeTrackingSubsystem* USleepWidget_Base::GetTimeSubsystem()
{
	if (CachedSubsystem.IsValid())
	{
		return CachedSubsystem.Get();
	}

	UTimeTrackingSubsystem* Sub = UTimeTrackingSubsystem::Get(this);
	CachedSubsystem = Sub;
	return Sub;
}
