// TimingHandler.cpp
#include "Subsystems/TimingHandler.h"
#include "Components/MiniGameComponent.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

void UTimingHandler::Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID)
{
    Super::Initialize(InOwner, InConfig, InObjectiveSetID);
    RhythmConfig = InConfig.RhythmConfig;
}

void UTimingHandler::Activate()
{
    Super::Activate();

    // Reset state
    CyclePosition = 0.0f;
    SuccessCount = 0;
    MissCount = 0;
    LastAccuracy = 0.0f;
    bWindowHit = false;
    bWasInWindow = false;

    // Initialize from config
    CurrentCycleDuration = RhythmConfig.TimingSettings.CycleDuration;
    CurrentWindowStart = RhythmConfig.TimingSettings.WindowStart;
    CurrentWindowEnd = RhythmConfig.TimingSettings.WindowEnd;
    bInWindow = false;
}

void UTimingHandler::Deactivate()
{
    Super::Deactivate();
}

void UTimingHandler::TickHandler(float DeltaTime)
{
    Super::TickHandler(DeltaTime);

    if (!bIsActive) return;

    AdvanceCycle(DeltaTime);

    // Broadcast cycle progress
    OnCycleProgress.Broadcast(CyclePosition);
}

void UTimingHandler::ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed)
{
    if (!bPressed) return;

    // Primary action = timing hit
    if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Primary"))))
    {
        ProcessTimingInput();
    }
}

void UTimingHandler::ProcessTimingInput()
{
    if (bWindowHit) return; // Already hit this window

    if (bInWindow)
    {
        float Accuracy = CalculateAccuracy();
        HandleSuccess(Accuracy);
    }
    else
    {
        HandleMiss();
    }
}

void UTimingHandler::HandleSuccess(float Accuracy)
{
    bWindowHit = true;
    SuccessCount++;
    LastAccuracy = Accuracy;

    OnTimingResult.Broadcast(true, Accuracy);
    ReportTimingProgress();

    // Check completion
    if (SuccessCount >= RhythmConfig.RequiredSuccessCount)
    {
        ReportObjectiveComplete(FWWTagLibrary::Simulator_MiniGame_Objective_TimingHit());
        MarkComplete(true);
        return;
    }

    // Apply difficulty increases
    if (RhythmConfig.bIncreasingTempo)
    {
        ApplyTempoIncrease();
    }

    ApplyWindowShrink();
}

void UTimingHandler::HandleMiss()
{
    MissCount++;
    LastAccuracy = 0.0f;

    OnTimingResult.Broadcast(false, 0.0f);

    // Check failure
    if (RhythmConfig.MaxMissesAllowed > 0 && MissCount >= RhythmConfig.MaxMissesAllowed)
    {
        MarkFailed(TEXT("Too many misses"));
    }
}

void UTimingHandler::HandleWindowMissed()
{
    // Window passed without player input
    if (!bWindowHit)
    {
        MissCount++;
        OnTimingResult.Broadcast(false, 0.0f);

        if (RhythmConfig.MaxMissesAllowed > 0 && MissCount >= RhythmConfig.MaxMissesAllowed)
        {
            MarkFailed(TEXT("Too many misses"));
        }
    }
}

void UTimingHandler::AdvanceCycle(float DeltaTime)
{
    bWasInWindow = bInWindow;

    // Advance position
    float CycleSpeed = 1.0f / CurrentCycleDuration;
    CyclePosition += DeltaTime * CycleSpeed;

    // Check window state
    bInWindow = RhythmConfig.TimingSettings.IsInWindow(CyclePosition);

    // Detect window exit (missed opportunity)
    if (bWasInWindow && !bInWindow)
    {
        HandleWindowMissed();
    }

    // Handle cycle wrap
    if (CyclePosition >= 1.0f)
    {
        if (RhythmConfig.TimingSettings.bLooping)
        {
            ResetCycle();
        }
        else
        {
            // Non-looping - check if we have enough successes
            if (SuccessCount < RhythmConfig.RequiredSuccessCount)
            {
                MarkFailed(TEXT("Cycle ended without enough successes"));
            }
        }
    }
}

void UTimingHandler::ApplyTempoIncrease()
{
    if (!RhythmConfig.bIncreasingTempo) return;

    float NewDuration = CurrentCycleDuration * (1.0f - RhythmConfig.TempoIncreaseRate);
    CurrentCycleDuration = FMath::Max(NewDuration, RhythmConfig.MinCycleDuration);
}

void UTimingHandler::ApplyWindowShrink()
{
    float ShrinkRate = RhythmConfig.TimingSettings.WindowShrinkRate;
    if (ShrinkRate <= 0.0f) return;

    float WindowCenter = (CurrentWindowStart + CurrentWindowEnd) * 0.5f;
    float CurrentSize = CurrentWindowEnd - CurrentWindowStart;
    float NewSize = FMath::Max(CurrentSize - ShrinkRate, 0.05f); // Minimum window size

    CurrentWindowStart = WindowCenter - (NewSize * 0.5f);
    CurrentWindowEnd = WindowCenter + (NewSize * 0.5f);

    // Clamp to valid range
    CurrentWindowStart = FMath::Max(CurrentWindowStart, 0.0f);
    CurrentWindowEnd = FMath::Min(CurrentWindowEnd, 1.0f);
}

float UTimingHandler::CalculateAccuracy() const
{
    if (!bInWindow) return 0.0f;

    float WindowCenter = (CurrentWindowStart + CurrentWindowEnd) * 0.5f;
    float HalfWindowSize = (CurrentWindowEnd - CurrentWindowStart) * 0.5f;

    if (HalfWindowSize <= 0.0f) return 1.0f;

    float DistanceFromCenter = FMath::Abs(CyclePosition - WindowCenter);
    float NormalizedDistance = DistanceFromCenter / HalfWindowSize;

    // 1.0 at center, 0.0 at edges
    return FMath::Clamp(1.0f - NormalizedDistance, 0.0f, 1.0f);
}

void UTimingHandler::ResetCycle()
{
    CyclePosition = 0.0f;
    bWindowHit = false;
    bInWindow = false;
    bWasInWindow = false;
}

void UTimingHandler::ReportTimingProgress()
{
    if (RhythmConfig.RequiredSuccessCount > 0)
    {
        float Progress = static_cast<float>(SuccessCount) / static_cast<float>(RhythmConfig.RequiredSuccessCount);
        ReportObjectiveValue(FWWTagLibrary::Simulator_MiniGame_Objective_TimingHit(), Progress);
    }
}