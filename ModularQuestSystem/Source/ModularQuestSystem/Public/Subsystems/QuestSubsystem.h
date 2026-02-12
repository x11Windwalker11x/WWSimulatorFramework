// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Delegates/ModularQuestSystem/QuestDelegates.h"
#include "Lib/Data/ModularQuestSystem/QuestData.h"
#include "GameplayTagContainer.h"
#include "QuestSubsystem.generated.h"

class UObjectiveTrackerSubsystem;
class UEventBusSubsystem;
class UQuestTrackerComponent;
class UDataTable;

/**
 * Central quest registry and lifecycle manager.
 * Owns quest definitions (from DataTable), manages state transitions,
 * and binds to ObjectiveTrackerSubsystem for objective tracking.
 * Rewards distributed via EventBus (no L2->L2 deps).
 */
UCLASS()
class MODULARQUESTSYSTEM_API UQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UQuestSubsystem();

	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Static accessor */
	UFUNCTION(BlueprintPure, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static UQuestSubsystem* Get(const UObject* WorldContextObject);

	// ============================================================================
	// DELEGATES
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Quest|Delegates")
	FOnQuestStateChanged OnQuestStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Delegates")
	FOnQuestAccepted OnQuestAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Delegates")
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Delegates")
	FOnQuestFailed OnQuestFailed;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Delegates")
	FOnQuestTurnedIn OnQuestTurnedIn;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Delegates")
	FOnQuestAvailable OnQuestAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Delegates")
	FOnQuestObjectiveProgress OnQuestObjectiveProgress;

	// ============================================================================
	// QUEST REGISTRY
	// ============================================================================

	/** Load quest definitions from DataTable (FQuestData rows) */
	UFUNCTION(BlueprintCallable, Category = "Quest|Registry")
	void LoadQuestsFromDataTable(UDataTable* QuestTable);

	/** Load quest chain definitions from DataTable (FQuestChain rows) */
	UFUNCTION(BlueprintCallable, Category = "Quest|Registry")
	void LoadChainsFromDataTable(UDataTable* ChainTable);

	/** Register a single quest definition at runtime */
	UFUNCTION(BlueprintCallable, Category = "Quest|Registry")
	bool RegisterQuest(const FQuestData& QuestDef);

	/** Get a quest definition by ID */
	UFUNCTION(BlueprintPure, Category = "Quest|Registry")
	bool GetQuestData(FName QuestID, FQuestData& OutData) const;

	/** Get a quest chain by ID */
	UFUNCTION(BlueprintPure, Category = "Quest|Registry")
	bool GetQuestChain(FName ChainID, FQuestChain& OutChain) const;

	/** Get all registered quest IDs */
	UFUNCTION(BlueprintPure, Category = "Quest|Registry")
	TArray<FName> GetAllQuestIDs() const;

	/** Get quests by type tag */
	UFUNCTION(BlueprintPure, Category = "Quest|Registry")
	TArray<FName> GetQuestsByType(FGameplayTag QuestType) const;

	// ============================================================================
	// QUEST LIFECYCLE
	// ============================================================================

	/** Accept a quest for a player */
	UFUNCTION(BlueprintCallable, Category = "Quest|Lifecycle")
	bool AcceptQuest(FName QuestID, AActor* Player);

	/** Abandon an active quest */
	UFUNCTION(BlueprintCallable, Category = "Quest|Lifecycle")
	bool AbandonQuest(FName QuestID, AActor* Player);

	/** Force-fail a quest */
	UFUNCTION(BlueprintCallable, Category = "Quest|Lifecycle")
	bool FailQuest(FName QuestID, AActor* Player);

	/** Turn in a completed quest (claim rewards) */
	UFUNCTION(BlueprintCallable, Category = "Quest|Lifecycle")
	bool TurnInQuest(FName QuestID, AActor* Player);

	/** Force-complete a quest (debug/cheat) */
	UFUNCTION(BlueprintCallable, Category = "Quest|Lifecycle")
	bool ForceCompleteQuest(FName QuestID, AActor* Player);

	// ============================================================================
	// QUERIES
	// ============================================================================

	/** Get quest state tag for a player */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	FGameplayTag GetQuestState(FName QuestID, AActor* Player) const;

	/** Check if player meets prerequisites for a quest */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	bool MeetsPrerequisites(FName QuestID, AActor* Player) const;

	/** Get all active quest IDs for a player */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	TArray<FName> GetActiveQuests(AActor* Player) const;

	/** Get next quest in a chain for a player */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	FName GetNextChainQuest(FName ChainID, AActor* Player) const;

	/** Get quest progress (0-1) via ObjectiveTracker */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
	float GetQuestProgress(FName QuestID, AActor* Player) const;

private:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	/** Quest definition registry */
	TMap<FName, FQuestData> QuestRegistry;

	/** Quest chain registry */
	TMap<FName, FQuestChain> ChainRegistry;

	/** Reverse lookup: ObjectiveTracker SetID -> {QuestID, Player} */
	TMap<FGuid, TPair<FName, TWeakObjectPtr<AActor>>> SetIDToQuestMap;

	/** Cached subsystem refs (Rule #41) */
	UPROPERTY()
	TObjectPtr<UObjectiveTrackerSubsystem> CachedObjectiveTracker;

	UPROPERTY()
	TObjectPtr<UEventBusSubsystem> CachedEventBus;

	// ============================================================================
	// INTERNAL HELPERS
	// ============================================================================

	void CacheSubsystems();
	UQuestTrackerComponent* GetTrackerComponent(AActor* Player) const;

	/** Change quest state and broadcast delegates */
	bool TransitionQuestState(FName QuestID, AActor* Player, const FGameplayTag& NewState);

	/** ObjectiveTracker delegate handlers */
	UFUNCTION()
	void HandleObjectiveSetComplete(const FGuid& SetID, bool bBonusAchieved);

	UFUNCTION()
	void HandleObjectiveSetFailed(const FGuid& SetID);

	UFUNCTION()
	void HandleObjectiveUpdated(const FGuid& SetID, const FGameplayTag& ObjectiveTag, float NewValue);

	/** Broadcast quest event to EventBus */
	void BroadcastQuestEvent(const FGameplayTag& EventTag, FName QuestID, AActor* Player, float Value = 0.0f);

	/** Register SetID -> Quest mapping */
	void RegisterSetIDMapping(const FGuid& SetID, FName QuestID, AActor* Player);

	/** Unregister SetID mapping */
	void UnregisterSetIDMapping(const FGuid& SetID);

	/** Check and unlock newly available quests after a state change */
	void RefreshQuestAvailability(AActor* Player);
};
