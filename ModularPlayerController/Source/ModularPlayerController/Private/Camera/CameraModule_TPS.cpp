// CameraModule_TPS.cpp
// Windwalker Framework - Third Person Shoulder Camera Module

#include "Camera/CameraModule_TPS.h"
#include "WW_TagLibrary.h"

UCameraModule_TPS::UCameraModule_TPS()
{
    // === IDENTITY ===
    CameraModeTag = FWWTagLibrary::Camera_Mode_ThirdPerson();
    DisplayName = FText::FromString(TEXT("Third Person"));
    
    // === CAPABILITIES ===
    // Aim states
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_None());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Hip());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Aiming());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Scope());
    // Shoulder states
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Shoulder_Center());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Shoulder_Left());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Shoulder_Right());
    
    // === SPRING ARM CONFIG ===
    SpringArmConfig.DefaultArmLength = 300.f;
    SpringArmConfig.SocketOffset = FVector(0.f, 50.f, 50.f);
    SpringArmConfig.TargetOffset = FVector::ZeroVector;
    SpringArmConfig.bUsePawnControlRotation = true;
    SpringArmConfig.bEnableCameraLag = true;
    SpringArmConfig.CameraLagSpeed = 10.f;
    SpringArmConfig.bEnableCameraRotationLag = true;
    SpringArmConfig.CameraRotationLagSpeed = 10.f;
    
    // === ZOOM CONFIG ===
    ZoomConfig.MinArmLength = 100.f;
    ZoomConfig.MaxArmLength = 600.f;
    ZoomConfig.ZoomSpeed = 20.f;
    ZoomConfig.ZoomInterpSpeed = 10.f;
    
    // === AIM CONFIG ===
    // Hip fire
    AimConfig.HipFOV = 90.f;
    AimConfig.HipArmLength = 300.f;
    AimConfig.HipSocketOffset = FVector::ZeroVector;
    // ADS / Aiming
    AimConfig.AimingFOV = 70.f;
    AimConfig.AimingArmLength = 150.f;
    AimConfig.AimingSocketOffset = FVector(0.f, 20.f, 10.f);
    // Scope
    AimConfig.ScopeFOV = 40.f;
    AimConfig.ScopeArmLength = 80.f;
    AimConfig.ScopeSocketOffset = FVector(0.f, 10.f, 5.f);
    AimConfig.bScopeUsesRenderTarget = true;
    
    // === SHOULDER CONFIG ===
    ShoulderConfig.ShoulderOffsetY = 50.f;
    ShoulderConfig.CenterOffset = FVector::ZeroVector;
    
    // === INTERPOLATION CONFIG ===
    InterpConfig.FOVInterpSpeed = 10.f;
    InterpConfig.ArmLengthInterpSpeed = 10.f;
    InterpConfig.SocketOffsetInterpSpeed = 8.f;
    InterpConfig.ShoulderInterpSpeed = 12.f;
    
    // === CHARACTER CONFIG ===
    CharacterConfig.bUseControllerRotationYaw = false;
    CharacterConfig.bOrientRotationToMovement = true;
    
    // === BASE SETTINGS ===
    BaseFOV = 90.f;
    AttachSocketName = NAME_None;  // Uses SpringArm, not socket
}