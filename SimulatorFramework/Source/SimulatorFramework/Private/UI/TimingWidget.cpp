// TimingWidget.cpp
// Timing bar widget for rhythm/QTE minigames

#include "UI/TimingWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Subsystems/TimingHandler.h"
#include "TimerManager.h"

void UTimingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize displays
    if (Text_Accuracy)
    {
        Text_Accuracy->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UTimingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!CachedTimingHandler.IsValid())
    {
        return;
    }

    // Update indicator position every frame
    float CyclePosition = CachedTimingHandler->GetCyclePosition();
    UpdateIndicatorPosition(CyclePosition);

    // Update window display
    float WindowStart = CachedTimingHandler->GetWindowStart();
    float WindowEnd = CachedTimingHandler->GetWindowEnd();
    UpdateWindowDisplay(WindowStart, WindowEnd);

    // Check for window enter/exit
    bool bInWindow = CachedTimingHandler->IsInWindow();
    if (bInWindow && !bWasInWindow)
    {
        OnWindowEntered();
    }
    else if (!bInWindow && bWasInWindow)
    {
        OnWindowExited();
    }
    bWasInWindow = bInWindow;

    // Update indicator color based on window state
    if (Image_Indicator)
    {
        Image_Indicator->SetColorAndOpacity(bInWindow ? IndicatorInWindowColor : IndicatorOutsideColor);
    }
}

// ============================================================================
// MINIGAME HUD OVERRIDES
// ============================================================================

void UTimingWidget::OnMiniGameStarted_Implementation()
{
    Super::OnMiniGameStarted_Implementation();

    // Cache timing handler
    if (CachedHandler.IsValid())
    {
        CachedTimingHandler = Cast<UTimingHandler>(CachedHandler.Get());
    }

    // Reset state
    bWasInWindow = false;

    if (Border_Feedback)
    {
        Border_Feedback->SetBrushColor(FLinearColor::Transparent);
    }

    if (Text_Accuracy)
    {
        Text_Accuracy->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Initial progress display
    if (CachedTimingHandler.IsValid())
    {
        UpdateProgressDisplay(
            CachedTimingHandler->GetSuccessCount(),
            CachedTimingHandler->GetRequiredSuccessCount(),
            CachedTimingHandler->GetMissCount(),
            CachedTimingHandler->GetMaxMissesAllowed()
        );
    }
}

void UTimingWidget::OnMiniGameEnded_Implementation(bool bSuccess)
{
    Super::OnMiniGameEnded_Implementation(bSuccess);

    // Final feedback
    if (Border_Feedback)
    {
        Border_Feedback->SetBrushColor(bSuccess ? GoodColor : MissColor);
    }

    // Clear timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(FeedbackTimerHandle);
    }
}

void UTimingWidget::OnProgressUpdated_Implementation(float Progress, int32 CurrentStep, int32 TotalSteps)
{
    Super::OnProgressUpdated_Implementation(Progress, CurrentStep, TotalSteps);

    if (!CachedTimingHandler.IsValid())
    {
        return;
    }

    // Get hit result from handler
    float LastAccuracy = CachedTimingHandler->GetLastAccuracy();
    bool bLastWasSuccess = LastAccuracy > 0.0f;

    // Show feedback
    ShowHitFeedback(bLastWasSuccess, LastAccuracy);
    OnTimingHit(bLastWasSuccess, LastAccuracy);

    // Update progress
    UpdateProgressDisplay(
        CachedTimingHandler->GetSuccessCount(),
        CachedTimingHandler->GetRequiredSuccessCount(),
        CachedTimingHandler->GetMissCount(),
        CachedTimingHandler->GetMaxMissesAllowed()
    );
}

// ============================================================================
// DISPLAY UPDATES
// ============================================================================

