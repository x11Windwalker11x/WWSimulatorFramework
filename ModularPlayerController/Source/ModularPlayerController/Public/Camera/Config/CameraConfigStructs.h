// CameraConfigStructs.h
// Windwalker Framework - Camera Configuration Structs
// DATA ONLY - No logic

#pragma once

#include "CoreMinimal.h"
#include "CameraConfigStructs.generated.h"

/**
 * Camera preset for a specific gameplay state
 */
// Add to CameraConfigStructs.h

/**
 * Camera preset for custom state-driven camera
 * Used by UCameraModule_Custom for arbitrary states (killcam, stealth, cinematic, etc.)
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraStatePreset
{
    GENERATED_BODY()

    /** Tag that triggers this preset (Camera.Mode.Custom.Killcam, etc.) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset")
    FGameplayTag StateTag;
    
    /** Display name for debugging */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset")
    FText DisplayName;

    /** Priority - higher wins when multiple could apply */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset", meta=(ClampMin="0", ClampMax="100"))
    int32 Priority = 0;

    /** Transition speed (higher = faster) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset", meta=(ClampMin="0.1", ClampMax="50.0"))
    float TransitionSpeed = 5.f;

    // === CAMERA VALUES ===
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Camera")
    float FOV = 90.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Camera")
    float ArmLength = 300.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Camera")
    FVector SocketOffset = FVector::ZeroVector;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Camera")
    FVector TargetOffset = FVector::ZeroVector;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Camera")
    FRotator RotationOffset = FRotator::ZeroRotator;

    // === SPRING ARM ===
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|SpringArm")
    bool bUseSpringArm = true;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|SpringArm", meta=(EditCondition="bUseSpringArm"))
    bool bEnableCameraLag = true;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|SpringArm", meta=(EditCondition="bEnableCameraLag"))
    float CameraLagSpeed = 10.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|SpringArm", meta=(EditCondition="bUseSpringArm"))
    bool bEnableCameraRotationLag = false;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|SpringArm", meta=(EditCondition="bEnableCameraRotationLag"))
    float CameraRotationLagSpeed = 10.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|SpringArm")
    bool bUsePawnControlRotation = true;

    // === ATTACHMENT ===
    
    /** Socket to attach camera (if not using SpringArm or for specific attachment point) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Attachment")
    FName AttachSocket = NAME_None;
    
    /** Lock camera rotation to pawn rotation */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Attachment")
    bool bLockRotationToPawn = false;

    // === CHARACTER ===
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Character")
    bool bUseControllerRotationYaw = false;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Preset|Character")
    bool bOrientRotationToMovement = true;
};

/**
 * SpringArm configuration
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraSpringArmConfig
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="SpringArm")
    float DefaultArmLength = 300.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="SpringArm")
    FVector SocketOffset = FVector::ZeroVector;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="SpringArm")
    FVector TargetOffset = FVector::ZeroVector;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="SpringArm")
    bool bUsePawnControlRotation = true;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="SpringArm")
    bool bEnableCameraLag = false;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="SpringArm")
    float CameraLagSpeed = 10.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="SpringArm")
    bool bEnableCameraRotationLag = false;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="SpringArm")
    float CameraRotationLagSpeed = 10.f;
};

/**
 * Zoom configuration
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraZoomConfig
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Zoom")
    float MinArmLength = 50.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Zoom")
    float MaxArmLength = 600.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Zoom")
    float ZoomSpeed = 20.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Zoom")
    float ZoomInterpSpeed = 10.f;
};

/**
 * Aim state configuration
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraAimConfig
{
    GENERATED_BODY()
    
    // === HIP FIRE (Camera.Aim.Hip) ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Hip")
    float HipFOV = 85.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Hip")
    float HipArmLength = 280.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Hip")
    FVector HipSocketOffset = FVector::ZeroVector;
    
    // === AIMING / ADS (Camera.Aim.Aiming) ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Aiming")
    float AimingFOV = 70.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Aiming")
    float AimingArmLength = 150.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Aiming")
    FVector AimingSocketOffset = FVector(0.f, 50.f, 10.f);
    
    // === SCOPE (Camera.Aim.Scope) ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Scope")
    float ScopeFOV = 40.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Scope")
    float ScopeArmLength = 80.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Scope")
    FVector ScopeSocketOffset = FVector(0.f, 40.f, 5.f);
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Scope")
    bool bScopeUsesRenderTarget = true;
};

/**
 * Shoulder offset configuration (TPS only)
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraShoulderConfig
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Shoulder")
    float ShoulderOffsetY = 60.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Shoulder")
    FVector CenterOffset = FVector::ZeroVector;
};

/**
 * Interpolation speeds configuration
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraInterpolationConfig
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Interpolation")
    float FOVInterpSpeed = 10.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Interpolation")
    float ArmLengthInterpSpeed = 10.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Interpolation")
    float SocketOffsetInterpSpeed = 8.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Interpolation")
    float ShoulderInterpSpeed = 12.f;
};

/**
 * Character rotation configuration
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraCharacterConfig
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Character")
    bool bUseControllerRotationYaw = false;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Character")
    bool bOrientRotationToMovement = true;
};

/**
 * Runtime interpolation state (transient, not saved)
 */
USTRUCT(BlueprintType)
struct MODULARPLAYERCONTROLLER_API FCameraRuntimeState
{
    GENERATED_BODY()
    
    // Target values (interpolating TO)
    UPROPERTY(Transient)
    float TargetFOV = 90.f;
    
    UPROPERTY(Transient)
    float TargetArmLength = 300.f;
    
    UPROPERTY(Transient)
    FVector TargetSocketOffset = FVector::ZeroVector;
    
    // Current values (interpolating FROM)
    UPROPERTY(Transient)
    float CurrentFOV = 90.f;
    
    UPROPERTY(Transient)
    float CurrentArmLength = 300.f;
    
    UPROPERTY(Transient)
    FVector CurrentSocketOffset = FVector::ZeroVector;
    
    // Zoom state
    UPROPERTY(Transient)
    float ZoomTargetArmLength = 300.f;
    
    // Transform cache
    UPROPERTY(Transient)
    FTransform CachedTransform;
    
    UPROPERTY(Transient)
    float CachedArmLength = 300.f;
    
    UPROPERTY(Transient)
    bool bHasCachedTransform = false;
};