// Copyright Windwalker Productions. All Rights Reserved.

#include "UI/SleepVoteWidget_Base.h"
#include "Subsystems/TimeTrackingSubsystem.h"
#include "Lib/Data/WeatherTimeManager/SleepData.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void USleepVoteWidget_Base::NativeConstruct()
{
	Super::NativeConstruct();
	BindDelegates();

	// Start countdown timer
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CountdownHandle,
			this,
			&USleepVoteWidget_Base::TickCountdown,
			CountdownInterval,
			true
		);
	}
}

void USleepVoteWidget_Base::NativeDestruct()
{
	UnbindDelegates();

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownHandle);
	}

	Super::NativeDestruct();
}

// ============================================================================
// BLUEPRINT PURE GETTERS
// ============================================================================

float USleepVoteWidget_Base::GetVoteTimeRemaining() const
{
	if (UTimeTrackingSubsystem* Sub = const_cast<USleepVoteWidget_Base*>(this)->GetTimeSubsystem())
	{
		const FSleepVoteState& Vote = Sub->GetSleepVoteState();
		const double Elapsed = FPlatformTime::Seconds() - Vote.VoteStartTime;
		return FMath::Max(0.0f, Vote.VoteTimeoutSeconds - static_cast<float>(Elapsed));
	}
	return 0.0f;
}

float USleepVoteWidget_Base::GetVoteProgress() const
{
	if (UTimeTrackingSubsystem* Sub = const_cast<USleepVoteWidget_Base*>(this)->GetTimeSubsystem())
	{
		const FSleepVoteState& Vote = Sub->GetSleepVoteState();
		const int32 Total = Vote.VotesFor.Num() + Vote.VotesAgainst.Num();
		if (Total > 0)
		{
			return static_cast<float>(Vote.VotesFor.Num()) / static_cast<float>(Total);
		}
	}
	return 0.0f;
}

// ============================================================================
// BLUEPRINT NATIVE EVENT DEFAULTS
// ============================================================================

void USleepVoteWidget_Base::OnVoteUpdated_Implementation(int32 VotesFor, int32 VotesAgainst, int32 TotalPlayers)
{
	// BP override point
}

void USleepVoteWidget_Base::OnVoteTimedOut_Implementation()
{
	// BP override point
}

void USleepVoteWidget_Base::OnVoteApproved_Implementation()
{
	// BP override point
}

void USleepVoteWidget_Base::OnVoteRejected_Implementation()
{
	// BP override point
}

// ============================================================================
// DELEGATE BINDING
// ============================================================================

void USleepVoteWidget_Base::BindDelegates()
{
	if (UTimeTrackingSubsystem* Sub = GetTimeSubsystem())
	{
		Sub->OnSleepVoteChanged.AddDynamic(this, &USleepVoteWidget_Base::HandleVoteChanged);
		Sub->OnSleepStarted.AddDynamic(this, &USleepVoteWidget_Base::HandleSleepStarted);
		Sub->OnSleepCancelled.AddDynamic(this, &USleepVoteWidget_Base::HandleSleepCancelled);
	}
}

void USleepVoteWidget_Base::UnbindDelegates()
{
	if (UTimeTrackingSubsystem* Sub = GetTimeSubsystem())
	{
		Sub->OnSleepVoteChanged.RemoveDynamic(this, &USleepVoteWidget_Base::HandleVoteChanged);
		Sub->OnSleepStarted.RemoveDynamic(this, &USleepVoteWidget_Base::HandleSleepStarted);
		Sub->OnSleepCancelled.RemoveDynamic(this, &USleepVoteWidget_Base::HandleSleepCancelled);
	}
}

// ============================================================================
// DELEGATE HANDLERS
// ============================================================================

void USleepVoteWidget_Base::HandleVoteChanged(int32 VotesFor, int32 VotesAgainst, int32 TotalPlayers)
{
	OnVoteUpdated(VotesFor, VotesAgainst, TotalPlayers);
}

void USleepVoteWidget_Base::HandleSleepStarted(float TargetWakeHour)
{
	// Vote approved — sleep is starting
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownHandle);
	}

	OnVoteApproved();
}

void USleepVoteWidget_Base::HandleSleepCancelled(APlayerState* CancellingPlayer)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownHandle);
	}

	OnVoteRejected();
}

void USleepVoteWidget_Base::TickCountdown()
{
	const float Remaining = GetVoteTimeRemaining();
	if (Remaining <= 0.0f)
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CountdownHandle);
		}
		OnVoteTimedOut();
	}
}

UTimeTrackingSubsystem* USleepVoteWidget_Base::GetTimeSubsystem()
{
	if (CachedSubsystem.IsValid())
	{
		return CachedSubsystem.Get();
	}

	UTimeTrackingSubsystem* Sub = UTimeTrackingSubsystem::Get(this);
	CachedSubsystem = Sub;
	return Sub;
}
