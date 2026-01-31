// SweetspotHandler.cpp
#include "Subsystems/SweetspotHandler.h"
#include "Components/MiniGameComponent.h"
#include "Interfaces/SimulatorFramework/MiniGameStationInterface.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

void USweetspotHandler::Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID)
{
    Super::Initialize(InOwner, InConfig, InObjectiveSetID);
    LockpickConfig = InConfig.LockpickConfig;
}

void USweetspotHandler::Activate()
{
    Super::Activate();

    // Reset state
    CurrentPosition = 0.5f;
    bInSweetspot = false;
    bIsTensioned = false;
    TensionDamage = 0.0f;
    CurrentPinIndex = 0;
    UnlockedPins = 0;
    FeedbackIntensity = 0.0f;

    // Set attempts based on config
    AttemptsRemaining = (LockpickConfig.SweetspotSettings.MaxAttempts > 0)
        ? LockpickConfig.SweetspotSettings.MaxAttempts
        : 99; // Essentially unlimited

    // Generate sweetspots for all pins
    PinSweetspots.Empty();
    for (int32 i = 0; i < LockpickConfig.PinCount; i++)
    {
        PinSweetspots.Add(GenerateSweetspot());
    }

    // Set initial sweetspot
    if (PinSweetspots.Num() > 0)
    {
        SweetspotTarget = PinSweetspots[0];
    }

    // Calculate tolerance based on difficulty
    float Difficulty = GetStationDifficulty();
    CurrentTolerance = LockpickConfig.GetAdjustedTolerance(Difficulty);

    UpdateFeedback();
}

void USweetspotHandler::Deactivate()
{
    bIsTensioned = false;
    Super::Deactivate();
}

void USweetspotHandler::TickHandler(float DeltaTime)
{
    Super::TickHandler(DeltaTime);

    if (!bIsActive) return;

    // Apply tension damage if tensioned in wrong position
    if (bIsTensioned)
    {
        ApplyTension(DeltaTime);
    }

    UpdateFeedback();
}

void USweetspotHandler::ProcessAxisInput(const FVector2D& AxisValue, float DeltaTime)
{
    if (!bIsActive) return;

    // X axis moves pick position
    float MoveDelta = AxisValue.X * DeltaTime * 2.0f; // Sensitivity

    CurrentPosition = FMath::Clamp(CurrentPosition + MoveDelta, 0.0f, 1.0f);

    UpdateFeedback();
}

void USweetspotHandler::ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed)
{
    // Primary action = apply/release tension
    if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Primary"))))
    {
        if (bPressed)
        {
            bIsTensioned = true;
            TensionDamage = 0.0f;
        }
        else
        {
            ReleaseTension();
        }
    }
}

float USweetspotHandler::GenerateSweetspot() const
{
    // Random position with some margin from edges
    return FMath::FRandRange(0.15f, 0.85f);
}

float USweetspotHandler::CalculateFeedback(float Position) const
{
    float Distance = FMath::Abs(Position - SweetspotTarget);

    // Normalize distance relative to tolerance
    float NormalizedDist = Distance / CurrentTolerance;

    if (NormalizedDist <= 1.0f)
    {
        // In sweetspot zone - max feedback
        return 1.0f;
    }
    else
    {
        // Outside - gradual falloff
        float Falloff = FMath::Clamp(1.0f - ((NormalizedDist - 1.0f) * 0.5f), 0.0f, 1.0f);
        return FMath::Pow(Falloff, LockpickConfig.SweetspotSettings.FeedbackCurveExponent);
    }
}

void USweetspotHandler::ApplyTension(float DeltaTime)
{
    if (bInSweetspot)
    {
        // In sweetspot - unlock pin
        UnlockCurrentPin();
    }
    else
    {
        // Wrong position - accumulate damage
        TensionDamage += DeltaTime;

        // Break threshold based on how far off
        float Distance = FMath::Abs(CurrentPosition - SweetspotTarget);
        float BreakThreshold = FMath::Lerp(2.0f, 0.3f, FMath::Clamp(Distance / 0.5f, 0.0f, 1.0f));

        if (TensionDamage >= BreakThreshold)
        {
            BreakPick();
        }
    }
}

void USweetspotHandler::ReleaseTension()
{
    bIsTensioned = false;
    TensionDamage = 0.0f;
}

void USweetspotHandler::UnlockCurrentPin()
{
    bIsTensioned = false;
    UnlockedPins++;

    OnPinUnlocked.Broadcast(CurrentPinIndex);

    // Check if all pins unlocked
    if (UnlockedPins >= LockpickConfig.PinCount)
    {
        ReportObjectiveComplete(FWWTagLibrary::Simulator_MiniGame_Objective_LockOpened());
        MarkComplete(true);
    }
    else
    {
        AdvanceToNextPin();
    }
}

void USweetspotHandler::BreakPick()
{
    bIsTensioned = false;
    TensionDamage = 0.0f;

    if (LockpickConfig.SweetspotSettings.bFailureBreaksItem)
    {
        AttemptsRemaining--;
        OnPickBroken.Broadcast(AttemptsRemaining);

        if (AttemptsRemaining <= 0)
        {
            MarkFailed(TEXT("No picks remaining"));
            return;
        }
    }

    // Randomize sweetspot on failure if configured
    if (LockpickConfig.bRandomizeSweetspotOnFail)
    {
        ResetCurrentPin();
    }
}

void USweetspotHandler::AdvanceToNextPin()
{
    CurrentPinIndex++;

    if (CurrentPinIndex < PinSweetspots.Num())
    {
        SweetspotTarget = PinSweetspots[CurrentPinIndex];
        CurrentPosition = 0.5f; // Reset position
        UpdateFeedback();
    }
}

void USweetspotHandler::ResetCurrentPin()
{
    // Generate new sweetspot for current pin
    float NewSweetspot = GenerateSweetspot();
    
    if (PinSweetspots.IsValidIndex(CurrentPinIndex))
    {
        PinSweetspots[CurrentPinIndex] = NewSweetspot;
        SweetspotTarget = NewSweetspot;
    }

    CurrentPosition = 0.5f;
    UpdateFeedback();
}

void USweetspotHandler::UpdateFeedback()
{
    float NewIntensity = CalculateFeedback(CurrentPosition);
    bool bWasInSweetspot = bInSweetspot;

    FeedbackIntensity = NewIntensity;
    bInSweetspot = (FMath::Abs(CurrentPosition - SweetspotTarget) <= CurrentTolerance);

    // Broadcast if changed significantly
    if (FMath::Abs(NewIntensity - FeedbackIntensity) > 0.01f || bInSweetspot != bWasInSweetspot)
    {
        OnFeedbackChanged.Broadcast(FeedbackIntensity, bInSweetspot);
    }

    // Report progress
    float Progress = static_cast<float>(UnlockedPins) / static_cast<float>(LockpickConfig.PinCount);
    ReportObjectiveValue(FWWTagLibrary::Simulator_MiniGame_Objective_LockOpened(), Progress);
}

float USweetspotHandler::GetStationDifficulty() const
{
    if (UMiniGameComponent* Owner = OwnerComponent.Get())
    {
        if (AActor* Station = Owner->GetCurrentStation())
        {
            if (Station->GetClass()->ImplementsInterface(UMiniGameStationInterface::StaticClass()))
            {
                return IMiniGameStationInterface::Execute_GetDifficultyModifier(Station);
            }
        }
    }
    return 0.5f; // Default medium difficulty
}