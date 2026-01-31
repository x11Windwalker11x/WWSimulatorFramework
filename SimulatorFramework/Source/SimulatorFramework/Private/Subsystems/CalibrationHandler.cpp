// CalibrationHandler.cpp
#include "Subsystems/CalibrationHandler.h"
#include "Components/MiniGameComponent.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

void UCalibrationHandler::Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID)
{
    Super::Initialize(InOwner, InConfig, InObjectiveSetID);
    CalibrationConfig = InConfig.CalibrationConfig;
}

void UCalibrationHandler::Activate()
{
    Super::Activate();

    // Reset state
    CurrentValue = 0.5f;
    TargetValue = GenerateTarget();
    CurrentTolerance = CalibrationConfig.SweetspotSettings.ToleranceMax;
    bIsLockedOn = false;
    bWasLockedOn = false;
    TimeHeldOnTarget = 0.0f;
    TargetDirection = (FMath::RandBool()) ? 1.0f : -1.0f;
}

void UCalibrationHandler::Deactivate()
{
    Super::Deactivate();
}

void UCalibrationHandler::TickHandler(float DeltaTime)
{
    Super::TickHandler(DeltaTime);

    if (!bIsActive) return;

    // Update systems
    UpdateTarget(DeltaTime);
    UpdateTolerance(DeltaTime);
    UpdateLockState();
    UpdateHoldProgress(DeltaTime);

    // Broadcast feedback
    OnFeedbackUpdated.Broadcast(CurrentValue, TargetValue, GetHoldProgress());
}

void UCalibrationHandler::ProcessAxisInput(const FVector2D& AxisValue, float DeltaTime)
{
    // X or Y axis controls calibration value
    float InputDelta = AxisValue.Y * DeltaTime * 2.0f; // Sensitivity

    CurrentValue = FMath::Clamp(CurrentValue + InputDelta, 0.0f, 1.0f);
}

void UCalibrationHandler::UpdateTarget(float DeltaTime)
{
    if (!CalibrationConfig.bMovingTarget) return;

    // Move target
    float MoveDelta = CalibrationConfig.TargetMoveSpeed * TargetDirection * DeltaTime;
    TargetValue += MoveDelta;

    // Bounce at edges
    if (TargetValue >= 0.85f)
    {
        TargetValue = 0.85f;
        TargetDirection = -1.0f;
    }
    else if (TargetValue <= 0.15f)
    {
        TargetValue = 0.15f;
        TargetDirection = 1.0f;
    }
}

void UCalibrationHandler::UpdateTolerance(float DeltaTime)
{
    if (CalibrationConfig.ZoneShrinkRate <= 0.0f) return;

    // Shrink tolerance over time
    float ShrinkAmount = CalibrationConfig.ZoneShrinkRate * DeltaTime;
    CurrentTolerance -= ShrinkAmount;

    // Clamp to minimum
    CurrentTolerance = FMath::Max(CurrentTolerance, CalibrationConfig.SweetspotSettings.ToleranceMin);
}

void UCalibrationHandler::UpdateLockState()
{
    bWasLockedOn = bIsLockedOn;

    float Distance = FMath::Abs(CurrentValue - TargetValue);
    bIsLockedOn = (Distance <= CurrentTolerance);

    // Handle state transitions
    if (bIsLockedOn && !bWasLockedOn)
    {
        GainLock();
    }
    else if (!bIsLockedOn && bWasLockedOn)
    {
        LoseLock();
    }
}

void UCalibrationHandler::GainLock()
{
    OnLocked.Broadcast();
}

void UCalibrationHandler::LoseLock()
{
    // Reset hold progress on losing lock
    TimeHeldOnTarget = 0.0f;
    OnLost.Broadcast();
}

void UCalibrationHandler::UpdateHoldProgress(float DeltaTime)
{
    if (!bIsLockedOn) return;

    TimeHeldOnTarget += DeltaTime;
    ReportCalibrationProgress();

    // Check completion
    if (TimeHeldOnTarget >= CalibrationConfig.RequiredHoldDuration)
    {
        ReportObjectiveComplete(FWWTagLibrary::Simulator_MiniGame_Objective_CalibrationHeld());
        MarkComplete(true);
    }
}

float UCalibrationHandler::GetHoldProgress() const
{
    if (CalibrationConfig.RequiredHoldDuration <= 0.0f) return 0.0f;
    return FMath::Clamp(TimeHeldOnTarget / CalibrationConfig.RequiredHoldDuration, 0.0f, 1.0f);
}

float UCalibrationHandler::GetDistanceFromTarget() const
{
    if (CurrentTolerance <= 0.0f) return 1.0f;

    float Distance = FMath::Abs(CurrentValue - TargetValue);
    return FMath::Clamp(Distance / CurrentTolerance, 0.0f, 1.0f);
}

void UCalibrationHandler::ReportCalibrationProgress()
{
    ReportObjectiveValue(FWWTagLibrary::Simulator_MiniGame_Objective_CalibrationHeld(), GetHoldProgress());
}

float UCalibrationHandler::GenerateTarget() const
{
    // Random target with margin from edges
    return FMath::FRandRange(0.2f, 0.8f);
}