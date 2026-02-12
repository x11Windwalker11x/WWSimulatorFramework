// Copyright Windwalker Productions. All Rights Reserved.

#include "Subsystems/QuestSubsystem.h"
#include "Components/QuestTrackerComponent.h"
#include "Subsystems/ModularQuestSystem/ObjectiveTrackerSubsystem.h"
#include "Subsystems/EventBusSubsystem.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Lib/Data/ModularQuestSystem/GameplayEventData.h"
#include "Engine/DataTable.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuestSystem, Log, All);

UQuestSubsystem::UQuestSubsystem()
{
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CacheSubsystems();

	if (CachedObjectiveTracker)
	{
		CachedObjectiveTracker->OnObjectiveSetComplete.AddDynamic(this, &UQuestSubsystem::HandleObjectiveSetComplete);
		CachedObjectiveTracker->OnObjectiveSetFailed.AddDynamic(this, &UQuestSubsystem::HandleObjectiveSetFailed);
		CachedObjectiveTracker->OnObjectiveUpdated.AddDynamic(this, &UQuestSubsystem::HandleObjectiveUpdated);
	}

	UE_LOG(LogQuestSystem, Log, TEXT("QuestSubsystem initialized"));
}

void UQuestSubsystem::Deinitialize()
{
	if (CachedObjectiveTracker)
	{
		CachedObjectiveTracker->OnObjectiveSetComplete.RemoveDynamic(this, &UQuestSubsystem::HandleObjectiveSetComplete);
		CachedObjectiveTracker->OnObjectiveSetFailed.RemoveDynamic(this, &UQuestSubsystem::HandleObjectiveSetFailed);
		CachedObjectiveTracker->OnObjectiveUpdated.RemoveDynamic(this, &UQuestSubsystem::HandleObjectiveUpdated);
	}

	QuestRegistry.Empty();
	ChainRegistry.Empty();
	SetIDToQuestMap.Empty();

	UE_LOG(LogQuestSystem, Log, TEXT("QuestSubsystem deinitialized"));
	Super::Deinitialize();
}

UQuestSubsystem* UQuestSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;

	return GI->GetSubsystem<UQuestSubsystem>();
}

// ============================================================================
// QUEST REGISTRY
// ============================================================================

void UQuestSubsystem::LoadQuestsFromDataTable(UDataTable* QuestTable)
{
	if (!QuestTable)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("LoadQuestsFromDataTable: null DataTable"));
		return;
	}

	TArray<FQuestData*> Rows;
	QuestTable->GetAllRows<FQuestData>(TEXT("LoadQuestsFromDataTable"), Rows);

	int32 LoadedCount = 0;
	for (const FQuestData* Row : Rows)
	{
		if (Row && Row->IsValid())
		{
			QuestRegistry.Add(Row->QuestID, *Row);
			LoadedCount++;
		}
	}

	UE_LOG(LogQuestSystem, Log, TEXT("Loaded %d quests from DataTable '%s'"), LoadedCount, *QuestTable->GetName());
}

void UQuestSubsystem::LoadChainsFromDataTable(UDataTable* ChainTable)
{
	if (!ChainTable)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("LoadChainsFromDataTable: null DataTable"));
		return;
	}

	TArray<FQuestChain*> Rows;
	ChainTable->GetAllRows<FQuestChain>(TEXT("LoadChainsFromDataTable"), Rows);

	int32 LoadedCount = 0;
	for (const FQuestChain* Row : Rows)
	{
		if (Row && Row->IsValid())
		{
			ChainRegistry.Add(Row->ChainID, *Row);
			LoadedCount++;
		}
	}

	UE_LOG(LogQuestSystem, Log, TEXT("Loaded %d chains from DataTable '%s'"), LoadedCount, *ChainTable->GetName());
}

