// Copyright Windwalker Productions. All Rights Reserved.

#include "UI/DaySummaryWidget_Base.h"
#include "Subsystems/DaySummarySubsystem.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UDaySummaryWidget_Base::NativeDestruct()
{
	// Unbind from summary subsystem
	if (UDaySummarySubsystem* Sub = GetSummarySubsystem())
	{
		Sub->OnDaySummaryReady.RemoveDynamic(this, &UDaySummaryWidget_Base::HandleSummaryReady);
	}

	Super::NativeDestruct();
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UDaySummaryWidget_Base::InitializeSummary(int32 Day, float HoursSlept)
{
	CachedDayNumber = Day;
	CachedHoursSlept = HoursSlept;

	// Bind to receive sorted entries
	if (UDaySummarySubsystem* Sub = GetSummarySubsystem())
	{
		Sub->OnDaySummaryReady.AddDynamic(this, &UDaySummaryWidget_Base::HandleSummaryReady);
	}
}

// ============================================================================
// BLUEPRINT NATIVE EVENT DEFAULTS
// ============================================================================

void UDaySummaryWidget_Base::OnSummaryEntriesReceived_Implementation(const TArray<FDaySummaryEntry>& Entries)
{
	// BP override point
}

void UDaySummaryWidget_Base::OnSummaryClosed_Implementation()
{
	// BP override point
}

// ============================================================================
// CLOSE
// ============================================================================

void UDaySummaryWidget_Base::CloseSummary()
{
	OnSummaryClosed();
	RemoveFromParent();
}

// ============================================================================
// DELEGATE HANDLER
// ============================================================================

void UDaySummaryWidget_Base::HandleSummaryReady(const TArray<FDaySummaryEntry>& Entries)
{
	CachedEntries = Entries;
	OnSummaryEntriesReceived(Entries);

	// Unbind after receiving (one-shot)
	if (UDaySummarySubsystem* Sub = GetSummarySubsystem())
	{
		Sub->OnDaySummaryReady.RemoveDynamic(this, &UDaySummaryWidget_Base::HandleSummaryReady);
	}
}

// ============================================================================
// SUBSYSTEM ACCESS
// ============================================================================

UDaySummarySubsystem* UDaySummaryWidget_Base::GetSummarySubsystem()
{
	if (CachedSummarySubsystem.IsValid())
	{
		return CachedSummarySubsystem.Get();
	}

	UDaySummarySubsystem* Sub = UDaySummarySubsystem::Get(this);
	CachedSummarySubsystem = Sub;
	return Sub;
}
