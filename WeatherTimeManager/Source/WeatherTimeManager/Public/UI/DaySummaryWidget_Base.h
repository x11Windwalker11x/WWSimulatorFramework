// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "Lib/Data/WeatherTimeManager/DaySummaryData.h"
#include "DaySummaryWidget_Base.generated.h"

class UDaySummarySubsystem;

/**
 * UDaySummaryWidget_Base
 *
 * Base widget for the Stardew Valley-style day-end summary screen (Rule #46: extends UManagedWidget_Master).
 * Lives in WeatherTimeManager/UI/ (Rule #47: widget in owning plugin).
 * Blueprint-extendable for visual customization.
 *
 * Receives sorted FDaySummaryEntry array from UDaySummarySubsystem.
 * Requires player input to close (Continue button or ESC).
 */
UCLASS(Abstract)
class WEATHERTIMEMANAGER_API UDaySummaryWidget_Base : public UManagedWidget_Master
{
	GENERATED_BODY()

public:
	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the summary screen with context data.
	 * Binds to OnDaySummaryReady and stores day/sleep info.
	 * @param Day Current day number
	 * @param HoursSlept Hours the player slept
	 */
	UFUNCTION(BlueprintCallable, Category = "DaySummary|Widget")
	void InitializeSummary(int32 Day, float HoursSlept);

	// ============================================================================
	// BLUEPRINT PURE GETTERS
	// ============================================================================

	/** Get the day number this summary is for */
	UFUNCTION(BlueprintPure, Category = "DaySummary|Widget")
	int32 GetDayNumber() const { return CachedDayNumber; }

	/** Get hours slept */
	UFUNCTION(BlueprintPure, Category = "DaySummary|Widget")
	float GetHoursSlept() const { return CachedHoursSlept; }

	/** Get the summary entries (available after OnSummaryEntriesReceived) */
	UFUNCTION(BlueprintPure, Category = "DaySummary|Widget")
	const TArray<FDaySummaryEntry>& GetSummaryEntries() const { return CachedEntries; }

protected:
	virtual void NativeDestruct() override;

	// ============================================================================
	// BLUEPRINT NATIVE EVENTS
	// ============================================================================

	/** Called when sorted entries are received and ready for display */
	UFUNCTION(BlueprintNativeEvent, Category = "DaySummary|Widget")
	void OnSummaryEntriesReceived(const TArray<FDaySummaryEntry>& Entries);

	/** Called when the summary screen is closed by the player */
	UFUNCTION(BlueprintNativeEvent, Category = "DaySummary|Widget")
	void OnSummaryClosed();

	/** Close the summary (call from Blueprint Continue button or ESC) */
	UFUNCTION(BlueprintCallable, Category = "DaySummary|Widget")
	void CloseSummary();

private:
	/** Cached day number */
	int32 CachedDayNumber = 0;

	/** Cached hours slept */
	float CachedHoursSlept = 0.0f;

	/** Cached summary entries */
	TArray<FDaySummaryEntry> CachedEntries;

	/** Cached subsystem ref (Rule #41) */
	UPROPERTY()
	TWeakObjectPtr<UDaySummarySubsystem> CachedSummarySubsystem;

	/** Get the day summary subsystem */
	UDaySummarySubsystem* GetSummarySubsystem();

	/** Delegate handler for OnDaySummaryReady */
	UFUNCTION()
	void HandleSummaryReady(const TArray<FDaySummaryEntry>& Entries);
};
