// SequenceHandler.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/MiniGameHandlerBase.h"
#include "Lib/Data/Core/InputConfigData.h"
#include "SequenceHandler.generated.h"

/**
 * Handler for sequential input mini-games
 * Numpad codes, QTE sequences, button combos
 */
UCLASS(BlueprintType, Blueprintable)
class SIMULATORFRAMEWORK_API USequenceHandler : public UMiniGameHandlerBase
{
    GENERATED_BODY()

public:
    // === LIFECYCLE ===

    virtual void Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID) override;
    virtual void Activate() override;
    virtual void Deactivate() override;
    virtual void TickHandler(float DeltaTime) override;
    virtual bool NeedsTick() const override { return SequenceConfig.InputTimeout > 0.0f; }

    // === INPUT ===

    virtual void ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed) override;

    // === SEQUENCE STATE ===

    /** Get current input sequence */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sequence")
    const TArray<FGameplayTag>& GetCurrentSequence() const { return CurrentSequence; }

    /** Get expected sequence length */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sequence")
    int32 GetExpectedLength() const { return SequenceConfig.GetSequenceLength(); }

    /** Get current input count */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sequence")
    int32 GetCurrentInputCount() const { return CurrentSequence.Num(); }

    /** Get error count */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sequence")
    int32 GetErrorCount() const { return ErrorCount; }

    /** Check if last input was correct */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Sequence")
    bool WasLastInputCorrect() const { return bLastInputCorrect; }

protected:
    // === CONFIG ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sequence")
    FSequenceInputConfig SequenceConfig;

    /** Expected sequence (may come from station) */
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sequence")
    TArray<FGameplayTag> ExpectedSequence;

    // === STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sequence")
    TArray<FGameplayTag> CurrentSequence;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sequence")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sequence")
    float TimeSinceLastInput = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Sequence")
    bool bLastInputCorrect = true;

    // === INTERNAL ===

    /** Process a sequence input */
    void ProcessSequenceInput(const FGameplayTag& InputTag);

    /** Check if current sequence matches expected */
    bool CheckSequenceMatch() const;

    /** Check if current partial sequence is valid */
    bool IsPartialSequenceValid() const;

    /** Reset current sequence */
    void ResetSequence();

    /** Handle timeout */
    void HandleTimeout();

    /** Load expected sequence from station if available */
    void LoadExpectedSequence();

    /** Report sequence progress */
    void ReportSequenceProgress();
};