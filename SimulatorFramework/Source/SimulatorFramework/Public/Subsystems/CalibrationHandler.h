// CalibrationHandler.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/MiniGameHandlerBase.h"
#include "Lib/Data/Core/InputConfigData.h"
#include "Lib/Data/SimulatorFramework/MiniGameData.h"
#include "CalibrationHandler.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCalibrationFeedback, float, CurrentValue, float, TargetValue, float, HoldProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCalibrationLocked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCalibrationLost);

/**
 * Handler for precision calibration mini-games
 * Hold input in shrinking zone, optionally track moving target
 */
UCLASS(BlueprintType, Blueprintable)
class SIMULATORFRAMEWORK_API UCalibrationHandler : public UMiniGameHandlerBase
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

    // === CALIBRATION STATE ===

    /** Get current input value (0-1) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Calibration")
    float GetCurrentValue() const { return CurrentValue; }

    /** Get target value (0-1) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Calibration")
    float GetTargetValue() const { return TargetValue; }

    /** Get current tolerance */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Calibration")
    float GetCurrentTolerance() const { return CurrentTolerance; }

    /** Check if currently locked on target */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Calibration")
    bool IsLockedOn() const { return bIsLockedOn; }

    /** Get hold progress (0-1) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Calibration")
    float GetHoldProgress() const;

    /** Get time held on target */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Calibration")
    float GetTimeHeld() const { return TimeHeldOnTarget; }

    /** Get required hold duration */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Calibration")
    float GetRequiredHoldDuration() const { return CalibrationConfig.RequiredHoldDuration; }

    /** Get distance from target (0 = perfect, 1 = at tolerance edge) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Calibration")
    float GetDistanceFromTarget() const;

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Calibration")
    FOnCalibrationFeedback OnFeedbackUpdated;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Calibration")
    FOnCalibrationLocked OnLocked;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Calibration")
    FOnCalibrationLost OnLost;

protected:
    // === CONFIG ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Calibration")
    FCalibrationConfig CalibrationConfig;

    // === STATE ===

    /** Current player input value (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Calibration")
    float CurrentValue = 0.5f;

    /** Current target value (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Calibration")
    float TargetValue = 0.5f;

    /** Current tolerance (shrinks over time) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Calibration")
    float CurrentTolerance = 0.1f;

    /** Is player locked on target */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Calibration")
    bool bIsLockedOn = false;

    /** Time held on target */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Calibration")
    float TimeHeldOnTarget = 0.0f;

    /** Target movement direction (for moving targets) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Calibration")
    float TargetDirection = 1.0f;

    /** Was locked last frame */
    bool bWasLockedOn = false;

    // === INTERNAL ===

    /** Update target position (if moving) */
    void UpdateTarget(float DeltaTime);

    /** Update tolerance (shrinking) */
    void UpdateTolerance(float DeltaTime);

    /** Check lock state */
    void UpdateLockState();

    /** Handle gaining lock */
    void GainLock();

    /** Handle losing lock */
    void LoseLock();

    /** Update hold progress */
    void UpdateHoldProgress(float DeltaTime);

    /** Report progress */
    void ReportCalibrationProgress();

    /** Generate initial target */
    float GenerateTarget() const;
};