bool UQuestSubsystem::RegisterQuest(const FQuestData& QuestDef)
{
	if (!QuestDef.IsValid())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("RegisterQuest: invalid quest definition"));
		return false;
	}

	if (QuestRegistry.Contains(QuestDef.QuestID))
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("RegisterQuest: quest '%s' already registered"), *QuestDef.QuestID.ToString());
		return false;
	}

	QuestRegistry.Add(QuestDef.QuestID, QuestDef);
	UE_LOG(LogQuestSystem, Verbose, TEXT("Registered quest: %s"), *QuestDef.QuestID.ToString());
	return true;
}

bool UQuestSubsystem::GetQuestData(FName QuestID, FQuestData& OutData) const
{
	const FQuestData* Found = QuestRegistry.Find(QuestID);
	if (Found)
	{
		OutData = *Found;
		return true;
	}
	return false;
}

bool UQuestSubsystem::GetQuestChain(FName ChainID, FQuestChain& OutChain) const
{
	const FQuestChain* Found = ChainRegistry.Find(ChainID);
	if (Found)
	{
		OutChain = *Found;
		return true;
	}
	return false;
}

TArray<FName> UQuestSubsystem::GetAllQuestIDs() const
{
	TArray<FName> Result;
	QuestRegistry.GetKeys(Result);
	return Result;
}

TArray<FName> UQuestSubsystem::GetQuestsByType(FGameplayTag QuestType) const
{
	TArray<FName> Result;
	for (const auto& Pair : QuestRegistry)
	{
		if (Pair.Value.QuestType == QuestType)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

// ============================================================================
// QUEST LIFECYCLE
// ============================================================================

bool UQuestSubsystem::AcceptQuest(FName QuestID, AActor* Player)
{
	if (!Player)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AcceptQuest: null Player"));
		return false;
	}

	const FQuestData* QuestDef = QuestRegistry.Find(QuestID);
	if (!QuestDef)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AcceptQuest: quest '%s' not found in registry"), *QuestID.ToString());
		return false;
	}

	if (!MeetsPrerequisites(QuestID, Player))
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AcceptQuest: prerequisites not met for '%s'"), *QuestID.ToString());
		return false;
	}

	UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AcceptQuest: no QuestTrackerComponent on player"));
		return false;
	}

	// Check if quest is already active
	FGameplayTag CurrentState = GetQuestState(QuestID, Player);
	if (CurrentState == FWWTagLibrary::Quest_State_Active())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AcceptQuest: quest '%s' already active"), *QuestID.ToString());
		return false;
	}

	// Cannot accept completed/turned-in quests
	if (CurrentState == FWWTagLibrary::Quest_State_Completed() || CurrentState == FWWTagLibrary::Quest_State_TurnedIn())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AcceptQuest: quest '%s' already finished"), *QuestID.ToString());
		return false;
	}

	// Check max active quests before registering objectives (avoid orphaned sets)
	if (Tracker->IsAtMaxActiveQuests())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AcceptQuest: at max active quests"));
		return false;
	}

	// Register objectives with ObjectiveTracker
	CacheSubsystems();
	FGuid SetID;
	if (CachedObjectiveTracker && QuestDef->Objectives.IsValid())
	{
		SetID = CachedObjectiveTracker->RegisterObjectiveSet(QuestDef->Objectives);
		RegisterSetIDMapping(SetID, QuestID, Player);
	}

	const float WorldTime = Player->GetWorld() ? Player->GetWorld()->GetTimeSeconds() : 0.0f;

	// Check for existing instance (retry/re-accept after abandon or fail)
	FQuestInstance* ExistingInstance = Tracker->FindQuestInstance(QuestID);
	if (ExistingInstance)
	{
		FGameplayTag OldState = ExistingInstance->StateTag;
		ExistingInstance->StateTag = FWWTagLibrary::Quest_State_Active();
		ExistingInstance->ObjectiveSetID = SetID;
		ExistingInstance->AcceptedTimestamp = WorldTime;
		ExistingInstance->CompletedTimestamp = 0.0f;
		ExistingInstance->bBonusAchieved = false;

		OnQuestStateChanged.Broadcast(QuestID, OldState, FWWTagLibrary::Quest_State_Active(), Player);
	}
	else
	{
		// First accept: create new instance
		FQuestInstance Instance;
		Instance.QuestID = QuestID;
		Instance.StateTag = FWWTagLibrary::Quest_State_Active();
		Instance.ObjectiveSetID = SetID;
		Instance.AcceptedTimestamp = WorldTime;

		Tracker->AddQuestInstance(Instance);
		OnQuestStateChanged.Broadcast(QuestID, FWWTagLibrary::Quest_State_Unavailable(), FWWTagLibrary::Quest_State_Active(), Player);
	}

	OnQuestAccepted.Broadcast(QuestID, Player);
	BroadcastQuestEvent(FWWTagLibrary::Quest_Event_Accepted(), QuestID, Player);

	UE_LOG(LogQuestSystem, Log, TEXT("Quest '%s' accepted by %s"), *QuestID.ToString(), *Player->GetName());
	return true;
}

