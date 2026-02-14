// Copyright Windwalker Productions. All Rights Reserved.

#include "UI/DateTimeWidget_Base.h"
#include "Subsystems/TimeTrackingSubsystem.h"

UDateTimeWidget_Base::UDateTimeWidget_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// ============================================================================
// TEXT QUERIES
// ============================================================================

FText UDateTimeWidget_Base::GetDayNumberText() const
{
	return FText::FromString(FString::Printf(TEXT("Day %d"), GetCurrentDay()));
}

FText UDateTimeWidget_Base::GetTimeOfDayName() const
{
	return TagToDisplayName(GetTimeOfDayTag());
}

FText UDateTimeWidget_Base::GetWeatherName() const
{
	return TagToDisplayName(GetCurrentWeatherTag());
}

FText UDateTimeWidget_Base::GetFullDateTimeText() const
{
	return FText::FromString(FString::Printf(TEXT("Day %d - %s - %s"),
		GetCurrentDay(),
		*GetTimeOfDayName().ToString(),
		*GetWeatherName().ToString()));
}

// ============================================================================
// OVERRIDES
// ============================================================================

void UDateTimeWidget_Base::OnDayChanged_Implementation(int32 OldDay, int32 NewDay)
{
	Super::OnDayChanged_Implementation(OldDay, NewDay);
	OnDateChanged(NewDay);
}

void UDateTimeWidget_Base::OnTimeOfDayPeriodChanged_Implementation(const FGameplayTag& OldPeriod, const FGameplayTag& NewPeriod)
{
	Super::OnTimeOfDayPeriodChanged_Implementation(OldPeriod, NewPeriod);
	OnTimeOfDayPeriodNameChanged(TagToDisplayName(NewPeriod));
}

void UDateTimeWidget_Base::OnDateChanged_Implementation(int32 NewDay)
{
}

void UDateTimeWidget_Base::OnTimeOfDayPeriodNameChanged_Implementation(const FText& PeriodName)
{
}

// ============================================================================
// HELPERS
// ============================================================================

FText UDateTimeWidget_Base::TagToDisplayName(const FGameplayTag& Tag)
{
	if (!Tag.IsValid())
	{
		return FText::FromString(TEXT("Unknown"));
	}

	// Extract last segment: "Time.State.Morning" -> "Morning"
	const FString TagString = Tag.ToString();
	int32 LastDot;
	if (TagString.FindLastChar(TEXT('.'), LastDot))
	{
		return FText::FromString(TagString.Mid(LastDot + 1));
	}

	return FText::FromString(TagString);
}
