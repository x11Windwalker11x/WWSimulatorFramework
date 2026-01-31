// TemperatureHandler.cpp
#include "Subsystems/TemperatureHandler.h"
#include "Components/MiniGameComponent.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

void UTemperatureHandler::Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID)
{
    Super::Initialize(InOwner, InConfig, InObjectiveSetID);
    TempConfig = InConfig.TemperatureConfig;
}

void UTemperatureHandler::Activate()
{
    Super::Activate();

    // Reset state
    CurrentTemperature = GetStartingTemperature();
    CurrentHeatInput = 0.0f;
    bIsHeating = false;
    bInOptimalZone = false;
    bInDangerZone = false;
    bWasInOptimalZone = false;
    bWasInDangerZone = false;
    TimeInOptimalZone = 0.0f;
    bIsRuined = false;

    UpdateZoneStates();
}

void UTemperatureHandler::Deactivate()
{
    bIsHeating = false;
    Super::Deactivate();
}

void UTemperatureHandler::TickHandler(float DeltaTime)
{
    Super::TickHandler(DeltaTime);

    if (!bIsActive || bIsRuined) return;

    UpdateTemperature(DeltaTime);
    UpdateZoneStates();

    // Accumulate time in optimal zone
    if (bInOptimalZone)
    {
        TimeInOptimalZone += DeltaTime;
        ReportTemperatureProgress();

        // Check completion
        if (TimeInOptimalZone >= TempConfig.RequiredDurationInZone)
        {
            ReportObjectiveComplete(FWWTagLibrary::Simulator_MiniGame_Objective_TemperatureMaintained());
            MarkComplete(true);
            return;
        }
    }

    // Broadcast temperature change
    float Progress = (TempConfig.RequiredDurationInZone > 0.0f)
        ? TimeInOptimalZone / TempConfig.RequiredDurationInZone
        : 0.0f;
    OnTemperatureChanged.Broadcast(CurrentTemperature, Progress);
}

void UTemperatureHandler::ProcessAxisInput(const FVector2D& AxisValue, float DeltaTime)
{
    // Y axis controls heat input (analog control)
    CurrentHeatInput = FMath::Clamp(AxisValue.Y, 0.0f, 1.0f);
}

void UTemperatureHandler::ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed)
{
    // Primary action = heat toggle/hold
    if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Primary"))))
    {
        bIsHeating = bPressed;
        
        if (bIsHeating)
        {
            CurrentHeatInput = 1.0f; // Full heat when button held
        }
    }
    // Secondary action = cool (optional mechanic)
    else if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Secondary"))))
    {
        if (bPressed)
        {
            CurrentHeatInput = -0.5f; // Active cooling
        }
        else if (!bIsHeating)
        {
            CurrentHeatInput = 0.0f;
        }
    }
}

void UTemperatureHandler::UpdateTemperature(float DeltaTime)
{
    float TempDelta = 0.0f;

    if (CurrentHeatInput > 0.0f)
    {
        // Heating
        TempDelta = TempConfig.HeatRate * CurrentHeatInput * DeltaTime;
    }
    else if (CurrentHeatInput < 0.0f)
    {
        // Active cooling
        TempDelta = TempConfig.CoolRate * CurrentHeatInput * DeltaTime; // Negative
    }
    else
    {
        // Passive cooling (no input)
        TempDelta = -TempConfig.CoolRate * DeltaTime;
    }

    CurrentTemperature += TempDelta;

    // Clamp to reasonable bounds (can't go below absolute zero conceptually)
    CurrentTemperature = FMath::Max(CurrentTemperature, 0.0f);
}

void UTemperatureHandler::UpdateZoneStates()
{
    bWasInOptimalZone = bInOptimalZone;
    bWasInDangerZone = bInDangerZone;

    bInOptimalZone = TempConfig.IsInOptimalZone(CurrentTemperature);
    bInDangerZone = TempConfig.IsInDangerZone(CurrentTemperature);

    // Zone transitions
    if (bInOptimalZone && !bWasInOptimalZone)
    {
        EnterOptimalZone();
    }
    else if (!bInOptimalZone && bWasInOptimalZone)
    {
        LeaveOptimalZone();
    }

    if (bInDangerZone && !bWasInDangerZone)
    {
        EnterDangerZone();
    }
}

void UTemperatureHandler::EnterOptimalZone()
{
    OnZoneEntered.Broadcast(true);
}

void UTemperatureHandler::LeaveOptimalZone()
{
    // Optional: reset progress on leaving zone
    // TimeInOptimalZone = 0.0f; // Uncomment for strict mode
    
    OnZoneEntered.Broadcast(false);
}

void UTemperatureHandler::EnterDangerZone()
{
    // Determine if overheat or freeze
    if (TempConfig.bCanOverheat && CurrentTemperature > TempConfig.DangerMax)
    {
        RuinItem(TEXT("Overheated"));
    }
    else if (TempConfig.bCanFreeze && CurrentTemperature < TempConfig.DangerMin)
    {
        RuinItem(TEXT("Frozen"));
    }
}

void UTemperatureHandler::RuinItem(const FString& Reason)
{
    if (bIsRuined) return;

    bIsRuined = true;
    OnItemRuined.Broadcast(Reason);
    MarkFailed(Reason);
}

void UTemperatureHandler::ReportTemperatureProgress()
{
    if (TempConfig.RequiredDurationInZone > 0.0f)
    {
        float Progress = FMath::Clamp(TimeInOptimalZone / TempConfig.RequiredDurationInZone, 0.0f, 1.0f);
        ReportObjectiveValue(FWWTagLibrary::Simulator_MiniGame_Objective_TemperatureMaintained(), Progress);
    }
}

float UTemperatureHandler::GetStartingTemperature() const
{
    // Start at room temperature (below optimal, requires heating)
    return FMath::Min(TempConfig.OptimalMin - 50.0f, TempConfig.DangerMin + 10.0f);
}