bool UQuestSubsystem::AbandonQuest(FName QuestID, AActor* Player)
{
	if (!Player)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AbandonQuest: null Player"));
		return false;
	}

	UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return false;

	FGameplayTag CurrentState = GetQuestState(QuestID, Player);
	if (CurrentState != FWWTagLibrary::Quest_State_Active())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AbandonQuest: quest '%s' is not active"), *QuestID.ToString());
		return false;
	}

	// Unregister objectives
	FQuestInstance* Instance = Tracker->FindQuestInstance(QuestID);
	if (Instance && Instance->ObjectiveSetID.IsValid())
	{
		CacheSubsystems();
		if (CachedObjectiveTracker)
		{
			CachedObjectiveTracker->UnregisterObjectiveSet(Instance->ObjectiveSetID);
		}
		UnregisterSetIDMapping(Instance->ObjectiveSetID);
		Instance->ObjectiveSetID.Invalidate();
	}

	// Retryable quests go back to Available, non-retryable to Unavailable
	const FQuestData* QuestDef = QuestRegistry.Find(QuestID);
	const FGameplayTag NewState = (QuestDef && QuestDef->bRetryable)
		? FWWTagLibrary::Quest_State_Available()
		: FWWTagLibrary::Quest_State_Unavailable();

	return TransitionQuestState(QuestID, Player, NewState);
}

bool UQuestSubsystem::FailQuest(FName QuestID, AActor* Player)
{
	if (!Player) return false;

	FGameplayTag CurrentState = GetQuestState(QuestID, Player);
	if (CurrentState != FWWTagLibrary::Quest_State_Active())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("FailQuest: quest '%s' is not active"), *QuestID.ToString());
		return false;
	}

	// Unregister objectives
	UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (Tracker)
	{
		FQuestInstance* Instance = Tracker->FindQuestInstance(QuestID);
		if (Instance && Instance->ObjectiveSetID.IsValid())
		{
			CacheSubsystems();
			if (CachedObjectiveTracker)
			{
				CachedObjectiveTracker->UnregisterObjectiveSet(Instance->ObjectiveSetID);
			}
			UnregisterSetIDMapping(Instance->ObjectiveSetID);
			Instance->ObjectiveSetID.Invalidate();
		}
	}

	if (TransitionQuestState(QuestID, Player, FWWTagLibrary::Quest_State_Failed()))
	{
		OnQuestFailed.Broadcast(QuestID, Player);
		BroadcastQuestEvent(FWWTagLibrary::Quest_Event_Failed(), QuestID, Player);
		return true;
	}
	return false;
}

