// CameraModule_TrueFPS.cpp
// Windwalker Framework - True First Person Camera Module
// Camera attached to skeleton head, sees own body, procedural sway

#include "Camera/CameraModule_TrueFPS.h"
#include "WW_TagLibrary.h"

UCameraModule_TrueFPS::UCameraModule_TrueFPS()
{
    // === IDENTITY ===
    CameraModeTag = FWWTagLibrary::Camera_Mode_TrueFPS();
    DisplayName = FText::FromString(TEXT("True First Person"));
    
    // === CAPABILITIES ===
    // Aim states (no shoulder)
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_None());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Hip());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Aiming());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Scope());
    
    // === SPRING ARM CONFIG (disabled - TrueFPS uses socket) ===
    SpringArmConfig.DefaultArmLength = 0.f;
    SpringArmConfig.SocketOffset = FVector::ZeroVector;
    SpringArmConfig.TargetOffset = FVector::ZeroVector;
    SpringArmConfig.bUsePawnControlRotation = false;  // Head bone controls rotation
    SpringArmConfig.bEnableCameraLag = false;
    SpringArmConfig.CameraLagSpeed = 0.f;
    SpringArmConfig.bEnableCameraRotationLag = false;
    SpringArmConfig.CameraRotationLagSpeed = 0.f;
    
    // === ZOOM CONFIG (FOV only) ===
    ZoomConfig.MinArmLength = 0.f;
    ZoomConfig.MaxArmLength = 0.f;
    ZoomConfig.ZoomSpeed = 0.f;
    ZoomConfig.ZoomInterpSpeed = 10.f;
    
    // === AIM CONFIG ===
    // Hip fire
    AimConfig.HipFOV = 90.f;
    AimConfig.HipArmLength = 0.f;
    AimConfig.HipSocketOffset = FVector::ZeroVector;
    // ADS / Aiming
    AimConfig.AimingFOV = 60.f;
    AimConfig.AimingArmLength = 0.f;
    AimConfig.AimingSocketOffset = FVector(5.f, 0.f, 0.f);  // Slight forward offset for ADS
    // Scope
    AimConfig.ScopeFOV = 25.f;
    AimConfig.ScopeArmLength = 0.f;
    AimConfig.ScopeSocketOffset = FVector(8.f, 0.f, 0.f);
    AimConfig.bScopeUsesRenderTarget = true;
    
    // === SHOULDER CONFIG (not used) ===
    ShoulderConfig.ShoulderOffsetY = 0.f;
    ShoulderConfig.CenterOffset = FVector::ZeroVector;
    
    // === INTERPOLATION CONFIG ===
    InterpConfig.FOVInterpSpeed = 12.f;
    InterpConfig.ArmLengthInterpSpeed = 0.f;
    InterpConfig.SocketOffsetInterpSpeed = 15.f;  // Fast offset for weapon sway
    InterpConfig.ShoulderInterpSpeed = 0.f;
    
    // === CHARACTER CONFIG ===
    // TrueFPS: Character rotates with look, but head bone drives camera
    CharacterConfig.bUseControllerRotationYaw = true;
    CharacterConfig.bOrientRotationToMovement = false;
    
    // === BASE SETTINGS ===
    BaseFOV = 90.f;
    AttachSocketName = FName("FP_Camera");  // Dedicated FP camera socket on skeleton
}