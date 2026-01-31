// VaultNumpadStation.cpp
#include "Actors/VaultNumpadStation.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "TimerManager.h"

AVaultNumpadStation::AVaultNumpadStation()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default code: 1-2-3-4
    VaultCode.Add(FWWTagLibrary::Input_Numpad_1());
    VaultCode.Add(FWWTagLibrary::Input_Numpad_2());
    VaultCode.Add(FWWTagLibrary::Input_Numpad_3());
    VaultCode.Add(FWWTagLibrary::Input_Numpad_4());
}

void AVaultNumpadStation::BeginPlay()
{
    Super::BeginPlay();

    RemainingAttempts = MaxAttempts;

    // Set default tags if not configured
    if (!MiniGameID.IsValid())
    {
        MiniGameID = FGameplayTag::RequestGameplayTag(FName("Simulator.MiniGame.ID.Sequence.VaultNumpad"));
    }
    if (!CameraModeTag.IsValid())
    {
        CameraModeTag = FWWTagLibrary::Camera_Mode_Station_Numpad();
    }
}

// === IMiniGameStationInterface ===

FGameplayTag AVaultNumpadStation::GetMiniGameID_Implementation() const
{
    return MiniGameID;
}

bool AVaultNumpadStation::GetMiniGameConfig_Implementation(FMiniGameData& OutConfig) const
{
    // Build config inline (or could lookup from DataTable)
    OutConfig.MiniGameID = MiniGameID;
    OutConfig.TypeTag = FWWTagLibrary::Simulator_MiniGame_Type_Sequence();
    OutConfig.CameraModeTag = CameraModeTag;
    OutConfig.bCanCancel = true;
    OutConfig.TimeoutSeconds = 0.0f;

    // Sequence config
    OutConfig.SequenceConfig.ExpectedSequence = VaultCode;
    OutConfig.SequenceConfig.InputTimeout = 10.0f;
    OutConfig.SequenceConfig.bOrderMatters = true;
    OutConfig.SequenceConfig.bAllowBackspace = true;
    OutConfig.SequenceConfig.MaxErrors = 0; // Fail on first wrong digit

    // Objectives
    FObjectiveEntry CodeEntry;
    CodeEntry.Condition.ObjectiveTag = FWWTagLibrary::Simulator_MiniGame_Objective_CodeEntered();
    CodeEntry.Condition.Operator = ECompareOp::GreaterEqual;
    CodeEntry.Condition.TargetValue = 1.0f;
    CodeEntry.bIsMandatory = true;
    OutConfig.Objectives.Entries.Add(CodeEntry);

    return true;
}

void AVaultNumpadStation::OnMiniGameStarted_Implementation(const FGameplayTag& InMiniGameID)
{
    // Could play animation, sound, etc.
    UE_LOG(LogTemp, Log, TEXT("VaultNumpad: MiniGame started"));
}

void AVaultNumpadStation::OnMiniGameEnded_Implementation(const FGameplayTag& InMiniGameID, bool bSuccess, bool bBonus)
{
    if (bSuccess)
    {
        OpenVault();
    }
    else
    {
        // Failed attempt
        if (MaxAttempts > 0)
        {
            RemainingAttempts--;

            if (RemainingAttempts <= 0)
            {
                TriggerLockout();
            }
        }
    }
}

TArray<FSnapPointData> AVaultNumpadStation::GetSnapPoints_Implementation() const
{
    // Numpad doesn't use snap points
    return TArray<FSnapPointData>();
}

TArray<FGameplayTag> AVaultNumpadStation::GetStoredCode_Implementation() const
{
    return VaultCode;
}

float AVaultNumpadStation::GetDifficultyModifier_Implementation() const
{
    return Difficulty;
}

bool AVaultNumpadStation::IsMiniGameAvailable_Implementation() const
{
    return !bIsOpen && !bIsLockedOut;
}

// === VAULT STATE ===

void AVaultNumpadStation::SetVaultCode(const TArray<FGameplayTag>& NewCode)
{
    VaultCode = NewCode;
}

void AVaultNumpadStation::LockVault()
{
    if (!bIsOpen) return;

    bIsOpen = false;
    RemainingAttempts = MaxAttempts;
    OnVaultStateChanged.Broadcast(false);
}

void AVaultNumpadStation::OpenVault()
{
    if (bIsOpen) return;

    bIsOpen = true;
    OnVaultStateChanged.Broadcast(true);

    UE_LOG(LogTemp, Log, TEXT("VaultNumpad: Vault opened!"));
}

void AVaultNumpadStation::TriggerLockout()
{
    bIsLockedOut = true;

    UE_LOG(LogTemp, Warning, TEXT("VaultNumpad: Lockout triggered!"));

    if (LockoutDuration > 0.0f)
    {
        LockoutTimeRemaining = LockoutDuration;
        GetWorldTimerManager().SetTimer(
            LockoutTimerHandle,
            this,
            &AVaultNumpadStation::ClearLockout,
            LockoutDuration,
            false
        );
    }
}

void AVaultNumpadStation::ClearLockout()
{
    bIsLockedOut = false;
    RemainingAttempts = MaxAttempts;
    LockoutTimeRemaining = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("VaultNumpad: Lockout cleared"));
}