// CameraModule_Custom.cpp
// Windwalker Framework - Custom State-Driven Camera Module

#include "Camera/CameraModule_Custom.h"
#include "WW_TagLibrary.h"

UCameraModule_Custom::UCameraModule_Custom()
{
    // === IDENTITY ===
    CameraModeTag = FWWTagLibrary::Camera_Mode_Custom();
    DisplayName = FText::FromString(TEXT("Custom"));
    
    // === DEFAULT PRESET TAG ===
    DefaultPresetTag = FWWTagLibrary::Camera_Mode_Custom_Default();
    
    // === DEFAULT PRESETS (can be overridden in editor) ===
    
    // Default preset
    FCameraStatePreset DefaultPreset;
    DefaultPreset.StateTag = FWWTagLibrary::Camera_Mode_Custom_Default();
    DefaultPreset.DisplayName = FText::FromString(TEXT("Default"));
    DefaultPreset.Priority = 0;
    DefaultPreset.TransitionSpeed = 5.f;
    DefaultPreset.FOV = 90.f;
    DefaultPreset.ArmLength = 300.f;
    DefaultPreset.SocketOffset = FVector(0.f, 50.f, 50.f);
    DefaultPreset.bUseSpringArm = true;
    DefaultPreset.bEnableCameraLag = true;
    DefaultPreset.CameraLagSpeed = 10.f;
    DefaultPreset.bUsePawnControlRotation = true;
    DefaultPreset.bUseControllerRotationYaw = false;
    DefaultPreset.bOrientRotationToMovement = true;
    StatePresets.Add(DefaultPreset);
    
    // Killcam preset
    FCameraStatePreset KillcamPreset;
    KillcamPreset.StateTag = FWWTagLibrary::Camera_Mode_Custom_Killcam();
    KillcamPreset.DisplayName = FText::FromString(TEXT("Killcam"));
    KillcamPreset.Priority = 10;
    KillcamPreset.TransitionSpeed = 8.f;
    KillcamPreset.FOV = 60.f;
    KillcamPreset.ArmLength = 200.f;
    KillcamPreset.SocketOffset = FVector(0.f, 100.f, 30.f);
    KillcamPreset.bUseSpringArm = true;
    KillcamPreset.bEnableCameraLag = false;
    KillcamPreset.bUsePawnControlRotation = false;
    KillcamPreset.bLockRotationToPawn = false;
    KillcamPreset.bUseControllerRotationYaw = false;
    KillcamPreset.bOrientRotationToMovement = false;
    StatePresets.Add(KillcamPreset);
    
    // Stealth preset
    FCameraStatePreset StealthPreset;
    StealthPreset.StateTag = FWWTagLibrary::Camera_Mode_Custom_Stealth();
    StealthPreset.DisplayName = FText::FromString(TEXT("Stealth"));
    StealthPreset.Priority = 5;
    StealthPreset.TransitionSpeed = 3.f;
    StealthPreset.FOV = 75.f;
    StealthPreset.ArmLength = 250.f;
    StealthPreset.SocketOffset = FVector(0.f, 40.f, 80.f);
    StealthPreset.RotationOffset = FRotator(-20.f, 0.f, 0.f);
    StealthPreset.bUseSpringArm = true;
    StealthPreset.bEnableCameraLag = true;
    StealthPreset.CameraLagSpeed = 5.f;
    StealthPreset.bUsePawnControlRotation = true;
    StealthPreset.bUseControllerRotationYaw = false;
    StealthPreset.bOrientRotationToMovement = true;
    StatePresets.Add(StealthPreset);
    
    // Cinematic preset
    FCameraStatePreset CinematicPreset;
    CinematicPreset.StateTag = FWWTagLibrary::Camera_Mode_Custom_Cinematic();
    CinematicPreset.DisplayName = FText::FromString(TEXT("Cinematic"));
    CinematicPreset.Priority = 100;
    CinematicPreset.TransitionSpeed = 2.f;
    CinematicPreset.FOV = 50.f;
    CinematicPreset.ArmLength = 400.f;
    CinematicPreset.SocketOffset = FVector(0.f, 0.f, 100.f);
    CinematicPreset.bUseSpringArm = true;
    CinematicPreset.bEnableCameraLag = true;
    CinematicPreset.CameraLagSpeed = 2.f;
    CinematicPreset.bEnableCameraRotationLag = true;
    CinematicPreset.CameraRotationLagSpeed = 2.f;
    CinematicPreset.bUsePawnControlRotation = false;
    CinematicPreset.bLockRotationToPawn = true;
    CinematicPreset.bUseControllerRotationYaw = false;
    CinematicPreset.bOrientRotationToMovement = false;
    StatePresets.Add(CinematicPreset);
    
    // Populate capabilities from presets
    PopulateSupportedCapabilities();
    
    // === BASE CONFIG (fallback if no preset active) ===
    SpringArmConfig.DefaultArmLength = 300.f;
    SpringArmConfig.SocketOffset = FVector(0.f, 50.f, 50.f);
    SpringArmConfig.bUsePawnControlRotation = true;
    SpringArmConfig.bEnableCameraLag = true;
    SpringArmConfig.CameraLagSpeed = 10.f;
    
    ZoomConfig.MinArmLength = 50.f;
    ZoomConfig.MaxArmLength = 800.f;
    ZoomConfig.ZoomSpeed = 20.f;
    ZoomConfig.ZoomInterpSpeed = 10.f;
    
    InterpConfig.FOVInterpSpeed = 5.f;
    InterpConfig.ArmLengthInterpSpeed = 5.f;
    InterpConfig.SocketOffsetInterpSpeed = 5.f;
    
    CharacterConfig.bUseControllerRotationYaw = false;
    CharacterConfig.bOrientRotationToMovement = true;
    
    BaseFOV = 90.f;
    AttachSocketName = NAME_None;
}

