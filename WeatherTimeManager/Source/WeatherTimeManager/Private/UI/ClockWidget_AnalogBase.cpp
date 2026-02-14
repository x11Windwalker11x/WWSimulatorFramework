// Copyright Windwalker Productions. All Rights Reserved.

#include "UI/ClockWidget_AnalogBase.h"
#include "Subsystems/TimeTrackingSubsystem.h"

UClockWidget_AnalogBase::UClockWidget_AnalogBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// ============================================================================
// ANGLE QUERIES
// ============================================================================

float UClockWidget_AnalogBase::GetHourHandAngle() const
{
	const float Hour = GetCurrentHour();
	// 12-hour clock: each hour = 30 degrees, plus minute contribution
	const float Hour12 = FMath::Fmod(Hour, 12.0f);
	return Hour12 * 30.0f; // 360 / 12 = 30 degrees per hour
}

float UClockWidget_AnalogBase::GetMinuteHandAngle() const
{
	const float Hour = GetCurrentHour();
	const float Minutes = FMath::Fmod(Hour, 1.0f) * 60.0f;
	return Minutes * 6.0f; // 360 / 60 = 6 degrees per minute
}

float UClockWidget_AnalogBase::GetSecondHandAngle() const
{
	const float Hour = GetCurrentHour();
	const float FractionalMinutes = FMath::Fmod(Hour * 60.0f, 1.0f);
	const float Seconds = FractionalMinutes * 60.0f;
	return Seconds * 6.0f; // 360 / 60 = 6 degrees per second
}

// ============================================================================
// OVERRIDES
// ============================================================================

void UClockWidget_AnalogBase::OnHourChanged_Implementation(int32 OldHour, int32 NewHour)
{
	Super::OnHourChanged_Implementation(OldHour, NewHour);

	// Check minute change
	const float Hour = GetCurrentHour();
	const int32 CurrentMinute = FMath::FloorToInt32(FMath::Fmod(Hour, 1.0f) * 60.0f);

	if (CurrentMinute != CachedMinute)
	{
		const int32 OldMinute = CachedMinute;
		CachedMinute = CurrentMinute;
		OnMinuteChanged(OldMinute, CurrentMinute);
	}
}

void UClockWidget_AnalogBase::OnMinuteChanged_Implementation(int32 OldMinute, int32 NewMinute)
{
}
