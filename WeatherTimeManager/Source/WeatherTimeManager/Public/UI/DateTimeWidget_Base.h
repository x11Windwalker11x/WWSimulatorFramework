// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/TimeWeatherWidget_Base.h"
#include "DateTimeWidget_Base.generated.h"

/**
 * Date/time/weather combined display widget base class.
 * Shows day number, time-of-day period name, weather name, and full datetime text.
 * BP subclass binds texts to UTextBlock widgets.
 */
UCLASS(Abstract, Blueprintable)
class WEATHERTIMEMANAGER_API UDateTimeWidget_Base : public UTimeWeatherWidget_Base
{
	GENERATED_BODY()

public:
	UDateTimeWidget_Base(const FObjectInitializer& ObjectInitializer);

	// ============================================================================
	// TEXT QUERIES (BlueprintPure)
	// ============================================================================

	/** Get day number as text (e.g. "Day 5") */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|DateTime")
	FText GetDayNumberText() const;

	/** Get time-of-day period name (e.g. "Morning") */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|DateTime")
	FText GetTimeOfDayName() const;

	/** Get current weather name (e.g. "Rain") */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|DateTime")
	FText GetWeatherName() const;

	/** Get full date/time text (e.g. "Day 5 - Morning - Clear") */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|DateTime")
	FText GetFullDateTimeText() const;

protected:
	/** Called when the date (day number) changes */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|DateTime")
	void OnDateChanged(int32 NewDay);

	/** Called when the time-of-day period changes */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|DateTime")
	void OnTimeOfDayPeriodNameChanged(const FText& PeriodName);

	virtual void OnDayChanged_Implementation(int32 OldDay, int32 NewDay) override;
	virtual void OnTimeOfDayPeriodChanged_Implementation(const FGameplayTag& OldPeriod, const FGameplayTag& NewPeriod) override;

private:
	/** Extract readable name from a gameplay tag (last segment) */
	static FText TagToDisplayName(const FGameplayTag& Tag);
};