bool UQuestSubsystem::TurnInQuest(FName QuestID, AActor* Player)
{
	if (!Player) return false;

	FGameplayTag CurrentState = GetQuestState(QuestID, Player);
	if (CurrentState != FWWTagLibrary::Quest_State_Completed())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("TurnInQuest: quest '%s' is not completed"), *QuestID.ToString());
		return false;
	}

	UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return false;

	FQuestInstance* Instance = Tracker->FindQuestInstance(QuestID);
	const FQuestData* QuestDef = QuestRegistry.Find(QuestID);

	// Broadcast reward events via EventBus
	CacheSubsystems();
	if (QuestDef && CachedEventBus)
	{
		for (const FQuestReward& Reward : QuestDef->Rewards)
		{
			if (Reward.IsValid())
			{
				FGameplayEventPayload Payload = FGameplayEventPayload::Make(Reward.RewardType, Player);
				Payload.PrimaryID = QuestID;
				Payload.SecondaryID = Reward.RewardID;
				Payload.IntValue = Reward.Amount;
				CachedEventBus->BroadcastEvent(Payload);
			}
		}

		// Bonus rewards if applicable
		if (Instance && Instance->bBonusAchieved)
		{
			for (const FQuestReward& Reward : QuestDef->BonusRewards)
			{
				if (Reward.IsValid())
				{
					FGameplayEventPayload Payload = FGameplayEventPayload::Make(Reward.RewardType, Player);
					Payload.PrimaryID = QuestID;
					Payload.SecondaryID = Reward.RewardID;
					Payload.IntValue = Reward.Amount;
					CachedEventBus->BroadcastEvent(Payload);
				}
			}
		}
	}

	// Unregister objectives
	if (Instance && Instance->ObjectiveSetID.IsValid())
	{
		if (CachedObjectiveTracker)
		{
			CachedObjectiveTracker->UnregisterObjectiveSet(Instance->ObjectiveSetID);
		}
		UnregisterSetIDMapping(Instance->ObjectiveSetID);
		Instance->ObjectiveSetID.Invalidate();
	}

	if (TransitionQuestState(QuestID, Player, FWWTagLibrary::Quest_State_TurnedIn()))
	{
		OnQuestTurnedIn.Broadcast(QuestID, Player);
		BroadcastQuestEvent(FWWTagLibrary::Quest_Event_TurnedIn(), QuestID, Player);
		RefreshQuestAvailability(Player);
		return true;
	}
	return false;
}

bool UQuestSubsystem::ForceCompleteQuest(FName QuestID, AActor* Player)
{
	if (!Player) return false;

	FGameplayTag CurrentState = GetQuestState(QuestID, Player);
	if (CurrentState != FWWTagLibrary::Quest_State_Active())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("ForceCompleteQuest: quest '%s' is not active"), *QuestID.ToString());
		return false;
	}

	UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return false;

	FQuestInstance* Instance = Tracker->FindQuestInstance(QuestID);
	if (Instance)
	{
		Instance->bBonusAchieved = true;
	}

	if (TransitionQuestState(QuestID, Player, FWWTagLibrary::Quest_State_Completed()))
	{
		OnQuestCompleted.Broadcast(QuestID, true, Player);
		BroadcastQuestEvent(FWWTagLibrary::Quest_Event_Completed(), QuestID, Player, 1.0f);
		UE_LOG(LogQuestSystem, Log, TEXT("ForceCompleteQuest: '%s' force-completed (debug)"), *QuestID.ToString());
		return true;
	}
	return false;
}

// ============================================================================
// QUERIES
// ============================================================================

FGameplayTag UQuestSubsystem::GetQuestState(FName QuestID, AActor* Player) const
{
	const UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return FWWTagLibrary::Quest_State_Unavailable();

	const FQuestInstance* Instance = Tracker->FindQuestInstance(QuestID);
	if (!Instance) return FWWTagLibrary::Quest_State_Unavailable();

	return Instance->StateTag;
}

bool UQuestSubsystem::MeetsPrerequisites(FName QuestID, AActor* Player) const
{
	const FQuestData* QuestDef = QuestRegistry.Find(QuestID);
	if (!QuestDef) return false;

	const UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return false;

	// Check prerequisite quests (must be TurnedIn)
	for (const FName& PrereqID : QuestDef->PrerequisiteQuestIDs)
	{
		const FQuestInstance* PrereqInstance = Tracker->FindQuestInstance(PrereqID);
		if (!PrereqInstance || PrereqInstance->StateTag != FWWTagLibrary::Quest_State_TurnedIn())
		{
			return false;
		}
	}

	// Check prerequisite tags (player must have all)
	if (QuestDef->PrerequisiteTags.Num() > 0)
	{
		if (!Tracker->GetPlayerTags().HasAll(QuestDef->PrerequisiteTags))
		{
			return false;
		}
	}

	return true;
}

