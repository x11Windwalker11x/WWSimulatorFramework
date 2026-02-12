// Copyright Windwalker Productions. All Rights Reserved.

#include "Components/QuestTrackerComponent.h"
#include "Subsystems/QuestSubsystem.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuestTracker, Log, All);

UQuestTrackerComponent::UQuestTrackerComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UQuestTrackerComponent::BeginPlay()
{
	Super::BeginPlay();
	CacheSubsystem();
}

void UQuestTrackerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UQuestTrackerComponent, QuestLog, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UQuestTrackerComponent, PlayerTags, COND_OwnerOnly);
}

// ============================================================================
// QUEST LOG ACCESS
// ============================================================================

void UQuestTrackerComponent::AddQuestInstance(const FQuestInstance& Instance)
{
	if (!Instance.IsValid())
	{
		UE_LOG(LogQuestTracker, Warning, TEXT("AddQuestInstance: invalid instance"));
		return;
	}

	// Check for max active quests
	if (MaxActiveQuests > 0 && Instance.StateTag == FWWTagLibrary::Quest_State_Active())
	{
		if (GetActiveQuestCount() >= MaxActiveQuests)
		{
			UE_LOG(LogQuestTracker, Warning, TEXT("AddQuestInstance: at max active quests (%d)"), MaxActiveQuests);
			return;
		}
	}

	QuestLog.Add(Instance);
	UE_LOG(LogQuestTracker, Verbose, TEXT("Added quest '%s' to log"), *Instance.QuestID.ToString());
}

FQuestInstance* UQuestTrackerComponent::FindQuestInstance(FName QuestID)
{
	for (FQuestInstance& Instance : QuestLog)
	{
		if (Instance.QuestID == QuestID)
		{
			return &Instance;
		}
	}
	return nullptr;
}

const FQuestInstance* UQuestTrackerComponent::FindQuestInstance(FName QuestID) const
{
	for (const FQuestInstance& Instance : QuestLog)
	{
		if (Instance.QuestID == QuestID)
		{
			return &Instance;
		}
	}
	return nullptr;
}

int32 UQuestTrackerComponent::GetActiveQuestCount() const
{
	int32 Count = 0;
	for (const FQuestInstance& Instance : QuestLog)
	{
		if (Instance.StateTag == FWWTagLibrary::Quest_State_Active())
		{
			Count++;
		}
	}
	return Count;
}

bool UQuestTrackerComponent::IsAtMaxActiveQuests() const
{
	if (MaxActiveQuests <= 0) return false;
	return GetActiveQuestCount() >= MaxActiveQuests;
}

// ============================================================================
// PLAYER TAGS
// ============================================================================

void UQuestTrackerComponent::AddPlayerTag(FGameplayTag Tag)
{
	if (Tag.IsValid())
	{
		PlayerTags.AddTag(Tag);
	}
}

void UQuestTrackerComponent::RemovePlayerTag(FGameplayTag Tag)
{
	if (Tag.IsValid())
	{
		PlayerTags.RemoveTag(Tag);
	}
}

bool UQuestTrackerComponent::HasPlayerTag(FGameplayTag Tag) const
{
	return PlayerTags.HasTag(Tag);
}

// ============================================================================
// SERVER RPCs
// ============================================================================

void UQuestTrackerComponent::Server_RequestAcceptQuest_Implementation(FName QuestID)
{
	CacheSubsystem();
	if (!CachedQuestSubsystem)
	{
		UE_LOG(LogQuestTracker, Warning, TEXT("Server_RequestAcceptQuest: no QuestSubsystem"));
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogQuestTracker, Warning, TEXT("Server_RequestAcceptQuest: no owner"));
		return;
	}

	CachedQuestSubsystem->AcceptQuest(QuestID, Owner);
}

void UQuestTrackerComponent::Server_RequestAbandonQuest_Implementation(FName QuestID)
{
	CacheSubsystem();
	if (!CachedQuestSubsystem)
	{
		UE_LOG(LogQuestTracker, Warning, TEXT("Server_RequestAbandonQuest: no QuestSubsystem"));
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogQuestTracker, Warning, TEXT("Server_RequestAbandonQuest: no owner"));
		return;
	}

	CachedQuestSubsystem->AbandonQuest(QuestID, Owner);
}

void UQuestTrackerComponent::Server_RequestTurnInQuest_Implementation(FName QuestID)
{
	CacheSubsystem();
	if (!CachedQuestSubsystem)
	{
		UE_LOG(LogQuestTracker, Warning, TEXT("Server_RequestTurnInQuest: no QuestSubsystem"));
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogQuestTracker, Warning, TEXT("Server_RequestTurnInQuest: no owner"));
		return;
	}

	CachedQuestSubsystem->TurnInQuest(QuestID, Owner);
}

// ============================================================================
// INTERNAL
// ============================================================================

void UQuestTrackerComponent::OnRep_QuestLog()
{
	UE_LOG(LogQuestTracker, Verbose, TEXT("QuestLog replicated (%d entries)"), QuestLog.Num());
}

void UQuestTrackerComponent::OnRep_PlayerTags()
{
	UE_LOG(LogQuestTracker, Verbose, TEXT("PlayerTags replicated (%d tags)"), PlayerTags.Num());
}

void UQuestTrackerComponent::CacheSubsystem()
{
	if (!CachedQuestSubsystem)
	{
		CachedQuestSubsystem = UQuestSubsystem::Get(this);
	}
}
