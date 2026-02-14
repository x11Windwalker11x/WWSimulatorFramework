// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/WeatherTimeManager/SleepData.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "SleepManagerAuthority.generated.h"

class UTimeTrackingSubsystem;

/**
 * ASleepManagerAuthority
 *
 * Server-spawned replicated actor that serves as the network proxy for MP sleep operations.
 * Required because UTimeTrackingSubsystem (GameInstanceSubsystem) cannot own RPCs.
 *
 * Forwards all calls to cached UTimeTrackingSubsystem (same-plugin, Rule #30 OK).
 * Lazy-spawned on first MP RequestSleep() or exposed via SpawnSleepAuthority().
 */
UCLASS()
class WEATHERTIMEMANAGER_API ASleepManagerAuthority : public AActor
{
	GENERATED_BODY()

public:
	ASleepManagerAuthority();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================================
	// REPLICATED STATE
	// ============================================================================

	/** Current sleep state tag (replicated to all clients) */
	UPROPERTY(ReplicatedUsing = OnRep_SleepStateTag, BlueprintReadOnly, Category = "Sleep|MP")
	FGameplayTag SleepStateTag;

	/** Sleep progress (0.0 - 1.0) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sleep|MP")
	float SleepProgress = 0.0f;

	/** Votes for sleep */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sleep|MP")
	int32 VotesFor = 0;

	/** Votes against sleep */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sleep|MP")
	int32 VotesAgainst = 0;

	/** Total players in vote */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sleep|MP")
	int32 TotalPlayers = 0;

	// ============================================================================
	// SERVER RPCs
	// ============================================================================

	/** Request sleep from client */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Sleep|MP")
	void Server_RequestSleep(float TargetWakeHour, float SpeedMultiplier);

	/** Cancel sleep from client */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Sleep|MP")
	void Server_CancelSleep();

	/** Cast vote from client */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Sleep|MP")
	void Server_CastSleepVote(int32 PlayerID, bool bApprove);

	// ============================================================================
	// CLIENT RPCs
	// ============================================================================

	/** Prompt client to vote on sleep */
	UFUNCTION(Client, Reliable, Category = "Sleep|MP")
	void Client_SleepVotePrompt(float TargetWakeHour, float TimeoutSeconds);

	// ============================================================================
	// MULTICAST RPCs
	// ============================================================================

	/** Notify all clients of sleep state change */
	UFUNCTION(NetMulticast, Reliable, Category = "Sleep|MP")
	void Multicast_SleepStateChanged(FGameplayTag NewState);

	/** Update all clients with sleep progress */
	UFUNCTION(NetMulticast, Unreliable, Category = "Sleep|MP")
	void Multicast_SleepProgressUpdate(float Progress);

private:
	UFUNCTION()
	void OnRep_SleepStateTag();

	/** Cached subsystem reference (Rule #41) */
	UPROPERTY()
	TWeakObjectPtr<UTimeTrackingSubsystem> CachedSubsystem;

	/** Get and cache the time tracking subsystem */
	UTimeTrackingSubsystem* GetTimeSubsystem();
};
