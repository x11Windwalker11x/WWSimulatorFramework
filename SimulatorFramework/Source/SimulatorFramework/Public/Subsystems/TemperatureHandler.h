// TemperatureHandler.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/MiniGameHandlerBase.h"
#include "Lib/Data/Core/InputConfigData.h"
#include "TemperatureHandler.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTemperatureChanged, float, CurrentTemp, float, TargetProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZoneEntered, bool, bOptimalZone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRuined, const FString&, Reason);

/**
 * Handler for temperature control mini-games
 * Cooking, smithing, heat management
 */
UCLASS(BlueprintType, Blueprintable)
class SIMULATORFRAMEWORK_API UTemperatureHandler : public UMiniGameHandlerBase
{
    GENERATED_BODY()

public:
    // === LIFECYCLE ===

    virtual void Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID) override;
    virtual void Activate() override;
    virtual void Deactivate() override;
    virtual void TickHandler(float DeltaTime) override;
    virtual bool NeedsTick() const override { return true; }

    // === INPUT ===

    virtual void ProcessAxisInput(const FVector2D& AxisValue, float DeltaTime) override;
    virtual void ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed) override;

    // === TEMPERATURE STATE ===

    /** Get current temperature */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    float GetCurrentTemperature() const { return CurrentTemperature; }

    /** Get optimal zone min */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    float GetOptimalMin() const { return TempConfig.OptimalMin; }

    /** Get optimal zone max */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    float GetOptimalMax() const { return TempConfig.OptimalMax; }

    /** Get danger zone min */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    float GetDangerMin() const { return TempConfig.DangerMin; }

    /** Get danger zone max */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    float GetDangerMax() const { return TempConfig.DangerMax; }

    /** Check if in optimal zone */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    bool IsInOptimalZone() const { return bInOptimalZone; }

    /** Check if in danger zone */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    bool IsInDangerZone() const { return bInDangerZone; }

    /** Get time spent in optimal zone */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    float GetTimeInOptimalZone() const { return TimeInOptimalZone; }

    /** Get required time in optimal zone */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    float GetRequiredTimeInZone() const { return TempConfig.RequiredDurationInZone; }

    /** Get current heat input (0-1) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    float GetHeatInput() const { return CurrentHeatInput; }

    /** Check if item is ruined */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Temperature")
    bool IsRuined() const { return bIsRuined; }

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Temperature")
    FOnTemperatureChanged OnTemperatureChanged;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Temperature")
    FOnZoneEntered OnZoneEntered;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Temperature")
    FOnItemRuined OnItemRuined;

protected:
    // === CONFIG ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Temperature")
    FTemperatureZoneConfig TempConfig;

    // === STATE ===

    /** Current temperature */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Temperature")
    float CurrentTemperature = 0.0f;

    /** Current heat input (0-1, from player) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Temperature")
    float CurrentHeatInput = 0.0f;

    /** Is heat button held */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Temperature")
    bool bIsHeating = false;

    /** Is in optimal zone */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Temperature")
    bool bInOptimalZone = false;

    /** Is in danger zone */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Temperature")
    bool bInDangerZone = false;

    /** Time accumulated in optimal zone */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Temperature")
    float TimeInOptimalZone = 0.0f;

    /** Is item ruined */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Temperature")
    bool bIsRuined = false;

    /** Was in optimal zone last frame */
    bool bWasInOptimalZone = false;

    /** Was in danger zone last frame */
    bool bWasInDangerZone = false;

    // === INTERNAL ===

    /** Update temperature based on input */
    void UpdateTemperature(float DeltaTime);

    /** Check zone states */
    void UpdateZoneStates();

    /** Handle entering optimal zone */
    void EnterOptimalZone();

    /** Handle leaving optimal zone */
    void LeaveOptimalZone();

    /** Handle entering danger zone */
    void EnterDangerZone();

    /** Handle item ruined (overheat/freeze) */
    void RuinItem(const FString& Reason);

    /** Report progress */
    void ReportTemperatureProgress();

    /** Get starting temperature */
    float GetStartingTemperature() const;
};