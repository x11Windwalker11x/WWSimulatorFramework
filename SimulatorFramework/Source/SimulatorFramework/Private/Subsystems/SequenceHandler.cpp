// SequenceHandler.cpp
#include "Subsystems/SequenceHandler.h"
#include "Components/MiniGameComponent.h"
#include "Interfaces/SimulatorFramework/MiniGameStationInterface.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

void USequenceHandler::Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID)
{
    Super::Initialize(InOwner, InConfig, InObjectiveSetID);
    SequenceConfig = InConfig.SequenceConfig;
}

void USequenceHandler::Activate()
{
    Super::Activate();

    // Reset state
    CurrentSequence.Empty();
    ErrorCount = 0;
    TimeSinceLastInput = 0.0f;
    bLastInputCorrect = true;

    // Load expected sequence
    LoadExpectedSequence();
}

void USequenceHandler::Deactivate()
{
    CurrentSequence.Empty();
    Super::Deactivate();
}

void USequenceHandler::TickHandler(float DeltaTime)
{
    Super::TickHandler(DeltaTime);

    if (!bIsActive || CurrentSequence.Num() == 0) return;

    // Check timeout
    if (SequenceConfig.InputTimeout > 0.0f)
    {
        TimeSinceLastInput += DeltaTime;
        if (TimeSinceLastInput >= SequenceConfig.InputTimeout)
        {
            HandleTimeout();
        }
    }
}

void USequenceHandler::ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed)
{
    if (!bPressed) return; // Only process press, not release

    // Check if this is a sequence input tag (Input.Numpad.*, Input.QTE.*)
    if (ActionTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Input.Numpad"))) ||
        ActionTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Input.QTE"))))
    {
        ProcessSequenceInput(ActionTag);
    }
    // Handle backspace/clear
    else if (ActionTag.MatchesTagExact(FWWTagLibrary::Input_Numpad_Clear()))
    {
        ResetSequence();
    }
    // Handle enter/confirm
    else if (ActionTag.MatchesTagExact(FWWTagLibrary::Input_Numpad_Enter()))
    {
        if (CheckSequenceMatch())
        {
            ReportObjectiveComplete(FWWTagLibrary::Simulator_MiniGame_Objective_CodeEntered());
            MarkComplete(true);
        }
        else
        {
            ErrorCount++;
            bLastInputCorrect = false;
            
            if (SequenceConfig.MaxErrors > 0 && ErrorCount > SequenceConfig.MaxErrors)
            {
                MarkFailed(TEXT("Too many errors"));
            }
            else
            {
                ResetSequence();
            }
        }
    }
}

void USequenceHandler::ProcessSequenceInput(const FGameplayTag& InputTag)
{
    TimeSinceLastInput = 0.0f;

    // Handle backspace
    if (SequenceConfig.bAllowBackspace && 
        InputTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Backspace"))))
    {
        if (CurrentSequence.Num() > 0)
        {
            CurrentSequence.Pop();
            ReportSequenceProgress();
        }
        return;
    }

    // Add input to sequence
    CurrentSequence.Add(InputTag);

    // Check validity
    if (SequenceConfig.bOrderMatters)
    {
        // For ordered sequences, check if partial is still valid
        if (!IsPartialSequenceValid())
        {
            bLastInputCorrect = false;
            ErrorCount++;

            if (SequenceConfig.MaxErrors > 0 && ErrorCount > SequenceConfig.MaxErrors)
            {
                MarkFailed(TEXT("Too many errors"));
                return;
            }

            // Reset on error
            ResetSequence();
            return;
        }
        bLastInputCorrect = true;
    }

    ReportSequenceProgress();

    // Check for completion
    if (CurrentSequence.Num() == ExpectedSequence.Num())
    {
        if (CheckSequenceMatch())
        {
            ReportObjectiveComplete(FWWTagLibrary::Simulator_MiniGame_Objective_CodeEntered());
            MarkComplete(true);
        }
        else if (!SequenceConfig.bOrderMatters)
        {
            // Unordered but wrong combination
            ErrorCount++;
            bLastInputCorrect = false;

            if (SequenceConfig.MaxErrors > 0 && ErrorCount > SequenceConfig.MaxErrors)
            {
                MarkFailed(TEXT("Too many errors"));
            }
            else
            {
                ResetSequence();
            }
        }
    }
}

bool USequenceHandler::CheckSequenceMatch() const
{
    if (CurrentSequence.Num() != ExpectedSequence.Num())
    {
        return false;
    }

    if (SequenceConfig.bOrderMatters)
    {
        // Exact match required
        for (int32 i = 0; i < CurrentSequence.Num(); i++)
        {
            if (CurrentSequence[i] != ExpectedSequence[i])
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        // Unordered - check all expected are present
        for (const FGameplayTag& Expected : ExpectedSequence)
        {
            if (!CurrentSequence.Contains(Expected))
            {
                return false;
            }
        }
        return true;
    }
}

bool USequenceHandler::IsPartialSequenceValid() const
{
    if (!SequenceConfig.bOrderMatters)
    {
        return true; // Can't validate partial unordered
    }

    // Check each input matches expected at same position
    for (int32 i = 0; i < CurrentSequence.Num(); i++)
    {
        if (i >= ExpectedSequence.Num())
        {
            return false; // Too many inputs
        }
        if (CurrentSequence[i] != ExpectedSequence[i])
        {
            return false;
        }
    }
    return true;
}

void USequenceHandler::ResetSequence()
{
    CurrentSequence.Empty();
    TimeSinceLastInput = 0.0f;
    ReportSequenceProgress();
}

void USequenceHandler::HandleTimeout()
{
    // Timeout = reset, not fail (unless configured otherwise)
    ResetSequence();
}

void USequenceHandler::LoadExpectedSequence()
{
    // Try station first
    if (UMiniGameComponent* Owner = OwnerComponent.Get())
    {
        if (AActor* Station = Owner->GetCurrentStation())
        {
            if (Station->GetClass()->ImplementsInterface(UMiniGameStationInterface::StaticClass()))
            {
                TArray<FGameplayTag> StationCode = IMiniGameStationInterface::Execute_GetStoredCode(Station);
                if (StationCode.Num() > 0)
                {
                    ExpectedSequence = StationCode;
                    return;
                }
            }
        }
    }

    // Fallback to config
    ExpectedSequence = SequenceConfig.ExpectedSequence;
}

void USequenceHandler::ReportSequenceProgress()
{
    if (ExpectedSequence.Num() > 0)
    {
        float Progress = static_cast<float>(CurrentSequence.Num()) / static_cast<float>(ExpectedSequence.Num());
        ReportObjectiveValue(FWWTagLibrary::Simulator_MiniGame_Objective_SequenceComplete(), Progress);
    }
}