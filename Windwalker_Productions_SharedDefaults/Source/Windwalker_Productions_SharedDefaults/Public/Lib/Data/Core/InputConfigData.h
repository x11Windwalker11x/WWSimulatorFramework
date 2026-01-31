// InputConfigData.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lib/Enums/WW_Enums.h"
#include "InputConfigData.generated.h"

/**
 * Configuration for a single input axis
 * Atomic struct - smallest unit of input definition
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FAxisInputConfig
{
    GENERATED_BODY()

    /** Is this axis enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = false;

    /** Input source for this axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EInputSource Source = EInputSource::Mouse;

    /** Sensitivity multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", ClampMax = "10.0"))
    float Sensitivity = 1.0f;

    /** Invert input direction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInvert = false;

    /** Clamp output value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bClamp = false;

    /** Minimum clamp value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bClamp"))
    float ClampMin = -180.0f;

    /** Maximum clamp value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bClamp"))
    float ClampMax = 180.0f;

    /** Reference frame for this axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAxisReference Reference = EAxisReference::View;

    bool IsValid() const { return bEnabled; }
};

/**
 * Configuration for sweetspot/target finding mechanics
 * Used by: Lockpicking, dial codes, calibration
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FSweetspotConfig
{
    GENERATED_BODY()

    /** Target value to find (0-1 normalized or 0-360 for dials) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue = 0.5f;

    /** Tolerance at easiest difficulty */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float ToleranceMax = 0.15f;

    /** Tolerance at hardest difficulty */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float ToleranceMin = 0.02f;

    /** Type of feedback when approaching sweetspot */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFeedbackType FeedbackType = EFeedbackType::Combined;

    /** Exponent for feedback intensity curve (1 = linear, 2 = quadratic) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float FeedbackCurveExponent = 1.5f;

    /** Does failure break the tool/pick */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFailureBreaksItem = true;

    /** Maximum attempts before lockout (0 = unlimited) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAttempts = 0;

    /** Get tolerance for given difficulty (0-1, 0=hardest) */
    float GetToleranceForDifficulty(float Difficulty) const
    {
        return FMath::Lerp(ToleranceMin, ToleranceMax, FMath::Clamp(Difficulty, 0.0f, 1.0f));
    }
};

/**
 * Configuration for timing window mechanics
 * Used by: Rhythm games, tumbler clicks, QTE timing
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FTimingWindowConfig
{
    GENERATED_BODY()

    /** Window start position in cycle (0-1 normalized) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindowStart = 0.4f;

    /** Window end position in cycle (0-1 normalized) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindowEnd = 0.6f;

    /** Duration of one cycle in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"))
    float CycleDuration = 1.0f;

    /** How much window shrinks per success */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "0.5"))
    float WindowShrinkRate = 0.02f;

    /** Does the cycle repeat */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = true;

    /** Get window size */
    float GetWindowSize() const { return FMath::Max(0.01f, WindowEnd - WindowStart); }

    /** Check if position is within window */
    bool IsInWindow(float NormalizedPosition) const
    {
        return NormalizedPosition >= WindowStart && NormalizedPosition <= WindowEnd;
    }
};

/**
 * Configuration for sequential input mechanics
 * Used by: Numpad codes, QTE sequences, button combos
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FSequenceInputConfig
{
    GENERATED_BODY()

    /** Expected input sequence (tags like Input.Numpad.1, Input.QTE.North) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> ExpectedSequence;

    /** Time allowed between inputs before reset (0 = no timeout) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float InputTimeout = 5.0f;

    /** Must inputs be in exact order */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOrderMatters = true;

    /** Can player backspace/undo last input */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowBackspace = true;

    /** Maximum errors before fail (0 = fail on first error) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxErrors = 0;

    bool IsValid() const { return ExpectedSequence.Num() > 0; }
    int32 GetSequenceLength() const { return ExpectedSequence.Num(); }
};

/**
 * Configuration for snap point mechanics
 * Used by: Assembly, jigsaw, part attachment
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FSnapPointConfig
{
    GENERATED_BODY()

    /** Distance at which snap preview activates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0"))
    float SnapDistance = 50.0f;

    /** Must rotation match for snap */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequireRotationMatch = true;

    /** Rotation tolerance in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", ClampMax = "180.0", EditCondition = "bRequireRotationMatch"))
    float RotationTolerance = 15.0f;

    /** Tags of items that can snap here */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer AcceptedItemTags;

    /** Check if item tag is accepted */
    bool AcceptsItem(const FGameplayTag& ItemTag) const
    {
        return AcceptedItemTags.IsEmpty() || AcceptedItemTags.HasTag(ItemTag);
    }
};

/**
 * Configuration for temperature zone mechanics
 * Used by: Cooking, smithing, heat control
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FTemperatureZoneConfig
{
    GENERATED_BODY()

    /** Minimum optimal temperature */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OptimalMin = 150.0f;

    /** Maximum optimal temperature */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OptimalMax = 200.0f;

    /** Temperature below this = frozen/ruined */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DangerMin = 50.0f;

    /** Temperature above this = burned/ruined */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DangerMax = 300.0f;

    /** Rate of heating (degrees per second at max input) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"))
    float HeatRate = 25.0f;

    /** Rate of cooling (degrees per second with no input) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"))
    float CoolRate = 10.0f;

    /** Can item be ruined by overheating */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanOverheat = true;

    /** Can item be ruined by freezing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanFreeze = false;

    /** Duration must stay in optimal zone (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float RequiredDurationInZone = 5.0f;

    /** Check if temperature is in optimal zone */
    bool IsInOptimalZone(float Temperature) const
    {
        return Temperature >= OptimalMin && Temperature <= OptimalMax;
    }

    /** Check if temperature is in danger zone */
    bool IsInDangerZone(float Temperature) const
    {
        return (bCanFreeze && Temperature < DangerMin) || (bCanOverheat && Temperature > DangerMax);
    }
};