TArray<FName> UQuestSubsystem::GetActiveQuests(AActor* Player) const
{
	TArray<FName> Result;
	const UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return Result;

	for (const FQuestInstance& Instance : Tracker->GetQuestLog())
	{
		if (Instance.StateTag == FWWTagLibrary::Quest_State_Active())
		{
			Result.Add(Instance.QuestID);
		}
	}
	return Result;
}

FName UQuestSubsystem::GetNextChainQuest(FName ChainID, AActor* Player) const
{
	const FQuestChain* Chain = ChainRegistry.Find(ChainID);
	if (!Chain || Chain->QuestSequence.Num() == 0) return NAME_None;

	const UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return Chain->QuestSequence[0];

	for (const FName& QuestID : Chain->QuestSequence)
	{
		const FQuestInstance* Instance = Tracker->FindQuestInstance(QuestID);
		if (!Instance || Instance->StateTag != FWWTagLibrary::Quest_State_TurnedIn())
		{
			return QuestID;
		}
	}

	return NAME_None;
}

float UQuestSubsystem::GetQuestProgress(FName QuestID, AActor* Player) const
{
	const UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return 0.0f;

	const FQuestInstance* Instance = Tracker->FindQuestInstance(QuestID);
	if (!Instance || !Instance->ObjectiveSetID.IsValid()) return 0.0f;

	if (CachedObjectiveTracker)
	{
		return CachedObjectiveTracker->GetSetProgress(Instance->ObjectiveSetID);
	}
	return 0.0f;
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

void UQuestSubsystem::CacheSubsystems()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	if (!CachedObjectiveTracker)
	{
		CachedObjectiveTracker = GI->GetSubsystem<UObjectiveTrackerSubsystem>();
	}

	if (!CachedEventBus)
	{
		CachedEventBus = GI->GetSubsystem<UEventBusSubsystem>();
	}
}

UQuestTrackerComponent* UQuestSubsystem::GetTrackerComponent(AActor* Player) const
{
	if (!Player) return nullptr;
	return Player->FindComponentByClass<UQuestTrackerComponent>();
}

bool UQuestSubsystem::TransitionQuestState(FName QuestID, AActor* Player, const FGameplayTag& NewState)
{
	UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return false;

	FQuestInstance* Instance = Tracker->FindQuestInstance(QuestID);
	if (!Instance)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("TransitionQuestState: no instance for quest '%s'"), *QuestID.ToString());
		return false;
	}

	const FGameplayTag OldState = Instance->StateTag;
	if (OldState == NewState) return false;

	Instance->StateTag = NewState;

	// Set completion timestamp for terminal states
	if (NewState == FWWTagLibrary::Quest_State_Completed() || NewState == FWWTagLibrary::Quest_State_Failed())
	{
		Instance->CompletedTimestamp = Player->GetWorld() ? Player->GetWorld()->GetTimeSeconds() : 0.0f;
	}

	OnQuestStateChanged.Broadcast(QuestID, OldState, NewState, Player);
	UE_LOG(LogQuestSystem, Verbose, TEXT("Quest '%s': %s -> %s"), *QuestID.ToString(), *OldState.ToString(), *NewState.ToString());
	return true;
}

// ============================================================================
// OBJECTIVE TRACKER HANDLERS
// ============================================================================