void UTimingWidget::UpdateIndicatorPosition(float CyclePosition)
{
    if (!Image_Indicator)
    {
        return;
    }

    // Calculate position along bar (0 = left, 1 = right)
    float XPosition = CyclePosition * BarWidth;

    // Set render translation
    Image_Indicator->SetRenderTranslation(FVector2D(XPosition, 0.0f));
}

void UTimingWidget::UpdateWindowDisplay(float WindowStart, float WindowEnd)
{
    if (!Border_Window)
    {
        return;
    }

    // Calculate window position and size
    float StartX = WindowStart * BarWidth;
    float WindowWidth = (WindowEnd - WindowStart) * BarWidth;

    // Update window border position and size
    Border_Window->SetRenderTranslation(FVector2D(StartX, 0.0f));

    // Note: For proper sizing, the UMG widget should use a SizeBox parent
    // or override via Blueprint with proper layout
}

void UTimingWidget::ShowHitFeedback(bool bSuccess, float Accuracy)
{
    if (!GetWorld())
    {
        return;
    }

    // Determine color based on result
    FLinearColor FeedbackColor = MissColor;
    if (bSuccess)
    {
        FeedbackColor = (Accuracy >= PerfectThreshold) ? PerfectColor : GoodColor;
    }

    // Apply to feedback border
    if (Border_Feedback)
    {
        Border_Feedback->SetBrushColor(FeedbackColor);
    }

    // Show accuracy text
    if (Text_Accuracy)
    {
        if (bSuccess)
        {
            FString AccuracyStr;
            if (Accuracy >= PerfectThreshold)
            {
                AccuracyStr = TEXT("PERFECT!");
            }
            else if (Accuracy >= 0.7f)
            {
                AccuracyStr = TEXT("Great!");
            }
            else
            {
                AccuracyStr = TEXT("Good");
            }
            Text_Accuracy->SetText(FText::FromString(AccuracyStr));
            Text_Accuracy->SetColorAndOpacity(FSlateColor(FeedbackColor));
        }
        else
        {
            Text_Accuracy->SetText(FText::FromString(TEXT("Miss!")));
            Text_Accuracy->SetColorAndOpacity(FSlateColor(MissColor));
        }
        Text_Accuracy->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    // Clear after duration
    GetWorld()->GetTimerManager().ClearTimer(FeedbackTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(
        FeedbackTimerHandle,
        this,
        &UTimingWidget::ClearFeedback,
        FeedbackDuration,
        false
    );
}

void UTimingWidget::ClearFeedback()
{
    if (Border_Feedback)
    {
        Border_Feedback->SetBrushColor(FLinearColor::Transparent);
    }

    if (Text_Accuracy)
    {
        Text_Accuracy->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UTimingWidget::UpdateProgressDisplay(int32 Successes, int32 Required, int32 Misses, int32 MaxMisses)
{
    if (Text_Progress)
    {
        Text_Progress->SetText(FText::FromString(
            FString::Printf(TEXT("%d / %d"), Successes, Required)
        ));
    }

    if (Text_Misses)
    {
        if (MaxMisses > 0)
        {
            Text_Misses->SetText(FText::FromString(
                FString::Printf(TEXT("Misses: %d / %d"), Misses, MaxMisses)
            ));

            // Warning color if close to failure
            float MissRatio = static_cast<float>(Misses) / static_cast<float>(MaxMisses);
            if (MissRatio >= 0.75f)
            {
                Text_Misses->SetColorAndOpacity(FSlateColor(MissColor));
            }
            else
            {
                Text_Misses->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            }
        }
        else
        {
            Text_Misses->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

// ============================================================================
// BLUEPRINT EVENTS
// ============================================================================

void UTimingWidget::OnTimingHit_Implementation(bool bSuccess, float Accuracy)
{
    // Default: no-op, override for custom effects (sounds, animations)
}

void UTimingWidget::OnWindowEntered_Implementation()
{
    // Default: no-op, override for "now!" prompts
}

void UTimingWidget::OnWindowExited_Implementation()
{
    // Default: no-op
}
