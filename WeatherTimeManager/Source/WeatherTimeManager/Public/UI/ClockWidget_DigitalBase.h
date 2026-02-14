// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/TimeWeatherWidget_Base.h"
#include "ClockWidget_DigitalBase.generated.h"

/**
 * Digital clock widget base class.
 * Provides formatted time strings for 12h/24h display.
 * BP subclass binds text to UTextBlock for visual clock.
 */
UCLASS(Abstract, Blueprintable)
class WEATHERTIMEMANAGER_API UClockWidget_DigitalBase : public UTimeWeatherWidget_Base
{
	GENERATED_BODY()

public:
	UClockWidget_DigitalBase(const FObjectInitializer& ObjectInitializer);

	// ============================================================================
	// FORMAT QUERIES (BlueprintPure)
	// ============================================================================

	/** Get formatted time in 12-hour format (e.g. "1:30 PM") */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|Digital")
	FText GetFormattedTime12H() const;

	/** Get formatted time in 24-hour format (e.g. "13:30") */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|Digital")
	FText GetFormattedTime24H() const;

	/** Get formatted time with seconds (e.g. "13:30:45") */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|Digital")
	FText GetFormattedTimeWithSeconds() const;

protected:
	/** Whether to use 24-hour format by default */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TimeWeather|Widget|Digital")
	bool bUse24HourFormat = false;

	/** Called when the display text updates */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|Digital")
	void OnTimeDisplayUpdated(const FText& FormattedTime);

	virtual void OnHourChanged_Implementation(int32 OldHour, int32 NewHour) override;
};
