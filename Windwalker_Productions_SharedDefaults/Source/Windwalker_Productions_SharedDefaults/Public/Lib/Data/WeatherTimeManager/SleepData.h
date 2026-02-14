// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SleepData.generated.h"

/**
 * FSleepRequest
 * Stores all state needed for an active or pending sleep operation.
 * Rule #12: Data struct with zero logic (except IsValid).
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FSleepRequest
{
	GENERATED_BODY()

	/** Current sleep state tag (Sleep.State.*) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep")
	FGameplayTag SleepStateTag;

	/** Target hour to wake up (0.0 - 24.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep")
	float TargetWakeHour = 6.0f;

	/** Time speed before sleep started (restored on complete/cancel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep")
	float OriginalTimeSpeed = 1.0f;

	/** Time speed multiplier during sleep (default 60x) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep")
	float SleepSpeedMultiplier = 60.0f;

	/** Hour when sleep started */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep")
	float SleepStartHour = 0.0f;

	/** Unique ID of the player who initiated the sleep request */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep")
	int32 RequestingPlayerID = -1;

	bool IsValid() const
	{
		return SleepStateTag.IsValid();
	}
};

/**
 * FSleepVoteState
 * Tracks the MP consensus vote for a sleep request.
 * Rule #12: Data struct with zero logic (except IsValid).
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FSleepVoteState
{
	GENERATED_BODY()

	/** Player IDs who voted for sleep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep|Vote")
	TArray<int32> VotesFor;

	/** Player IDs who voted against sleep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep|Vote")
	TArray<int32> VotesAgainst;

	/** Seconds before vote times out (default 30) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep|Vote")
	float VoteTimeoutSeconds = 30.0f;

	/** Percentage of players required to approve (default 100%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep|Vote")
	float VoteThresholdPercent = 100.0f;

	/** World time when vote started */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sleep|Vote")
	double VoteStartTime = 0.0;

	bool IsValid() const
	{
		return VoteTimeoutSeconds > 0.0f;
	}
};
