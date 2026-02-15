// Copyright Windwalker Productions. All Rights Reserved.

#include "Subsystems/DaySummarySubsystem.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

void UDaySummarySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UDaySummarySubsystem::Deinitialize()
{
	PendingEntries.Empty();
	Super::Deinitialize();
}

UDaySummarySubsystem* UDaySummarySubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UDaySummarySubsystem>();
}

void UDaySummarySubsystem::RequestDaySummary(int32 Day, float HoursSlept)
{
	// Clear any stale entries from a previous request
	PendingEntries.Empty();

	// Broadcast to all contributors — they call SubmitEntry() synchronously
	OnDaySummaryRequested.Broadcast(Day, HoursSlept);

	// Schedule finalize on next tick so contributors in latent paths can also submit
	if (const UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &UDaySummarySubsystem::FinalizeAndBroadcast);
	}
	else
	{
		// Fallback: finalize immediately
		FinalizeAndBroadcast();
	}
}

void UDaySummarySubsystem::SubmitEntry(const FDaySummaryEntry& Entry)
{
	if (Entry.IsValid())
	{
		PendingEntries.Add(Entry);
	}
}

void UDaySummarySubsystem::FinalizeAndBroadcast()
{
	// Sort: by CategoryTag name first, then by SortOrder within category
	PendingEntries.Sort([](const FDaySummaryEntry& A, const FDaySummaryEntry& B)
	{
		const FString TagA = A.CategoryTag.ToString();
		const FString TagB = B.CategoryTag.ToString();

		if (TagA != TagB)
		{
			return TagA < TagB;
		}

		return A.SortOrder < B.SortOrder;
	});

	OnDaySummaryReady.Broadcast(PendingEntries);

	UE_LOG(LogTemp, Log, TEXT("DaySummary: Finalized %d entries"), PendingEntries.Num());
}
