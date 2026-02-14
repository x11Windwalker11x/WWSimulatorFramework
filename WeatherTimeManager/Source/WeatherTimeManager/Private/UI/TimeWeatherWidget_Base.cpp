// Copyright Windwalker Productions. All Rights Reserved.

#include "UI/TimeWeatherWidget_Base.h"
#include "Subsystems/TimeTrackingSubsystem.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

UTimeWeatherWidget_Base::UTimeWeatherWidget_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetCategoryTag = FWWTagLibrary::UI_Widget_Category_TimeWeather();
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UTimeWeatherWidget_Base::NativeConstruct()
{
	Super::NativeConstruct();

	// Cache subsystem ref (Rule #41)
	CachedSubsystem = UTimeTrackingSubsystem::Get(this);
	BindDelegates();
}

void UTimeWeatherWidget_Base::NativeDestruct()
{
	UnbindDelegates();
	Super::NativeDestruct();
}

// ============================================================================
// STATE QUERIES
// ============================================================================

float UTimeWeatherWidget_Base::GetCurrentHour() const
{
	if (UTimeTrackingSubsystem* Sub = CachedSubsystem.Get())
	{
		return Sub->GetCurrentHour();
	}
	return 0.0f;
}

int32 UTimeWeatherWidget_Base::GetCurrentDay() const
{
	if (UTimeTrackingSubsystem* Sub = CachedSubsystem.Get())
	{
		return Sub->GetCurrentDay();
	}
	return 1;
}

FGameplayTag UTimeWeatherWidget_Base::GetTimeOfDayTag() const
{
	if (UTimeTrackingSubsystem* Sub = CachedSubsystem.Get())
	{
		return Sub->GetTimeOfDayTag();
	}
	return FGameplayTag();
}

FGameplayTag UTimeWeatherWidget_Base::GetCurrentWeatherTag() const
{
	if (UTimeTrackingSubsystem* Sub = CachedSubsystem.Get())
	{
		return Sub->GetCurrentWeatherTag();
	}
	return FGameplayTag();
}

bool UTimeWeatherWidget_Base::IsTimePaused() const
{
	if (UTimeTrackingSubsystem* Sub = CachedSubsystem.Get())
	{
		return Sub->IsTimePaused();
	}
	return false;
}

float UTimeWeatherWidget_Base::GetTimeSpeed() const
{
	if (UTimeTrackingSubsystem* Sub = CachedSubsystem.Get())
	{
		return Sub->GetTimeSpeed();
	}
	return 1.0f;
}

// ============================================================================
// BLUEPRINT NATIVE EVENT DEFAULTS
// ============================================================================

void UTimeWeatherWidget_Base::OnHourChanged_Implementation(int32 OldHour, int32 NewHour) {}
void UTimeWeatherWidget_Base::OnDayChanged_Implementation(int32 OldDay, int32 NewDay) {}
void UTimeWeatherWidget_Base::OnTimeOfDayPeriodChanged_Implementation(const FGameplayTag& OldPeriod, const FGameplayTag& NewPeriod) {}
void UTimeWeatherWidget_Base::OnWeatherStateChanged_Implementation(const FGameplayTag& OldWeather, const FGameplayTag& NewWeather) {}
void UTimeWeatherWidget_Base::OnTimePausedEvent_Implementation() {}
void UTimeWeatherWidget_Base::OnTimeResumedEvent_Implementation() {}

// ============================================================================
// DELEGATE HANDLERS
// ============================================================================

void UTimeWeatherWidget_Base::HandleHourChanged(int32 OldHour, int32 NewHour)
{
	OnHourChanged(OldHour, NewHour);
}

void UTimeWeatherWidget_Base::HandleDayChanged(int32 OldDay, int32 NewDay)
{
	OnDayChanged(OldDay, NewDay);
}

void UTimeWeatherWidget_Base::HandleTimeOfDayChanged(const FGameplayTag& OldPeriod, const FGameplayTag& NewPeriod)
{
	OnTimeOfDayPeriodChanged(OldPeriod, NewPeriod);
}

void UTimeWeatherWidget_Base::HandleWeatherChanged(const FGameplayTag& OldWeather, const FGameplayTag& NewWeather)
{
	OnWeatherStateChanged(OldWeather, NewWeather);
}

void UTimeWeatherWidget_Base::HandleTimePaused()
{
	OnTimePausedEvent();
}

void UTimeWeatherWidget_Base::HandleTimeResumed()
{
	OnTimeResumedEvent();
}

// ============================================================================
// BIND / UNBIND
// ============================================================================

void UTimeWeatherWidget_Base::BindDelegates()
{
	UTimeTrackingSubsystem* Sub = CachedSubsystem.Get();
	if (!Sub)
	{
		return;
	}

	Sub->OnHourChanged.AddDynamic(this, &UTimeWeatherWidget_Base::HandleHourChanged);
	Sub->OnDayChanged.AddDynamic(this, &UTimeWeatherWidget_Base::HandleDayChanged);
	Sub->OnTimeOfDayChanged.AddDynamic(this, &UTimeWeatherWidget_Base::HandleTimeOfDayChanged);
	Sub->OnWeatherChanged.AddDynamic(this, &UTimeWeatherWidget_Base::HandleWeatherChanged);
	Sub->OnTimePaused.AddDynamic(this, &UTimeWeatherWidget_Base::HandleTimePaused);
	Sub->OnTimeResumed.AddDynamic(this, &UTimeWeatherWidget_Base::HandleTimeResumed);
}

void UTimeWeatherWidget_Base::UnbindDelegates()
{
	UTimeTrackingSubsystem* Sub = CachedSubsystem.Get();
	if (!Sub)
	{
		return;
	}

	Sub->OnHourChanged.RemoveDynamic(this, &UTimeWeatherWidget_Base::HandleHourChanged);
	Sub->OnDayChanged.RemoveDynamic(this, &UTimeWeatherWidget_Base::HandleDayChanged);
	Sub->OnTimeOfDayChanged.RemoveDynamic(this, &UTimeWeatherWidget_Base::HandleTimeOfDayChanged);
	Sub->OnWeatherChanged.RemoveDynamic(this, &UTimeWeatherWidget_Base::HandleWeatherChanged);
	Sub->OnTimePaused.RemoveDynamic(this, &UTimeWeatherWidget_Base::HandleTimePaused);
	Sub->OnTimeResumed.RemoveDynamic(this, &UTimeWeatherWidget_Base::HandleTimeResumed);
}
