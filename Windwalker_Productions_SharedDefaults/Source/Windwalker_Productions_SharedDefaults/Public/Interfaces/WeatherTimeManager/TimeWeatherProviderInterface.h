// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "TimeWeatherProviderInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UTimeWeatherProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that provide sky/atmosphere visuals driven by the TimeTrackingSubsystem.
 * Implementors (e.g. BP_SkySphere, volumetric cloud actors) receive time and weather state updates.
 * Rule #29: Mandatory getter for component access.
 */
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API ITimeWeatherProviderInterface
{
	GENERATED_BODY()

public:
	/** Set the visual time-of-day (hour 0-24) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TimeWeather|Provider")
	void SetTimeOfDay(float Hour);

	/** Get current visual time-of-day */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TimeWeather|Provider")
	float GetCurrentTimeOfDay() const;

	/** Set the weather state (tag + intensity) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TimeWeather|Provider")
	void SetWeatherState(const FGameplayTag& WeatherTag, float Intensity);

	/** Get the current weather tag being displayed */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TimeWeather|Provider")
	FGameplayTag GetCurrentWeatherTag() const;

	/** Set weather transition blend alpha (0.0 = old weather, 1.0 = new weather) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TimeWeather|Provider")
	void SetWeatherTransitionAlpha(float Alpha);

	/** Mandatory getter: return the component that drives visuals (Rule #29) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TimeWeather|Provider")
	UActorComponent* GetTimeWeatherComponent();
};
