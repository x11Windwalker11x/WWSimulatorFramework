// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Lib/Data/ModularQuestSystem/QuestData.h"
#include "GameplayTagContainer.h"
#include "QuestTrackerComponent.generated.h"

class UQuestSubsystem;

/**
 * Per-player quest tracking component.
 * Maintains the quest log (replicated) and player tags for prerequisite checks.
 * Attach to PlayerState or Pawn.
 * Rule #13: Replicated + GetLifetimeReplicatedProps
 * Rule #41: Cached subsystem ref
 */
UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class MODULARQUESTSYSTEM_API UQuestTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestTrackerComponent();

	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================================
	// QUEST LOG ACCESS
	// ============================================================================

	/** Add a quest instance to the log */
	void AddQuestInstance(const FQuestInstance& Instance);

	/** Find quest instance by ID (mutable) */
	FQuestInstance* FindQuestInstance(FName QuestID);

	/** Find quest instance by ID (const) */
	const FQuestInstance* FindQuestInstance(FName QuestID) const;

	/** Get the full quest log */
	UFUNCTION(BlueprintPure, Category = "Quest|Tracker")
	const TArray<FQuestInstance>& GetQuestLog() const { return QuestLog; }

	/** Get quest count */
	UFUNCTION(BlueprintPure, Category = "Quest|Tracker")
	int32 GetQuestCount() const { return QuestLog.Num(); }

	/** Get active quest count */
	UFUNCTION(BlueprintPure, Category = "Quest|Tracker")
	int32 GetActiveQuestCount() const;

	/** Check if at max active quests */
	UFUNCTION(BlueprintPure, Category = "Quest|Tracker")
	bool IsAtMaxActiveQuests() const;

	// ============================================================================
	// PLAYER TAGS
	// ============================================================================

	/** Get player tags (for prerequisite checks) */
	UFUNCTION(BlueprintPure, Category = "Quest|Tracker")
	const FGameplayTagContainer& GetPlayerTags() const { return PlayerTags; }

	/** Add a tag to the player */
	UFUNCTION(BlueprintCallable, Category = "Quest|Tracker")
	void AddPlayerTag(FGameplayTag Tag);

	/** Remove a tag from the player */
	UFUNCTION(BlueprintCallable, Category = "Quest|Tracker")
	void RemovePlayerTag(FGameplayTag Tag);

	/** Check if player has a specific tag */
	UFUNCTION(BlueprintPure, Category = "Quest|Tracker")
	bool HasPlayerTag(FGameplayTag Tag) const;

	// ============================================================================
	// SERVER RPCs
	// ============================================================================

	/** Request quest accept from server */
	UFUNCTION(Server, Reliable, Category = "Quest|Network")
	void Server_RequestAcceptQuest(FName QuestID);

	/** Request quest abandon from server */
	UFUNCTION(Server, Reliable, Category = "Quest|Network")
	void Server_RequestAbandonQuest(FName QuestID);

	/** Request quest turn-in from server */
	UFUNCTION(Server, Reliable, Category = "Quest|Network")
	void Server_RequestTurnInQuest(FName QuestID);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Maximum number of simultaneously active quests (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config", meta = (ClampMin = "0"))
	int32 MaxActiveQuests = 20;

private:
	/** Per-player quest log — replicated to owning client */
	UPROPERTY(ReplicatedUsing = OnRep_QuestLog)
	TArray<FQuestInstance> QuestLog;

	/** Player tags for prerequisite checks — replicated to owning client */
	UPROPERTY(ReplicatedUsing = OnRep_PlayerTags)
	FGameplayTagContainer PlayerTags;

	/** Cached quest subsystem ref (Rule #41) */
	UPROPERTY()
	TObjectPtr<UQuestSubsystem> CachedQuestSubsystem;

	/** Rep notify for quest log changes */
	UFUNCTION()
	void OnRep_QuestLog();

	/** Rep notify for player tag changes */
	UFUNCTION()
	void OnRep_PlayerTags();

	/** Cache subsystem reference */
	void CacheSubsystem();
};
