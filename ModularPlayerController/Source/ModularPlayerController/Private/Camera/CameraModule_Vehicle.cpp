// CameraModule_Vehicle.cpp
// Windwalker Framework - Vehicle/Racing Camera Module
// Chase cam with rotation lag and speed-responsive behavior

#include "Camera/CameraModule_Vehicle.h"
#include "WW_TagLibrary.h"

UCameraModule_Vehicle::UCameraModule_Vehicle()
{
    // === IDENTITY ===
    CameraModeTag = FWWTagLibrary::Camera_Mode_Vehicle();
    DisplayName = FText::FromString(TEXT("Vehicle"));
    
    // === CAPABILITIES ===
    // Basic aim for turrets/weapons
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_None());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Hip());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Aiming());
    // No shoulder - not applicable to vehicles
    // No scope - vehicles typically don't scope
    
    // === SPRING ARM CONFIG (Chase Cam) ===
    SpringArmConfig.DefaultArmLength = 600.f;           // Further back
    SpringArmConfig.SocketOffset = FVector(0.f, 0.f, 200.f);  // High up
    SpringArmConfig.TargetOffset = FVector(0.f, 0.f, 50.f);   // Look at vehicle top
    SpringArmConfig.bUsePawnControlRotation = false;    // Follows vehicle, not mouse
    SpringArmConfig.bEnableCameraLag = true;
    SpringArmConfig.CameraLagSpeed = 4.f;               // Slow lag for cinematic
    SpringArmConfig.bEnableCameraRotationLag = true;    // Key for chase cam feel
    SpringArmConfig.CameraRotationLagSpeed = 4.f;       // Slow rotation catch-up
    
    // === ZOOM CONFIG ===
    ZoomConfig.MinArmLength = 300.f;
    ZoomConfig.MaxArmLength = 1200.f;
    ZoomConfig.ZoomSpeed = 40.f;
    ZoomConfig.ZoomInterpSpeed = 6.f;
    
    // === AIM CONFIG (for vehicle weapons) ===
    // Hip - normal driving
    AimConfig.HipFOV = 90.f;
    AimConfig.HipArmLength = 600.f;
    AimConfig.HipSocketOffset = FVector::ZeroVector;
    // Aiming - tighter for targeting
    AimConfig.AimingFOV = 70.f;
    AimConfig.AimingArmLength = 400.f;
    AimConfig.AimingSocketOffset = FVector(0.f, 0.f, 30.f);
    // Scope - not typically used
    AimConfig.ScopeFOV = 50.f;
    AimConfig.ScopeArmLength = 300.f;
    AimConfig.ScopeSocketOffset = FVector::ZeroVector;
    AimConfig.bScopeUsesRenderTarget = false;
    
    // === SHOULDER CONFIG (not used) ===
    ShoulderConfig.ShoulderOffsetY = 0.f;
    ShoulderConfig.CenterOffset = FVector::ZeroVector;
    
    // === INTERPOLATION CONFIG ===
    InterpConfig.FOVInterpSpeed = 5.f;
    InterpConfig.ArmLengthInterpSpeed = 5.f;
    InterpConfig.SocketOffsetInterpSpeed = 5.f;
    InterpConfig.ShoulderInterpSpeed = 0.f;
    
    // === CHARACTER/VEHICLE CONFIG ===
    // Vehicle controls its own rotation entirely
    CharacterConfig.bUseControllerRotationYaw = false;
    CharacterConfig.bOrientRotationToMovement = false;
    
    // === BASE SETTINGS ===
    BaseFOV = 90.f;
    AttachSocketName = NAME_None;  // Uses SpringArm
}