void UQuestSubsystem::HandleObjectiveSetComplete(const FGuid& SetID, bool bBonusAchieved)
{
	const TPair<FName, TWeakObjectPtr<AActor>>* Mapping = SetIDToQuestMap.Find(SetID);
	if (!Mapping) return;

	const FName QuestID = Mapping->Key;
	AActor* Player = Mapping->Value.Get();
	if (!Player) return;

	UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (Tracker)
	{
		FQuestInstance* Instance = Tracker->FindQuestInstance(QuestID);
		if (Instance)
		{
			Instance->bBonusAchieved = bBonusAchieved;
		}
	}

	if (TransitionQuestState(QuestID, Player, FWWTagLibrary::Quest_State_Completed()))
	{
		OnQuestCompleted.Broadcast(QuestID, bBonusAchieved, Player);
		BroadcastQuestEvent(FWWTagLibrary::Quest_Event_Completed(), QuestID, Player, bBonusAchieved ? 1.0f : 0.0f);
	}
}

void UQuestSubsystem::HandleObjectiveSetFailed(const FGuid& SetID)
{
	const TPair<FName, TWeakObjectPtr<AActor>>* Mapping = SetIDToQuestMap.Find(SetID);
	if (!Mapping) return;

	const FName QuestID = Mapping->Key;
	AActor* Player = Mapping->Value.Get();
	if (!Player) return;

	UnregisterSetIDMapping(SetID);

	if (TransitionQuestState(QuestID, Player, FWWTagLibrary::Quest_State_Failed()))
	{
		OnQuestFailed.Broadcast(QuestID, Player);
		BroadcastQuestEvent(FWWTagLibrary::Quest_Event_Failed(), QuestID, Player);
	}
}

void UQuestSubsystem::HandleObjectiveUpdated(const FGuid& SetID, const FGameplayTag& ObjectiveTag, float NewValue)
{
	const TPair<FName, TWeakObjectPtr<AActor>>* Mapping = SetIDToQuestMap.Find(SetID);
	if (!Mapping) return;

	const FName QuestID = Mapping->Key;
	AActor* Player = Mapping->Value.Get();
	if (!Player) return;

	OnQuestObjectiveProgress.Broadcast(QuestID, ObjectiveTag, NewValue, Player);
	BroadcastQuestEvent(FWWTagLibrary::Quest_Event_ObjectiveProgress(), QuestID, Player, NewValue);
}

void UQuestSubsystem::BroadcastQuestEvent(const FGameplayTag& EventTag, FName QuestID, AActor* Player, float Value)
{
	CacheSubsystems();
	if (!CachedEventBus) return;

	FGameplayEventPayload Payload = FGameplayEventPayload::Make(EventTag, Player);
	Payload.PrimaryID = QuestID;
	Payload.Value = Value;
	CachedEventBus->BroadcastEvent(Payload);
}

void UQuestSubsystem::RegisterSetIDMapping(const FGuid& SetID, FName QuestID, AActor* Player)
{
	SetIDToQuestMap.Add(SetID, TPair<FName, TWeakObjectPtr<AActor>>(QuestID, Player));
}

void UQuestSubsystem::UnregisterSetIDMapping(const FGuid& SetID)
{
	SetIDToQuestMap.Remove(SetID);
}

void UQuestSubsystem::RefreshQuestAvailability(AActor* Player)
{
	if (!Player) return;

	UQuestTrackerComponent* Tracker = GetTrackerComponent(Player);
	if (!Tracker) return;

	for (const auto& Pair : QuestRegistry)
	{
		const FGameplayTag CurrentState = GetQuestState(Pair.Key, Player);

		// Only check quests that are currently Unavailable (no instance yet)
		if (CurrentState == FWWTagLibrary::Quest_State_Unavailable())
		{
			if (MeetsPrerequisites(Pair.Key, Player))
			{
				FQuestInstance Instance;
				Instance.QuestID = Pair.Key;
				Instance.StateTag = FWWTagLibrary::Quest_State_Available();
				Tracker->AddQuestInstance(Instance);

				OnQuestAvailable.Broadcast(Pair.Key, Player);
				UE_LOG(LogQuestSystem, Log, TEXT("Quest '%s' now available for %s"), *Pair.Key.ToString(), *Player->GetName());
			}
		}
	}
}
