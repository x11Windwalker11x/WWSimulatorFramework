// CameraModule_Custom.h
// Windwalker Framework - Custom State-Driven Camera Module

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModule_Master.h"
#include "CameraModule_Custom.generated.h"

/**
 * Custom state-driven camera module
 * Uses TArray of presets for arbitrary states (killcam, stealth, cinematic, etc.)
 * 
 * Usage:
 * 1. Define presets in editor for each state
 * 2. Call SetActivePreset(Tag) when state changes
 * 3. Camera smoothly transitions to new preset
 * 
 * Example:
 *   Manager->SetActiveCameraMode(Camera.Mode.Custom.Killcam);
 *   // or
 *   CustomModule->SetActivePreset(Camera.Mode.Custom.Killcam);
 */
UCLASS(Blueprintable, BlueprintType)
class MODULARPLAYERCONTROLLER_API UCameraModule_Custom : public UCameraModule_Master
{
    GENERATED_BODY()

public:
    UCameraModule_Custom();

    // ========================================================================
    // LIFECYCLE OVERRIDES
    // ========================================================================
    
    virtual void OnActivated() override;

    // ========================================================================
    // PRESET SYSTEM
    // ========================================================================
    
    /**
     * Set active preset by tag
     * Finds matching preset and applies its values
     * @return true if preset found and applied
     */
    UFUNCTION(BlueprintCallable, Category="Camera Module|Custom")
    bool SetActivePreset(FGameplayTag PresetTag);
    
    /**
     * Get current active preset tag
     */
    UFUNCTION(BlueprintPure, Category="Camera Module|Custom")
    FGameplayTag GetActivePresetTag() const { return ActivePresetTag; }
    
    /**
     * Get current active preset (nullptr if none)
     */
    UFUNCTION(BlueprintPure, Category="Camera Module|Custom")
    const FCameraStatePreset* GetActivePreset() const;
    
    /**
     * Find preset by tag
     */
    UFUNCTION(BlueprintPure, Category="Camera Module|Custom")
    const FCameraStatePreset* FindPresetByTag(FGameplayTag PresetTag) const;
    
    /**
     * Get all defined presets
     */
    UFUNCTION(BlueprintPure, Category="Camera Module|Custom")
    const TArray<FCameraStatePreset>& GetStatePresets() const { return StatePresets; }

protected:
    // ========================================================================
    // PRESET DATA
    // ========================================================================
    
    /** All available state presets - define in editor */
    UPROPERTY(EditDefaultsOnly, Category="Custom|Presets")
    TArray<FCameraStatePreset> StatePresets;
    
    /** Default preset to use on activation (Camera.Mode.Custom.Default) */
    UPROPERTY(EditDefaultsOnly, Category="Custom|Presets")
    FGameplayTag DefaultPresetTag;
    
    // ========================================================================
    // RUNTIME STATE
    // ========================================================================
    
    /** Currently active preset tag */
    UPROPERTY(Transient)
    FGameplayTag ActivePresetTag;
    
    /** Cached index of active preset (-1 if none) */
    UPROPERTY(Transient)
    int32 ActivePresetIndex = INDEX_NONE;

    // ========================================================================
    // INTERNAL
    // ========================================================================
    
    void ApplyPreset(const FCameraStatePreset& Preset);
    void PopulateSupportedCapabilities();
};