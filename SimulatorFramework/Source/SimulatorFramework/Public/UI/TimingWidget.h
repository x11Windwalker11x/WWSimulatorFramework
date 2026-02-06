// TimingWidget.h
// Timing bar widget for rhythm/QTE minigames

#pragma once

#include "CoreMinimal.h"
#include "UI/MiniGameHUD_Base.h"
#include "TimingWidget.generated.h"

class UProgressBar;
class UImage;
class UTextBlock;
class UBorder;
class UTimingHandler;

/**
 * UTimingWidget
 * Visual timing bar for rhythm-based minigames
 * Displays moving indicator, timing window, progress, and accuracy feedback
 */
UCLASS()
class SIMULATORFRAMEWORK_API UTimingWidget : public UMiniGameHUD_Base
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ============================================================================
    // MINIGAME HUD OVERRIDES
    // ============================================================================

    virtual void OnMiniGameStarted_Implementation(const FGameplayTag& MiniGameID, UMiniGameHandlerBase* Handler) override;
    virtual void OnMiniGameEnded_Implementation(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus) override;
    virtual void OnProgressUpdated_Implementation(const FGameplayTag& ObjectiveTag, float Progress) override;

    // ============================================================================
    // BOUND WIDGETS - TIMING BAR
    // ============================================================================

    /** Background bar representing the full cycle */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Bar_TimingBackground;

    /** Visual indicator for timing window zone */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UBorder> Border_Window;

    /** Moving indicator showing current position */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_Indicator;

    // ============================================================================
    // BOUND WIDGETS - DISPLAY
    // ============================================================================

    /** Progress text (e.g., "3/5 hits") */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Progress;

    /** Accuracy display for last hit */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Accuracy;

    /** Miss counter */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Misses;

    /** Visual feedback border */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UBorder> Border_Feedback;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    /** Color for the timing window */
    UPROPERTY(EditDefaultsOnly, Category = "Timing|Colors")
    FLinearColor WindowColor = FLinearColor(0.0f, 0.8f, 0.0f, 0.5f);

    /** Color for perfect hit feedback */
    UPROPERTY(EditDefaultsOnly, Category = "Timing|Colors")
    FLinearColor PerfectColor = FLinearColor(1.0f, 0.84f, 0.0f);

    /** Color for good hit feedback */
    UPROPERTY(EditDefaultsOnly, Category = "Timing|Colors")
    FLinearColor GoodColor = FLinearColor::Green;

    /** Color for miss feedback */
    UPROPERTY(EditDefaultsOnly, Category = "Timing|Colors")
    FLinearColor MissColor = FLinearColor::Red;

    /** Color for indicator when in window */
    UPROPERTY(EditDefaultsOnly, Category = "Timing|Colors")
    FLinearColor IndicatorInWindowColor = FLinearColor::Green;

    /** Color for indicator when outside window */
    UPROPERTY(EditDefaultsOnly, Category = "Timing|Colors")
    FLinearColor IndicatorOutsideColor = FLinearColor::White;

    /** Accuracy threshold for "perfect" (0-1, 1 = dead center) */
    UPROPERTY(EditDefaultsOnly, Category = "Timing|Feedback")
    float PerfectThreshold = 0.9f;

    /** Duration of feedback flash (seconds) */
    UPROPERTY(EditDefaultsOnly, Category = "Timing|Feedback")
    float FeedbackDuration = 0.3f;

    /** Bar width for positioning (pixels) */
    UPROPERTY(EditDefaultsOnly, Category = "Timing|Layout")
    float BarWidth = 400.0f;

    // ============================================================================
    // BLUEPRINT EVENTS
    // ============================================================================

    /** Called on timing hit - override for custom effects */
    UFUNCTION(BlueprintNativeEvent, Category = "Timing")
    void OnTimingHit(bool bSuccess, float Accuracy);

    /** Called when indicator enters timing window */
    UFUNCTION(BlueprintNativeEvent, Category = "Timing")
    void OnWindowEntered();

    /** Called when indicator exits timing window */
    UFUNCTION(BlueprintNativeEvent, Category = "Timing")
    void OnWindowExited();

private:
    /** Cached timing handler reference */
    TWeakObjectPtr<UTimingHandler> CachedTimingHandler;

    /** Previous in-window state for edge detection */
    bool bWasInWindow = false;

    /** Timer handle for feedback */
    FTimerHandle FeedbackTimerHandle;

    /** Update the indicator position based on cycle progress */
    void UpdateIndicatorPosition(float CyclePosition);

    /** Update the window highlight based on current window bounds */
    void UpdateWindowDisplay(float WindowStart, float WindowEnd);

    /** Show visual feedback for hit result */
    void ShowHitFeedback(bool bSuccess, float Accuracy);

    /** Clear feedback state */
    void ClearFeedback();

    /** Update progress display */
    void UpdateProgressDisplay(int32 Successes, int32 Required, int32 Misses, int32 MaxMisses);
};
