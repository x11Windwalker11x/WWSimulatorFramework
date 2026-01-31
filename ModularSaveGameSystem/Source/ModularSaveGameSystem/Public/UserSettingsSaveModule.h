// UserSettingsSaveModule.h
// Windwalker Framework - User Settings Save Module
// Saves user-configurable settings like FOV

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MasterSaveGame.h"
#include "UserSettingsSaveModule.generated.h"

/**
 * Camera settings that users can configure
 */
USTRUCT(BlueprintType)
struct MODULARSAVEGAMESYSTEM_API FCameraUserSettings
{
    GENERATED_BODY()
    
    /** User-configured FOV */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Camera")
    float FieldOfView = 90.f;
    
    /** Last used camera mode */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Camera")
    FGameplayTag LastCameraMode;
    
    /** Last shoulder side */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Camera")
    FGameplayTag LastShoulderState;
    
    /** Mouse sensitivity */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Camera")
    float MouseSensitivity = 1.0f;
    
    /** Invert Y axis */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Camera")
    bool bInvertY = false;
    
    /** Enable camera shake */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Camera")
    bool bEnableCameraShake = true;
    
    /** Head bob intensity (0-1) */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0.0", ClampMax="1.0"))
    float HeadBobIntensity = 0.5f;
};

/**
 * Audio settings that users can configure
 */
USTRUCT(BlueprintType)
struct MODULARSAVEGAMESYSTEM_API FAudioUserSettings
{
    GENERATED_BODY()
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MasterVolume = 1.0f;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MusicVolume = 0.8f;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
    float SFXVolume = 1.0f;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
    float VoiceVolume = 1.0f;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
    float AmbientVolume = 0.7f;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Audio")
    bool bEnableSubtitles = true;
};

/**
 * Graphics settings that users can configure
 */
USTRUCT(BlueprintType)
struct MODULARSAVEGAMESYSTEM_API FGraphicsUserSettings
{
    GENERATED_BODY()
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics")
    int32 ResolutionX = 1920;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics")
    int32 ResolutionY = 1080;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics")
    bool bFullscreen = true;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics")
    bool bVSync = true;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics")
    int32 FrameRateLimit = 60;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics", meta=(ClampMin="0", ClampMax="4"))
    int32 ShadowQuality = 3;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics", meta=(ClampMin="0", ClampMax="4"))
    int32 TextureQuality = 3;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics", meta=(ClampMin="0", ClampMax="4"))
    int32 EffectsQuality = 3;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics", meta=(ClampMin="0", ClampMax="4"))
    int32 PostProcessQuality = 3;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics")
    bool bMotionBlur = true;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Graphics")
    float Gamma = 2.2f;
};

/**
 * Gameplay settings that users can configure
 */
USTRUCT(BlueprintType)
struct MODULARSAVEGAMESYSTEM_API FGameplayUserSettings
{
    GENERATED_BODY()
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Gameplay")
    bool bShowDamageNumbers = true;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Gameplay")
    bool bShowHealthBars = true;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Gameplay")
    bool bAutoPickupItems = false;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Gameplay")
    bool bShowTutorialHints = true;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Gameplay")
    FString Language = TEXT("en");
};

/**
 * User Settings Save Module
 * Stores all user-configurable settings
 */
UCLASS(BlueprintType)
class MODULARSAVEGAMESYSTEM_API UUserSettingsSaveModule : public UModularSaveData
{
    GENERATED_BODY()

public:
    UUserSettingsSaveModule();

    // === SETTINGS DATA ===
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Settings")
    FCameraUserSettings CameraSettings;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Settings")
    FAudioUserSettings AudioSettings;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Settings")
    FGraphicsUserSettings GraphicsSettings;
    
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Settings")
    FGameplayUserSettings GameplaySettings;

    // === GETTERS ===
    
    UFUNCTION(BlueprintPure, Category="User Settings")
    const FCameraUserSettings& GetCameraSettings() const { return CameraSettings; }
    
    UFUNCTION(BlueprintPure, Category="User Settings")
    const FAudioUserSettings& GetAudioSettings() const { return AudioSettings; }
    
    UFUNCTION(BlueprintPure, Category="User Settings")
    const FGraphicsUserSettings& GetGraphicsSettings() const { return GraphicsSettings; }
    
    UFUNCTION(BlueprintPure, Category="User Settings")
    const FGameplayUserSettings& GetGameplaySettings() const { return GameplaySettings; }

    // === SETTERS ===
    
    UFUNCTION(BlueprintCallable, Category="User Settings")
    void SetCameraSettings(const FCameraUserSettings& InSettings) { CameraSettings = InSettings; }
    
    UFUNCTION(BlueprintCallable, Category="User Settings")
    void SetAudioSettings(const FAudioUserSettings& InSettings) { AudioSettings = InSettings; }
    
    UFUNCTION(BlueprintCallable, Category="User Settings")
    void SetGraphicsSettings(const FGraphicsUserSettings& InSettings) { GraphicsSettings = InSettings; }
    
    UFUNCTION(BlueprintCallable, Category="User Settings")
    void SetGameplaySettings(const FGameplayUserSettings& InSettings) { GameplaySettings = InSettings; }

    // === CONVENIENCE - FOV ===
    
    UFUNCTION(BlueprintPure, Category="User Settings|Camera")
    float GetFOV() const { return CameraSettings.FieldOfView; }
    
    UFUNCTION(BlueprintCallable, Category="User Settings|Camera")
    void SetFOV(float NewFOV) { CameraSettings.FieldOfView = FMath::Clamp(NewFOV, 60.f, 120.f); }

    // === UModularSaveData Interface ===
    
    virtual void ClearData() override;
    virtual bool ValidateData_Implementation() override;
    virtual void MigrateData_Implementation(int32 FromVersion, int32 ToVersion) override;
};