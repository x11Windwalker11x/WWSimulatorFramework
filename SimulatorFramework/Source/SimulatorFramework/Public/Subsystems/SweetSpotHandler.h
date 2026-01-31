// SweetspotHandler.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/MiniGameHandlerBase.h"
#include "Lib/Data/Core/InputConfigData.h"
#include "Lib/Data/SimulatorFramework/MiniGameData.h"
#include "SweetspotHandler.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSweetspotFeedback, float, Intensity, bool, bInZone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPinUnlocked, int32, PinIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPickBroken, int32, AttemptsRemaining);

/**
 * Handler for sweetspot-finding mini-games
 * Lockpicking (Skyrim/Fallout style), dial codes, calibration
 */
UCLASS(BlueprintType, Blueprintable)
class SIMULATORFRAMEWORK_API USweetspotHandler : public UMiniGameHandlerBase
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

    // === SWEETSPOT STATE ===

    /** Get current position (0-1 normalized) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sweetspot")
    float GetCurrentPosition() const { return CurrentPosition; }

    /** Get current feedback intensity (0-1) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sweetspot")
    float GetFeedbackIntensity() const { return FeedbackIntensity; }

    /** Check if currently in sweetspot zone */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sweetspot")
    bool IsInSweetspot() const { return bInSweetspot; }

    /** Get current pin index (for multi-pin locks) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sweetspot")
    int32 GetCurrentPinIndex() const { return CurrentPinIndex; }

    /** Get total pin count */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sweetspot")
    int32 GetTotalPins() const { return LockpickConfig.PinCount; }

    /** Get attempts remaining */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sweetspot")
    int32 GetAttemptsRemaining() const { return AttemptsRemaining; }

    /** Check if pick is currently tensioned */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sweetspot")
    bool IsTensioned() const { return bIsTensioned; }

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Sweetspot")
    FOnSweetspotFeedback OnFeedbackChanged;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Sweetspot")
    FOnPinUnlocked OnPinUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Sweetspot")
    FOnPickBroken OnPickBroken;

protected:
    // === CONFIG ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    FLockpickConfig LockpickConfig;

    // === STATE ===

    /** Current position on dial/pick (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    float CurrentPosition = 0.5f;

    /** Current sweetspot target (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    float SweetspotTarget = 0.5f;

    /** Current tolerance (affected by difficulty) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    float CurrentTolerance = 0.1f;

    /** Current feedback intensity */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    float FeedbackIntensity = 0.0f;

    /** Is position in sweetspot */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    bool bInSweetspot = false;

    /** Is player applying tension (trying to turn) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    bool bIsTensioned = false;

    /** Time spent tensioned in wrong position */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    float TensionDamage = 0.0f;

    /** Current pin being worked on */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    int32 CurrentPinIndex = 0;

    /** Pins successfully unlocked */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    int32 UnlockedPins = 0;

    /** Remaining attempts (picks) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    int32 AttemptsRemaining = 0;

    /** Per-pin sweetspot positions */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sweetspot")
    TArray<float> PinSweetspots;

    // === INTERNAL ===

    /** Generate random sweetspot position */
    float GenerateSweetspot() const;

    /** Calculate feedback intensity based on distance to sweetspot */
    float CalculateFeedback(float Position) const;

    /** Apply tension (attempt to unlock) */
    void ApplyTension(float DeltaTime);

    /** Release tension */
    void ReleaseTension();

    /** Handle successful pin unlock */
    void UnlockCurrentPin();

    /** Handle pick break */
    void BreakPick();

    /** Advance to next pin */
    void AdvanceToNextPin();

    /** Reset current pin (after break) */
    void ResetCurrentPin();

    /** Update feedback values */
    void UpdateFeedback();

    /** Get difficulty from station (0-1, 0=hardest) */
    float GetStationDifficulty() const;
};