// CameraModule_FPS.cpp
// Windwalker Framework - Classic First Person Camera Module

#include "Camera/CameraModule_FPS.h"
#include "WW_TagLibrary.h"

UCameraModule_FPS::UCameraModule_FPS()
{
    // === IDENTITY ===
    CameraModeTag = FWWTagLibrary::Camera_Mode_FirstPerson();
    DisplayName = FText::FromString(TEXT("First Person"));
    
    // === CAPABILITIES ===
    // Aim states (no shoulder - FPS doesn't need it)
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_None());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Hip());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Aiming());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Scope());
    
    // === SPRING ARM CONFIG (disabled - FPS uses socket attachment) ===
    SpringArmConfig.DefaultArmLength = 0.f;  // 0 = no SpringArm
    SpringArmConfig.SocketOffset = FVector::ZeroVector;
    SpringArmConfig.TargetOffset = FVector::ZeroVector;
    SpringArmConfig.bUsePawnControlRotation = true;
    SpringArmConfig.bEnableCameraLag = false;
    SpringArmConfig.CameraLagSpeed = 0.f;
    SpringArmConfig.bEnableCameraRotationLag = false;
    SpringArmConfig.CameraRotationLagSpeed = 0.f;
    
    // === ZOOM CONFIG (FOV zoom only, no arm length) ===
    ZoomConfig.MinArmLength = 0.f;
    ZoomConfig.MaxArmLength = 0.f;
    ZoomConfig.ZoomSpeed = 0.f;
    ZoomConfig.ZoomInterpSpeed = 10.f;
    
    // === AIM CONFIG ===
    // Hip fire - normal view
    AimConfig.HipFOV = 90.f;
    AimConfig.HipArmLength = 0.f;
    AimConfig.HipSocketOffset = FVector::ZeroVector;
    // ADS / Aiming - tighter FOV
    AimConfig.AimingFOV = 65.f;
    AimConfig.AimingArmLength = 0.f;
    AimConfig.AimingSocketOffset = FVector::ZeroVector;
    // Scope - render target or extreme FOV
    AimConfig.ScopeFOV = 30.f;
    AimConfig.ScopeArmLength = 0.f;
    AimConfig.ScopeSocketOffset = FVector::ZeroVector;
    AimConfig.bScopeUsesRenderTarget = true;
    
    // === SHOULDER CONFIG (not used) ===
    ShoulderConfig.ShoulderOffsetY = 0.f;
    ShoulderConfig.CenterOffset = FVector::ZeroVector;
    
    // === INTERPOLATION CONFIG ===
    InterpConfig.FOVInterpSpeed = 15.f;  // Fast FOV transitions
    InterpConfig.ArmLengthInterpSpeed = 0.f;
    InterpConfig.SocketOffsetInterpSpeed = 0.f;
    InterpConfig.ShoulderInterpSpeed = 0.f;
    
    // === CHARACTER CONFIG ===
    // FPS: Character rotates with camera
    CharacterConfig.bUseControllerRotationYaw = true;
    CharacterConfig.bOrientRotationToMovement = false;
    
    // === BASE SETTINGS ===
    BaseFOV = 90.f;
    AttachSocketName = FName("head");  // Attach to head socket
}