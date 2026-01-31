#pragma once

struct CameraShakeConfigStructs
{
	
};
// CameraShakeConfigStructs.h
// Windwalker Framework - Camera Shake Configuration Structs
// DATA ONLY - No logic

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CameraShakeConfigStructs.generated.h"

/**
 * Oscillation axis configuration
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraShakeOscillation
{
    GENERATED_BODY()
    
    /** Amplitude of oscillation */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Oscillation")
    float Amplitude = 0.f;
    
    /** Frequency of oscillation (cycles per second) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Oscillation")
    float Frequency = 0.f;
    
    /** Time to blend in */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Oscillation")
    float BlendInTime = 0.1f;
    
    /** Time to blend out */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Oscillation")
    float BlendOutTime = 0.2f;
};

/**
 * Camera shake preset for specific events
 * Used by UCameraShakeModule for gameplay events (explosion, punch, landing, etc.)
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraShakePreset
{
    GENERATED_BODY()

    /** Tag that triggers this shake (CameraShake.Explosion, CameraShake.Punch, etc.) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset")
    FGameplayTag ShakeTag;
    
    /** Display name for debugging */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset")
    FText DisplayName;
    
    /** Priority - higher priority shakes can override lower ones */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset", meta=(ClampMin="0", ClampMax="100"))
    int32 Priority = 0;

    // === DURATION ===
    
    /** Total duration of shake (0 = infinite until stopped) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Duration", meta=(ClampMin="0.0"))
    float Duration = 0.5f;
    
    /** Time to blend in */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Duration", meta=(ClampMin="0.0"))
    float BlendInTime = 0.1f;
    
    /** Time to blend out */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Duration", meta=(ClampMin="0.0"))
    float BlendOutTime = 0.2f;

    // === LOCATION OSCILLATION ===
    
    /** Location oscillation on X axis (forward/back) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Location")
    FCameraShakeOscillation LocationX;
    
    /** Location oscillation on Y axis (left/right) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Location")
    FCameraShakeOscillation LocationY;
    
    /** Location oscillation on Z axis (up/down) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Location")
    FCameraShakeOscillation LocationZ;

    // === ROTATION OSCILLATION ===
    
    /** Rotation oscillation on Pitch (look up/down) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Rotation")
    FCameraShakeOscillation RotationPitch;
    
    /** Rotation oscillation on Yaw (look left/right) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Rotation")
    FCameraShakeOscillation RotationYaw;
    
    /** Rotation oscillation on Roll (tilt) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Rotation")
    FCameraShakeOscillation RotationRoll;

    // === FOV ===
    
    /** FOV oscillation */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|FOV")
    FCameraShakeOscillation FOV;

    // === SCALE ===
    
    /** Global scale multiplier for this shake */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Scale", meta=(ClampMin="0.0", ClampMax="10.0"))
    float Scale = 1.f;
    
    /** Whether shake scales with distance from source */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Scale")
    bool bScaleWithDistance = false;
    
    /** Inner radius - full intensity within this distance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Scale", meta=(EditCondition="bScaleWithDistance", ClampMin="0.0"))
    float InnerRadius = 100.f;
    
    /** Outer radius - zero intensity beyond this distance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Scale", meta=(EditCondition="bScaleWithDistance", ClampMin="0.0"))
    float OuterRadius = 1000.f;

    // === PLAYBACK ===
    
    /** Can this shake be played multiple times simultaneously */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Playback")
    bool bAllowMultipleInstances = false;
    
    /** Should this shake loop until manually stopped */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Playback")
    bool bLooping = false;
};

/**
 * Runtime shake instance state
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraShakeInstance
{
    GENERATED_BODY()
    
    /** Tag identifying this shake type */
    UPROPERTY(Transient)
    FGameplayTag ShakeTag;
    
    /** Unique instance ID */
    UPROPERTY(Transient)
    int32 InstanceID = 0;
    
    /** Preset index in array */
    UPROPERTY(Transient)
    int32 PresetIndex = INDEX_NONE;
    
    /** Current time elapsed */
    UPROPERTY(Transient)
    float CurrentTime = 0.f;
    
    /** Current scale (can be modified at runtime) */
    UPROPERTY(Transient)
    float CurrentScale = 1.f;
    
    /** Is this instance active */
    UPROPERTY(Transient)
    bool bIsActive = false;
    
    /** Is this instance blending out */
    UPROPERTY(Transient)
    bool bIsBlendingOut = false;
    
    /** World location of shake source (for distance scaling) */
    UPROPERTY(Transient)
    FVector SourceLocation = FVector::ZeroVector;
    
    /** Does this instance use source location */
    UPROPERTY(Transient)
    bool bHasSourceLocation = false;
};

/**
 * Output of shake calculation for a single frame
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraShakeOutput
{
    GENERATED_BODY()
    
    UPROPERTY(Transient)
    FVector LocationOffset = FVector::ZeroVector;
    
    UPROPERTY(Transient)
    FRotator RotationOffset = FRotator::ZeroRotator;
    
    UPROPERTY(Transient)
    float FOVOffset = 0.f;
};