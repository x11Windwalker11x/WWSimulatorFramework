// TimingHandler.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/MiniGameHandlerBase.h"
#include "Lib/Data/Core/InputConfigData.h"
#include "Lib/Data/SimulatorFramework/MiniGameData.h"
#include "TimingHandler.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimingResult, bool, bSuccess, float, Accuracy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCycleProgress, float, NormalizedPosition);

/**
 * Handler for timing-based mini-games
 * Rhythm hits, tumbler clicks, QTE timing windows
 */
UCLASS(BlueprintType, Blueprintable)
class SIMULATORFRAMEWORK_API UTimingHandler : public UMiniGameHandlerBase
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

    virtual void ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed) override;

    // === TIMING STATE ===

    /** Get current cycle position (0-1) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    float GetCyclePosition() const { return CyclePosition; }

    /** Get current cycle duration */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    float GetCurrentCycleDuration() const { return CurrentCycleDuration; }

    /** Check if currently in timing window */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    bool IsInWindow() const { return bInWindow; }

    /** Get window start position */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    float GetWindowStart() const { return CurrentWindowStart; }

    /** Get window end position */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    float GetWindowEnd() const { return CurrentWindowEnd; }

    /** Get successful hits count */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    int32 GetSuccessCount() const { return SuccessCount; }

    /** Get miss count */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    int32 GetMissCount() const { return MissCount; }

    /** Get required success count */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    int32 GetRequiredSuccessCount() const { return RhythmConfig.RequiredSuccessCount; }

    /** Get last hit accuracy (0-1, 1 = perfect center) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    float GetLastAccuracy() const { return LastAccuracy; }

    /** Get max misses allowed (0 = unlimited) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Timing")
    int32 GetMaxMissesAllowed() const { return RhythmConfig.MaxMissesAllowed; }

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Timing")
    FOnTimingResult OnTimingResult;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Timing")
    FOnCycleProgress OnCycleProgress;

protected:
    // === CONFIG ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    FRhythmConfig RhythmConfig;

    // === STATE ===

    /** Current position in cycle (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    float CyclePosition = 0.0f;

    /** Current cycle duration (may change with tempo) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    float CurrentCycleDuration = 1.0f;

    /** Current window start (may shrink) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    float CurrentWindowStart = 0.4f;

    /** Current window end (may shrink) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    float CurrentWindowEnd = 0.6f;

    /** Is cycle position in window */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    bool bInWindow = false;

    /** Has player already hit this window */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    bool bWindowHit = false;

    /** Successful hit count */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    int32 SuccessCount = 0;

    /** Miss count */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    int32 MissCount = 0;

    /** Last hit accuracy */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Timing")
    float LastAccuracy = 0.0f;

    /** Was in window last frame (for exit detection) */
    bool bWasInWindow = false;

    // === INTERNAL ===

    /** Process a timing input */
    void ProcessTimingInput();

    /** Handle successful hit */
    void HandleSuccess(float Accuracy);

    /** Handle miss (hit outside window) */
    void HandleMiss();

    /** Handle window passed without input */
    void HandleWindowMissed();

    /** Advance cycle position */
    void AdvanceCycle(float DeltaTime);

    /** Apply tempo increase after success */
    void ApplyTempoIncrease();

    /** Apply window shrink after success */
    void ApplyWindowShrink();

    /** Calculate accuracy based on position in window */
    float CalculateAccuracy() const;

    /** Reset for next cycle */
    void ResetCycle();

    /** Report progress */
    void ReportTimingProgress();
};