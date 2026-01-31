// UserSettingsSaveModule.cpp
// Windwalker Framework - User Settings Save Module Implementation

#include "UserSettingsSaveModule.h"

UUserSettingsSaveModule::UUserSettingsSaveModule()
{
    ModuleName = TEXT("UserSettings");
    ModuleVersion = 1;
    
    // Defaults are set in struct constructors
}

void UUserSettingsSaveModule::ClearData()
{
    CameraSettings = FCameraUserSettings();
    AudioSettings = FAudioUserSettings();
    GraphicsSettings = FGraphicsUserSettings();
    GameplaySettings = FGameplayUserSettings();
}

bool UUserSettingsSaveModule::ValidateData_Implementation()
{
    // Clamp camera settings
    CameraSettings.FieldOfView = FMath::Clamp(CameraSettings.FieldOfView, 60.f, 120.f);
    CameraSettings.MouseSensitivity = FMath::Clamp(CameraSettings.MouseSensitivity, 0.1f, 5.0f);
    CameraSettings.HeadBobIntensity = FMath::Clamp(CameraSettings.HeadBobIntensity, 0.f, 1.f);
    
    // Clamp audio settings
    AudioSettings.MasterVolume = FMath::Clamp(AudioSettings.MasterVolume, 0.f, 1.f);
    AudioSettings.MusicVolume = FMath::Clamp(AudioSettings.MusicVolume, 0.f, 1.f);
    AudioSettings.SFXVolume = FMath::Clamp(AudioSettings.SFXVolume, 0.f, 1.f);
    AudioSettings.VoiceVolume = FMath::Clamp(AudioSettings.VoiceVolume, 0.f, 1.f);
    AudioSettings.AmbientVolume = FMath::Clamp(AudioSettings.AmbientVolume, 0.f, 1.f);
    
    // Clamp graphics settings
    GraphicsSettings.ResolutionX = FMath::Max(640, GraphicsSettings.ResolutionX);
    GraphicsSettings.ResolutionY = FMath::Max(480, GraphicsSettings.ResolutionY);
    GraphicsSettings.FrameRateLimit = FMath::Clamp(GraphicsSettings.FrameRateLimit, 30, 240);
    GraphicsSettings.ShadowQuality = FMath::Clamp(GraphicsSettings.ShadowQuality, 0, 4);
    GraphicsSettings.TextureQuality = FMath::Clamp(GraphicsSettings.TextureQuality, 0, 4);
    GraphicsSettings.EffectsQuality = FMath::Clamp(GraphicsSettings.EffectsQuality, 0, 4);
    GraphicsSettings.PostProcessQuality = FMath::Clamp(GraphicsSettings.PostProcessQuality, 0, 4);
    GraphicsSettings.Gamma = FMath::Clamp(GraphicsSettings.Gamma, 1.0f, 3.0f);
    
    return true;
}

void UUserSettingsSaveModule::MigrateData_Implementation(int32 FromVersion, int32 ToVersion)
{
    // Future migration logic
    // Example:
    // if (FromVersion < 2 && ToVersion >= 2)
    // {
    //     // Migrate from v1 to v2
    // }
}