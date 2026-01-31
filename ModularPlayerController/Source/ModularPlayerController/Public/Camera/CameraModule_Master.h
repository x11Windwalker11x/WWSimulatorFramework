// CameraModule_Master.h
// Windwalker Framework - Base Camera Module
// Modules are DATA PROVIDERS - Manager ORCHESTRATES

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Config/CameraConfigStructs.h"
#include "CameraModule_Master.generated.h"

class AMPC_PlayerCameraManager;

/** Broadcast when zoom level changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnModuleZoomChanged,
    float, ArmLength,
    float, ZoomPercent);

/**
 * Base class for all camera modules
 * Stores configuration and runtime state
 * Manager reads this data and applies to actual components
 * 
 * Subclasses:
 * - UCameraModule_TPS
 * - UCameraModule_FPS
 * - UCameraModule_TrueFPS
 * - UCameraModule_TopDown
 * - UCameraModule_Vehicle
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class MODULARPLAYERCONTROLLER_API UCameraModule_Master : public UObject
{
    GENERATED_BODY()

public:
    UCameraModule_Master();

    // ========================================================================
    // LIFECYCLE
    // ========================================================================
    
    virtual void OnActivated();
    virtual void OnDeactivated();

    // ========================================================================
    // BROADCAST EVENTS
    // ========================================================================
    
    UPROPERTY(BlueprintAssignable, Category="Camera|Events")
    FOnModuleZoomChanged OnZoomChanged;

    // ========================================================================
    // STATE SETTERS (called by Manager)
    // ========================================================================
    
    void SetAimState(FGameplayTag NewAimState);
    void SetShoulderState(FGameplayTag NewShoulderState);

    // ========================================================================
    // ZOOM CONTROL
    // ========================================================================
    
    void AdjustZoom(float ScrollDelta);
    void SetTargetArmLength(float NewLength);
    void UpdateInterpolation(float DeltaTime);

    // ========================================================================
    // TRANSFORM CACHE
    // ========================================================================
    
    void CacheTransform(const FTransform& InTransform, float InArmLength);
    void GetCachedTransform(FTransform& OutTransform, float& OutArmLength) const;
    void ClearCachedTransform();

    // ========================================================================
    // IDENTITY GETTERS
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category="Camera Module")
    FGameplayTag GetCameraModeTag() const { return CameraModeTag; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module")
    const FText& GetDisplayName() const { return DisplayName; }

    // ========================================================================
    // CAPABILITY QUERY
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category="Camera Module")
    const FGameplayTagContainer& GetSupportedCapabilities() const { return SupportedCapabilities; }

    // ========================================================================
    // CONFIG STRUCT GETTERS
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Config")
    const FCameraSpringArmConfig& GetSpringArmConfig() const { return SpringArmConfig; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Config")
    const FCameraZoomConfig& GetZoomConfig() const { return ZoomConfig; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Config")
    const FCameraAimConfig& GetAimConfig() const { return AimConfig; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Config")
    const FCameraShoulderConfig& GetShoulderConfig() const { return ShoulderConfig; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Config")
    const FCameraInterpolationConfig& GetInterpConfig() const { return InterpConfig; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Config")
    const FCameraCharacterConfig& GetCharacterConfig() const { return CharacterConfig; }

    // ========================================================================
    // CONFIG STRUCT SETTERS
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category="Camera Module|Config")
    void SetSpringArmConfig(const FCameraSpringArmConfig& InConfig) { SpringArmConfig = InConfig; }
    
    UFUNCTION(BlueprintCallable, Category="Camera Module|Config")
    void SetZoomConfig(const FCameraZoomConfig& InConfig) { ZoomConfig = InConfig; }
    
    UFUNCTION(BlueprintCallable, Category="Camera Module|Config")
    void SetAimConfig(const FCameraAimConfig& InConfig) { AimConfig = InConfig; }
    
    UFUNCTION(BlueprintCallable, Category="Camera Module|Config")
    void SetShoulderConfig(const FCameraShoulderConfig& InConfig) { ShoulderConfig = InConfig; }
    
    UFUNCTION(BlueprintCallable, Category="Camera Module|Config")
    void SetInterpConfig(const FCameraInterpolationConfig& InConfig) { InterpConfig = InConfig; }
    
    UFUNCTION(BlueprintCallable, Category="Camera Module|Config")
    void SetCharacterConfig(const FCameraCharacterConfig& InConfig) { CharacterConfig = InConfig; }

    // ========================================================================
    // STATE GETTERS
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category="Camera Module|State")
    FGameplayTag GetCurrentAimState() const { return CurrentAimState; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module|State")
    FGameplayTag GetCurrentShoulderState() const { return CurrentShoulderState; }

    // ========================================================================
    // EFFECTIVE VALUE GETTERS (interpolated current values)
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Runtime")
    float GetEffectiveFOV() const { return RuntimeState.CurrentFOV; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Runtime")
    float GetEffectiveArmLength() const { return RuntimeState.CurrentArmLength; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Runtime")
    const FVector& GetEffectiveSocketOffset() const { return RuntimeState.CurrentSocketOffset; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module|Runtime")
    const FCameraRuntimeState& GetRuntimeState() const { return RuntimeState; }

    // ========================================================================
    // OTHER GETTERS
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category="Camera Module")
    float GetBaseFOV() const { return BaseFOV; }
    
    UFUNCTION(BlueprintCallable, Category="Camera Module")
    void SetBaseFOV(float NewFOV) { BaseFOV = FMath::Clamp(NewFOV, 60.f, 120.f); }
    
    UFUNCTION(BlueprintPure, Category="Camera Module")
    FName GetAttachSocketName() const { return AttachSocketName; }
    
    UFUNCTION(BlueprintPure, Category="Camera Module")
    const FCameraAimConfig& GetAimConfigForCurrentState() const { return AimConfig; }

protected:
    // ========================================================================
    // IDENTITY
    // ========================================================================
    
    UPROPERTY(EditDefaultsOnly, Category="Module")
    FGameplayTag CameraModeTag;
    
    UPROPERTY(EditDefaultsOnly, Category="Module")
    FText DisplayName;

    // ========================================================================
    // CAPABILITIES (contains Camera.Aim.X and Camera.Shoulder.X tags)
    // ========================================================================
    
    UPROPERTY(EditDefaultsOnly, Category="Module|Capabilities")
    FGameplayTagContainer SupportedCapabilities;

    // ========================================================================
    // CONFIG STRUCTS
    // ========================================================================
    
    UPROPERTY(EditDefaultsOnly, Category="Config")
    FCameraSpringArmConfig SpringArmConfig;
    
    UPROPERTY(EditDefaultsOnly, Category="Config")
    FCameraZoomConfig ZoomConfig;
    
    UPROPERTY(EditDefaultsOnly, Category="Config")
    FCameraAimConfig AimConfig;
    
    UPROPERTY(EditDefaultsOnly, Category="Config")
    FCameraShoulderConfig ShoulderConfig;
    
    UPROPERTY(EditDefaultsOnly, Category="Config")
    FCameraInterpolationConfig InterpConfig;
    
    UPROPERTY(EditDefaultsOnly, Category="Config")
    FCameraCharacterConfig CharacterConfig;

    // ========================================================================
    // BASE SETTINGS
    // ========================================================================
    
    /** Base FOV - can be overridden by user settings */
    UPROPERTY(EditDefaultsOnly, Category="Camera")
    float BaseFOV = 90.f;
    
    /** Socket name for non-SpringArm attachment (FPS modes) */
    UPROPERTY(EditDefaultsOnly, Category="Camera")
    FName AttachSocketName = NAME_None;

    // ========================================================================
    // RUNTIME STATE
    // ========================================================================
    
    UPROPERTY(Transient)
    FCameraRuntimeState RuntimeState;
    
    UPROPERTY(Transient)
    FGameplayTag CurrentAimState;
    
    UPROPERTY(Transient)
    FGameplayTag CurrentShoulderState;
    
    UPROPERTY(Transient)
    bool bIsActive = false;

    // ========================================================================
    // INTERNAL
    // ========================================================================
    
    void RecalculateTargets();
    void InitializeRuntimeState();
};