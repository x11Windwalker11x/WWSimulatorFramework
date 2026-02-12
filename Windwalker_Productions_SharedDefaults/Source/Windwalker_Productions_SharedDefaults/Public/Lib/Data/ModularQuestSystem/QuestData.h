// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/Core/ObjectiveData.h"
#include "QuestData.generated.h"

/**
 * Reward definition for quest completion
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FQuestReward
{
	GENERATED_BODY()

	/** Reward type tag (Quest.Reward.XP, Quest.Reward.Currency, Quest.Reward.Item) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag RewardType;

	/** Reward ID (item row name, currency type, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName RewardID = NAME_None;

	/** Numeric amount (XP amount, currency amount, item quantity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0"))
	int32 Amount = 0;

	bool IsValid() const { return RewardType.IsValid() && Amount > 0; }
};

/**
 * Static quest definition -- designed for DataTable or JSON loading
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FQuestData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique quest identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName QuestID = NAME_None;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText DisplayName;

	/** Quest description for journal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	/** Quest type tag (Quest.Type.Main, Quest.Type.Side, Quest.Type.Daily) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag QuestType;

	/** Objective definitions for this quest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FObjectiveSet Objectives;

	/** Prerequisites: quest IDs that must be TurnedIn before this quest becomes Available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FName> PrerequisiteQuestIDs;

	/** Prerequisite tags the player must have (level, reputation, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer PrerequisiteTags;

	/** Rewards on completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FQuestReward> Rewards;

	/** Bonus rewards if bonus threshold met */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FQuestReward> BonusRewards;

	/** Can this quest be retried after failure? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bRetryable = true;

	/** Is this quest part of a chain? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName ChainID = NAME_None;

	/** Sort priority for journal display (lower = higher in list) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0"))
	int32 SortPriority = 100;

	/** Optional time limit in seconds (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0.0"))
	float TimeLimitSeconds = 0.0f;

	/** Tags for this quest to enable event filtering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer QuestTags;

	bool IsValid() const { return !QuestID.IsNone() && Objectives.IsValid(); }
};

/**
 * Runtime quest instance -- per-player state for an accepted quest
 * State tracked via FGameplayTag (Quest.State.*), not enum
 * NOT for DataTables -- runtime only
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FQuestInstance
{
	GENERATED_BODY()

	/** Reference to the quest definition ID */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FName QuestID = NAME_None;

	/** Current state tag (Quest.State.Unavailable/Available/Active/Completed/Failed/TurnedIn) */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FGameplayTag StateTag;

	/** SetID in ObjectiveTrackerSubsystem (links to tracked objectives) */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FGuid ObjectiveSetID;

	/** Timestamp when quest was accepted */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	float AcceptedTimestamp = 0.0f;

	/** Timestamp when quest was completed/failed */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	float CompletedTimestamp = 0.0f;

	/** Whether bonus was achieved */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	bool bBonusAchieved = false;

	bool IsValid() const { return !QuestID.IsNone(); }
};

/**
 * Quest chain definition -- ordered sequence of quests
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FQuestChain : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique chain identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName ChainID = NAME_None;

	/** Display name for the chain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText DisplayName;

	/** Ordered list of quest IDs in this chain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FName> QuestSequence;

	/** Chain type tag (Quest.Chain.Story, Quest.Chain.Side) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag ChainType;

	bool IsValid() const { return !ChainID.IsNone() && QuestSequence.Num() > 0; }
};
