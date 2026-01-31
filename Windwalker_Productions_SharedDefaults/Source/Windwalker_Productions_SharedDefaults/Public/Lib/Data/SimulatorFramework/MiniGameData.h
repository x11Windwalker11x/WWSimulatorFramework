// MiniGameData.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/Core/InputConfigData.h"
#include "Lib/Data/Core/ObjectiveData.h"
#include "Lib/Data/Enums/WW_Enums.h"
#include "MiniGameData.generated.h"

class UMiniGameHandlerBase;

/**
 * Configuration for 6-axis manipulation
 * Composite struct - built from FAxisInputConfig atomics
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FManipulationConfig
{
    GENERATED_BODY()

    // === MOVEMENT ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FAxisInputConfig MovementX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FAxisInputConfig MovementY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FAxisInputConfig MovementZ;

    // === ROTATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    FAxisInputConfig RotationPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    FAxisInputConfig RotationYaw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    FAxisInputConfig RotationRoll;

    // === SNAP SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap")
    FSnapPointConfig SnapSettings;

    // === GRAB SETTINGS ===

    /** Can player drop object during manipulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    bool bAllowDrop = true;

    /** Maximum grab distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab", meta = (ClampMin = "50.0"))
    float MaxGrabDistance = 300.0f;

    /** Minimum hold distance from camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab", meta = (ClampMin = "10.0"))
    float HoldDistanceMin = 50.0f;

    /** Maximum hold distance from camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab", meta = (ClampMin = "50.0"))
    float HoldDistanceMax = 200.0f;

    /** Check if any movement axis is enabled */
    bool HasMovement() const
    {
        return MovementX.bEnabled || MovementY.bEnabled || MovementZ.bEnabled;
    }

    /** Check if any rotation axis is enabled */
    bool HasRotation() const
    {
        return RotationPitch.bEnabled || RotationYaw.bEnabled || RotationRoll.bEnabled;
    }
};

/**
 * Configuration for lockpicking mechanics
 * Composite struct - uses FSweetspotConfig
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FLockpickConfig
{
    GENERATED_BODY()

    /** Sweetspot detection settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweetspot")
    FSweetspotConfig SweetspotSettings;

    /** Lockpicking mechanic style */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    ELockpickStyle Style = ELockpickStyle::Skyrim;

    /** Number of pins/stages (for multi-pin locks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "1", ClampMax = "10"))
    int32 PinCount = 1;

    /** Difficulty multiplier (affects tolerance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float DifficultyMultiplier = 1.0f;

    /** Randomize sweetspot position on failure */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bRandomizeSweetspotOnFail = true;

    /** Time limit for lock attempt (0 = no limit) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float TimeLimit = 0.0f;

    /** Get adjusted tolerance based on difficulty */
    float GetAdjustedTolerance(float BaseDifficulty) const
    {
        float Tolerance = SweetspotSettings.GetToleranceForDifficulty(BaseDifficulty);
        return Tolerance / DifficultyMultiplier;
    }
};

/**
 * Configuration for calibration/precision mechanics
 * Composite struct - uses FSweetspotConfig
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FCalibrationConfig
{
    GENERATED_BODY()

    /** Sweetspot/target zone settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    FSweetspotConfig SweetspotSettings;

    /** Rate at which zone shrinks over time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ZoneShrinkRate = 0.0f;

    /** Duration player must hold in zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Completion", meta = (ClampMin = "0.1"))
    float RequiredHoldDuration = 2.0f;

    /** Does the target move */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bMovingTarget = false;

    /** Speed of moving target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", EditCondition = "bMovingTarget"))
    float TargetMoveSpeed = 0.5f;
};

/**
 * Configuration for rhythm/timing mechanics
 * Composite struct - uses FTimingWindowConfig
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FRhythmConfig
{
    GENERATED_BODY()

    /** Timing window settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    FTimingWindowConfig TimingSettings;

    /** Number of successful hits required */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Completion", meta = (ClampMin = "1"))
    int32 RequiredSuccessCount = 5;

    /** Maximum misses before failure (0 = no limit) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "0"))
    int32 MaxMissesAllowed = 3;

    /** Does tempo increase over time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIncreasingTempo = false;

    /** Rate of tempo increase per success */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "0.5", EditCondition = "bIncreasingTempo"))
    float TempoIncreaseRate = 0.05f;

    /** Minimum cycle duration (tempo cap) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.1", EditCondition = "bIncreasingTempo"))
    float MinCycleDuration = 0.3f;
};

/**
 * Snap point data for assembly mini-games
 * Defines a single attachment point on an object
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FSnapPointData
{
    GENERATED_BODY()

    /** Local position on parent actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector LocalPosition = FVector::ZeroVector;

    /** Required rotation for snap (local space) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FRotator RequiredRotation = FRotator::ZeroRotator;

    /** Tag identifying this snap point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FGameplayTag SnapPointTag;

    /** Tags of items accepted at this point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FGameplayTagContainer AcceptedItemTags;

    /** Snap distance override (0 = use config default) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float SnapDistanceOverride = 0.0f;

    /** Objective tag to report when item snaps here */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FGameplayTag OnSnapObjectiveTag;

    /** Is this snap point currently occupied */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsOccupied = false;

    /** Actor currently snapped here */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    TWeakObjectPtr<AActor> SnappedActor;

    bool AcceptsItem(const FGameplayTag& ItemTag) const
    {
        return AcceptedItemTags.IsEmpty() || AcceptedItemTags.HasTag(ItemTag);
    }
};

/**
 * Main mini-game definition for DataTables
 * References all config types via type tag selection
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FMiniGameData : public FTableRowBase
{
    GENERATED_BODY()

    // === IDENTITY ===

    /** Unique identifier for this mini-game */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FGameplayTag MiniGameID;

    /** Display name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FText DisplayName;

    /** Mini-game type (MiniGame.Type.*) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FGameplayTag TypeTag;

    // === CAMERA ===

    /** Camera mode to request (Camera.Mode.*) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FGameplayTag CameraModeTag;

    // === OBJECTIVES ===

    /** Objectives for this mini-game */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objectives")
    FObjectiveSet Objectives;

    // === BEHAVIOR ===

    /** Handler class to spawn (if null, inferred from TypeTag) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TSubclassOf<UMiniGameHandlerBase> HandlerClass;

    /** Can player cancel this mini-game */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bCanCancel = true;

    /** Time limit (0 = no timeout) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0"))
    float TimeoutSeconds = 0.0f;

    // === TYPE-SPECIFIC CONFIGS ===
    // Only one should be populated based on TypeTag

    /** Config for MiniGame.Type.Manipulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Manipulation")
    FManipulationConfig ManipulationConfig;

    /** Config for MiniGame.Type.Lockpick */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Lockpick")
    FLockpickConfig LockpickConfig;

    /** Config for MiniGame.Type.Sequence */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Sequence")
    FSequenceInputConfig SequenceConfig;

    /** Config for MiniGame.Type.Timing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Timing")
    FRhythmConfig RhythmConfig;

    /** Config for MiniGame.Type.Calibration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Calibration")
    FCalibrationConfig CalibrationConfig;

    /** Config for MiniGame.Type.Temperature */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Temperature")
    FTemperatureZoneConfig TemperatureConfig;

    // === VALIDATION ===

    bool IsValid() const
    {
        return MiniGameID.IsValid() && TypeTag.IsValid();
    }
};