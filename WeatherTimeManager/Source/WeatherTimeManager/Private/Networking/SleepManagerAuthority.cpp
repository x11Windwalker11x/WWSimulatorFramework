// Copyright Windwalker Productions. All Rights Reserved.

#include "Networking/SleepManagerAuthority.h"
#include "Subsystems/TimeTrackingSubsystem.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ASleepManagerAuthority::ASleepManagerAuthority()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = false;
}

void ASleepManagerAuthority::BeginPlay()
{
	Super::BeginPlay();

	// Register self with subsystem
	if (UTimeTrackingSubsystem* Sub = GetTimeSubsystem())
	{
		Sub->SetSleepAuthority(this);
	}
}

void ASleepManagerAuthority::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASleepManagerAuthority, SleepStateTag);
	DOREPLIFETIME(ASleepManagerAuthority, SleepProgress);
	DOREPLIFETIME(ASleepManagerAuthority, VotesFor);
	DOREPLIFETIME(ASleepManagerAuthority, VotesAgainst);
	DOREPLIFETIME(ASleepManagerAuthority, TotalPlayers);
}

// ============================================================================
// SERVER RPCs
// ============================================================================

bool ASleepManagerAuthority::Server_RequestSleep_Validate(float TargetWakeHour, float SpeedMultiplier)
{
	return TargetWakeHour >= 0.0f && TargetWakeHour < 24.0f && SpeedMultiplier >= 1.0f;
}

void ASleepManagerAuthority::Server_RequestSleep_Implementation(float TargetWakeHour, float SpeedMultiplier)
{
	if (UTimeTrackingSubsystem* Sub = GetTimeSubsystem())
	{
		Sub->RequestSleep(TargetWakeHour, SpeedMultiplier);
	}
}

bool ASleepManagerAuthority::Server_CancelSleep_Validate()
{
	return true;
}

void ASleepManagerAuthority::Server_CancelSleep_Implementation()
{
	if (UTimeTrackingSubsystem* Sub = GetTimeSubsystem())
	{
		Sub->CancelSleep(nullptr);
	}
}

bool ASleepManagerAuthority::Server_CastSleepVote_Validate(int32 PlayerID, bool bApprove)
{
	return PlayerID >= 0;
}

void ASleepManagerAuthority::Server_CastSleepVote_Implementation(int32 PlayerID, bool bApprove)
{
	if (UTimeTrackingSubsystem* Sub = GetTimeSubsystem())
	{
		Sub->CastSleepVote(PlayerID, bApprove);

		// Update replicated state
		const FSleepVoteState& VoteState = Sub->GetSleepVoteState();
		VotesFor = VoteState.VotesFor.Num();
		VotesAgainst = VoteState.VotesAgainst.Num();
	}
}

// ============================================================================
// CLIENT RPCs
// ============================================================================

void ASleepManagerAuthority::Client_SleepVotePrompt_Implementation(float TargetWakeHour, float TimeoutSeconds)
{
	// Client-side: UI system can bind to the subsystem's OnSleepRequested delegate
	// to display the vote prompt widget
}

// ============================================================================
// MULTICAST RPCs
// ============================================================================

void ASleepManagerAuthority::Multicast_SleepStateChanged_Implementation(FGameplayTag NewState)
{
	SleepStateTag = NewState;
}

void ASleepManagerAuthority::Multicast_SleepProgressUpdate_Implementation(float Progress)
{
	SleepProgress = Progress;
}

// ============================================================================
// INTERNAL
// ============================================================================

void ASleepManagerAuthority::OnRep_SleepStateTag()
{
	// Client-side state sync — subsystem delegates handle UI updates
}

UTimeTrackingSubsystem* ASleepManagerAuthority::GetTimeSubsystem()
{
	if (CachedSubsystem.IsValid())
	{
		return CachedSubsystem.Get();
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	UTimeTrackingSubsystem* Sub = GI->GetSubsystem<UTimeTrackingSubsystem>();
	CachedSubsystem = Sub;
	return Sub;
}
