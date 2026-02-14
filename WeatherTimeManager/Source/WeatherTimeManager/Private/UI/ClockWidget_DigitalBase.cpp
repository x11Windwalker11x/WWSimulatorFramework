// Copyright Windwalker Productions. All Rights Reserved.

#include "UI/ClockWidget_DigitalBase.h"
#include "Subsystems/TimeTrackingSubsystem.h"

UClockWidget_DigitalBase::UClockWidget_DigitalBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// ============================================================================
// FORMAT QUERIES
// ============================================================================

FText UClockWidget_DigitalBase::GetFormattedTime12H() const
{
	const float Hour = GetCurrentHour();
	const int32 Hour24 = FMath::FloorToInt32(Hour);
	const int32 Minute = FMath::FloorToInt32(FMath::Fmod(Hour, 1.0f) * 60.0f);

	int32 Hour12 = Hour24 % 12;
	if (Hour12 == 0)
	{
		Hour12 = 12;
	}

	const TCHAR* AMPM = (Hour24 < 12) ? TEXT("AM") : TEXT("PM");
	return FText::FromString(FString::Printf(TEXT("%d:%02d %s"), Hour12, Minute, AMPM));
}

FText UClockWidget_DigitalBase::GetFormattedTime24H() const
{
	const float Hour = GetCurrentHour();
	const int32 Hour24 = FMath::FloorToInt32(Hour);
	const int32 Minute = FMath::FloorToInt32(FMath::Fmod(Hour, 1.0f) * 60.0f);

	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Hour24, Minute));
}

FText UClockWidget_DigitalBase::GetFormattedTimeWithSeconds() const
{
	const float Hour = GetCurrentHour();
	const int32 Hour24 = FMath::FloorToInt32(Hour);
	const float FractionalHour = FMath::Fmod(Hour, 1.0f) * 60.0f;
	const int32 Minute = FMath::FloorToInt32(FractionalHour);
	const int32 Second = FMath::FloorToInt32(FMath::Fmod(FractionalHour, 1.0f) * 60.0f);

	return FText::FromString(FString::Printf(TEXT("%02d:%02d:%02d"), Hour24, Minute, Second));
}

// ============================================================================
// OVERRIDES
// ============================================================================

void UClockWidget_DigitalBase::OnHourChanged_Implementation(int32 OldHour, int32 NewHour)
{
	Super::OnHourChanged_Implementation(OldHour, NewHour);

	const FText DisplayTime = bUse24HourFormat ? GetFormattedTime24H() : GetFormattedTime12H();
	OnTimeDisplayUpdated(DisplayTime);
}

void UClockWidget_DigitalBase::OnTimeDisplayUpdated_Implementation(const FText& FormattedTime)
{
}
