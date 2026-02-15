// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/WeatherTimeManager/DaySummaryDelegates.h"
#include "Lib/Data/WeatherTimeManager/DaySummaryData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DaySummarySubsystem.generated.h"

/**
 * UDaySummarySubsystem
 *
 * MSB (L0.5) collection point for day-end summary entries.
 * Solves the L2-to-L2 problem: any L2 plugin can bind to OnDaySummaryRequested
 * and submit entries without knowing about other L2 plugins.
 *
 * Flow:
 *   CompleteSleep() -> RequestDaySummary(Day, Hours)
 *     -> Broadcasts OnDaySummaryRequested
 *     -> Contributors call SubmitEntry(FDaySummaryEntry)
 *     -> NextTick: FinalizeAndBroadcast() fires OnDaySummaryReady
 *     -> Widget receives sorted entries
 */
UCLASS()
class MODULARSYSTEMSBASE_API UDaySummarySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Static accessor */
	UFUNCTION(BlueprintPure, Category = "DaySummary", meta = (WorldContext = "WorldContextObject"))
	static UDaySummarySubsystem* Get(const UObject* WorldContextObject);

	// ============================================================================
	// DELEGATES
	// ============================================================================

	/** Broadcast to all contributors when summary is requested */
	UPROPERTY(BlueprintAssignable, Category = "DaySummary|Delegates")
	FOnDaySummaryRequested OnDaySummaryRequested;

	/** Broadcast when sorted entries are ready for display */
	UPROPERTY(BlueprintAssignable, Category = "DaySummary|Delegates")
	FOnDaySummaryReady OnDaySummaryReady;

	// ============================================================================
	// API
	// ============================================================================

	/**
	 * Request a day summary. Clears pending entries, broadcasts to contributors,
	 * then schedules NextTick finalize.
	 * @param Day Current day number
	 * @param HoursSlept Hours the player slept
	 */
	UFUNCTION(BlueprintCallable, Category = "DaySummary")
	void RequestDaySummary(int32 Day, float HoursSlept);

	/**
	 * Submit an entry during the collection frame.
	 * Called by L2 contributors in response to OnDaySummaryRequested.
	 * @param Entry The summary entry to add
	 */
	UFUNCTION(BlueprintCallable, Category = "DaySummary")
	void SubmitEntry(const FDaySummaryEntry& Entry);

private:
	/** Pending entries collected during the request frame */
	TArray<FDaySummaryEntry> PendingEntries;

	/** Sort by CategoryTag name then SortOrder, and broadcast OnDaySummaryReady */
	void FinalizeAndBroadcast();
};
