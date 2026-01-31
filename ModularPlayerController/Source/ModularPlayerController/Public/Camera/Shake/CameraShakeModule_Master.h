// CameraShakeModule_Master.h
// Windwalker Framework - Base Camera Shake Module
// Modules provide shake presets, Manager orchestrates playback

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Config/CameraShakeConfigStructs.h"
#include "CameraShakeModule_Master.generated.h"

class AMPC_PlayerCameraManager;

/** Broadcast when a shake starts */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnCameraShakeStarted,
    FGameplayTag, ShakeTag,
    int32, InstanceID);

/** Broadcast when a shake ends */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnCameraShakeEnded,
    FGameplayTag, ShakeTag,
    int32, InstanceID);

/**
 * Base class for camera shake modules
 * Stores shake presets and handles playback logic
 * 
 * Subclass to create themed shake collections:
 * - UCameraShakeModule_Combat (explosions, gunfire, melee)
 * - UCameraShakeModule_Environment (earthquake, wind, footsteps)
 * - UCameraShakeModule_Vehicle (engine, collision, boost)
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class MODULARPLAYERCONTROLLER_API UCameraShakeModule_Master : public UObject
{
    GENERATED_BODY()

public:
    UCameraShakeModule_Master();

    // ========================================================================
    // LIFECYCLE
    // ========================================================================
    
    virtual void Initialize(AMPC_PlayerCameraManager* InManager);
    virtual void Deinitialize();

    // ========================================================================
    // BROADCAST EVENTS
    // ========================================================================
    
    UPROPERTY(BlueprintAssignable, Category="CameraShake|Events")
    FOnCameraShakeStarted OnShakeStarted;
    
    UPROPERTY(BlueprintAssignable, Category="CameraShake|Events")
    FOnCameraShakeEnded OnShakeEnded;

    // ========================================================================
    // MAIN API - Single Entry Point
    // ========================================================================
    
    /**
     * Play a camera shake by tag
     * @param ShakeTag - Tag identifying the shake preset
     * @param Scale - Optional scale multiplier (default 1.0)
     * @return Instance ID for this shake (use to stop it), -1 if failed
     */
    int32 PlayShake(FGameplayTag ShakeTag, float Scale = 1.f);
    
    /**
     * Play a camera shake at a world location (distance-based falloff)
     * @param ShakeTag - Tag identifying the shake preset
     * @param SourceLocation - World location of shake source
     * @param Scale - Optional scale multiplier
     * @return Instance ID for this shake, -1 if failed
     */
    int32 PlayShakeAtLocation(FGameplayTag ShakeTag, FVector SourceLocation, float Scale = 1.f);
    
    /**
     * Stop a specific shake instance
     * @param InstanceID - ID returned from PlayShake
     * @param bImmediate - If true, stop immediately without blend out
     */
    void StopShake(int32 InstanceID, bool bImmediate = false);
    
    /**
     * Stop all shakes with a specific tag
     * @param ShakeTag - Tag to stop
     * @param bImmediate - If true, stop immediately without blend out
     */
    void StopShakeByTag(FGameplayTag ShakeTag, bool bImmediate = false);
    
    /**
     * Stop all active shakes
     * @param bImmediate - If true, stop immediately without blend out
     */
    void StopAllShakes(bool bImmediate = false);

    // ========================================================================
    // UPDATE (called by Manager)
    // ========================================================================
    
    /**
     * Update all active shakes and calculate combined output
     * @param DeltaTime - Frame delta time
     * @param CameraLocation - Current camera world location (for distance calc)
     * @return Combined shake output for this frame
     */
    FCameraShakeOutput UpdateShakes(float DeltaTime, const FVector& CameraLocation);

    // ========================================================================
    // GETTERS
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category="CameraShake")
    FGameplayTag GetModuleTag() const { return ModuleTag; }
    
    UFUNCTION(BlueprintPure, Category="CameraShake")
    const FText& GetDisplayName() const { return DisplayName; }
    
    UFUNCTION(BlueprintPure, Category="CameraShake")
    const FGameplayTagContainer& GetSupportedShakes() const { return SupportedShakes; }
    
    UFUNCTION(BlueprintPure, Category="CameraShake")
    const TArray<FCameraShakePreset>& GetShakePresets() const { return ShakePresets; }
    
    UFUNCTION(BlueprintPure, Category="CameraShake")
    const TArray<FCameraShakeInstance>& GetActiveInstances() const { return ActiveInstances; }
    
    UFUNCTION(BlueprintPure, Category="CameraShake")
    int32 GetActiveShakeCount() const;
    
    UFUNCTION(BlueprintPure, Category="CameraShake")
    bool IsShakeActive(FGameplayTag ShakeTag) const;

protected:
    // ========================================================================
    // IDENTITY
    // ========================================================================
    
    /** Tag identifying this shake module */
    UPROPERTY(EditDefaultsOnly, Category="Module")
    FGameplayTag ModuleTag;
    
    UPROPERTY(EditDefaultsOnly, Category="Module")
    FText DisplayName;

    // ========================================================================
    // SUPPORTED SHAKES (populated from presets)
    // ========================================================================
    
    UPROPERTY(Transient)
    FGameplayTagContainer SupportedShakes;

    // ========================================================================
    // SHAKE PRESETS
    // ========================================================================
    
    /** All available shake presets - define in subclass constructor */
    UPROPERTY(EditDefaultsOnly, Category="Shake|Presets")
    TArray<FCameraShakePreset> ShakePresets;

    // ========================================================================
    // RUNTIME STATE
    // ========================================================================
    
    UPROPERTY(Transient)
    TArray<FCameraShakeInstance> ActiveInstances;
    
    UPROPERTY(Transient)
    int32 NextInstanceID = 1;
    
    UPROPERTY(Transient)
    AMPC_PlayerCameraManager* OwnerManager = nullptr;

    // ========================================================================
    // INTERNAL
    // ========================================================================
    
    const FCameraShakePreset* FindPresetByTag(FGameplayTag ShakeTag) const;
    int32 FindPresetIndexByTag(FGameplayTag ShakeTag) const;
    void PopulateSupportedShakes();
    
    FCameraShakeOutput CalculateShakeOutput(const FCameraShakeInstance& Instance, const FCameraShakePreset& Preset, const FVector& CameraLocation);
    float CalculateOscillation(const FCameraShakeOscillation& Osc, float Time, float BlendAlpha);
    float CalculateBlendAlpha(const FCameraShakeInstance& Instance, const FCameraShakePreset& Preset);
    float CalculateDistanceScale(const FCameraShakeInstance& Instance, const FCameraShakePreset& Preset, const FVector& CameraLocation);
    
    void CleanupFinishedInstances();
};