void UCameraModule_Custom::OnActivated()
{
    // Apply default preset on activation
    if (DefaultPresetTag.IsValid())
    {
        SetActivePreset(DefaultPresetTag);
    }
    
    Super::OnActivated();
}

void UCameraModule_Custom::PopulateSupportedCapabilities()
{
    SupportedCapabilities.Reset();
    
    // Add all preset tags as supported capabilities
    for (const FCameraStatePreset& Preset : StatePresets)
    {
        if (Preset.StateTag.IsValid())
        {
            SupportedCapabilities.AddTag(Preset.StateTag);
        }
    }
    
    // Also support base aim states for compatibility
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_None());
}

bool UCameraModule_Custom::SetActivePreset(FGameplayTag PresetTag)
{
    if (!PresetTag.IsValid()) return false;
    if (ActivePresetTag == PresetTag) return true;
    
    // Find preset
    for (int32 i = 0; i < StatePresets.Num(); ++i)
    {
        if (StatePresets[i].StateTag.MatchesTagExact(PresetTag))
        {
            ActivePresetTag = PresetTag;
            ActivePresetIndex = i;
            ApplyPreset(StatePresets[i]);
            
            UE_LOG(LogTemp, Log, TEXT("CameraModule_Custom: Activated preset '%s'"), 
                *StatePresets[i].DisplayName.ToString());
            return true;
        }
    }
    
    // Try hierarchical match
    for (int32 i = 0; i < StatePresets.Num(); ++i)
    {
        if (PresetTag.MatchesTag(StatePresets[i].StateTag))
        {
            ActivePresetTag = StatePresets[i].StateTag;
            ActivePresetIndex = i;
            ApplyPreset(StatePresets[i]);
            
            UE_LOG(LogTemp, Log, TEXT("CameraModule_Custom: Activated preset '%s' (hierarchical match)"), 
                *StatePresets[i].DisplayName.ToString());
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CameraModule_Custom: Preset not found for tag '%s'"), 
        *PresetTag.ToString());
    return false;
}

const FCameraStatePreset* UCameraModule_Custom::GetActivePreset() const
{
    if (ActivePresetIndex != INDEX_NONE && StatePresets.IsValidIndex(ActivePresetIndex))
    {
        return &StatePresets[ActivePresetIndex];
    }
    return nullptr;
}

const FCameraStatePreset* UCameraModule_Custom::FindPresetByTag(FGameplayTag PresetTag) const
{
    for (const FCameraStatePreset& Preset : StatePresets)
    {
        if (Preset.StateTag.MatchesTagExact(PresetTag))
        {
            return &Preset;
        }
    }
    return nullptr;
}

void UCameraModule_Custom::ApplyPreset(const FCameraStatePreset& Preset)
{
    // Update SpringArm config
    SpringArmConfig.DefaultArmLength = Preset.ArmLength;
    SpringArmConfig.SocketOffset = Preset.SocketOffset;
    SpringArmConfig.TargetOffset = Preset.TargetOffset;
    SpringArmConfig.bUsePawnControlRotation = Preset.bUsePawnControlRotation;
    SpringArmConfig.bEnableCameraLag = Preset.bEnableCameraLag;
    SpringArmConfig.CameraLagSpeed = Preset.CameraLagSpeed;
    SpringArmConfig.bEnableCameraRotationLag = Preset.bEnableCameraRotationLag;
    SpringArmConfig.CameraRotationLagSpeed = Preset.CameraRotationLagSpeed;
    
    // Update character config
    CharacterConfig.bUseControllerRotationYaw = Preset.bUseControllerRotationYaw;
    CharacterConfig.bOrientRotationToMovement = Preset.bOrientRotationToMovement;
    
    // Update interpolation speed from preset
    InterpConfig.FOVInterpSpeed = Preset.TransitionSpeed;
    InterpConfig.ArmLengthInterpSpeed = Preset.TransitionSpeed;
    InterpConfig.SocketOffsetInterpSpeed = Preset.TransitionSpeed;
    
    // Update base values
    BaseFOV = Preset.FOV;
    AttachSocketName = Preset.AttachSocket;
    
    // Update runtime targets directly
    RuntimeState.TargetFOV = Preset.FOV;
    RuntimeState.TargetArmLength = Preset.ArmLength;
    RuntimeState.TargetSocketOffset = Preset.SocketOffset;
    RuntimeState.ZoomTargetArmLength = Preset.ArmLength;
}