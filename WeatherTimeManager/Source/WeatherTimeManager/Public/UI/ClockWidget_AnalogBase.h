// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/TimeWeatherWidget_Base.h"
#include "ClockWidget_AnalogBase.generated.h"

/**
 * Analog clock widget base class.
 * Provides hand angle calculations for hour, minute, second hands.
 * BP subclass binds RotateAngle to image widgets for visual clock.
 */
UCLASS(Abstract, Blueprintable)
class WEATHERTIMEMANAGER_API UClockWidget_AnalogBase : public UTimeWeatherWidget_Base
{
	GENERATED_BODY()

public:
	UClockWidget_AnalogBase(const FObjectInitializer& ObjectInitializer);

	// ============================================================================
	// ANGLE QUERIES (BlueprintPure)
	// ============================================================================

	/** Get hour hand angle in degrees (0-360, 12 o'clock = 0) */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|Analog")
	float GetHourHandAngle() const;

	/** Get minute hand angle in degrees (0-360, 12 o'clock = 0) */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|Analog")
	float GetMinuteHandAngle() const;

	/** Get second hand angle in degrees (0-360, 12 o'clock = 0) */
	UFUNCTION(BlueprintPure, Category = "TimeWeather|Widget|Analog")
	float GetSecondHandAngle() const;

protected:
	/** Called when the minute changes (derived from hour fractional) */
	UFUNCTION(BlueprintNativeEvent, Category = "TimeWeather|Widget|Analog")
	void OnMinuteChanged(int32 OldMinute, int32 NewMinute);

	virtual void OnHourChanged_Implementation(int32 OldHour, int32 NewHour) override;

private:
	/** Cached previous minute for change detection */
	int32 CachedMinute = -1;
};
