// CameraModule_TopDown.cpp
// Windwalker Framework - Top Down Camera Module
// RTS/ARPG style - fixed angle, zoom, edge panning

#include "Camera/CameraModule_TopDown.h"
#include "WW_TagLibrary.h"

UCameraModule_TopDown::UCameraModule_TopDown()
{
    // === IDENTITY ===
    CameraModeTag = FWWTagLibrary::Camera_Mode_TopDown();
    DisplayName = FText::FromString(TEXT("Top Down"));
    
    // === CAPABILITIES ===
    // Only basic aim (for character state, not camera change)
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_None());
    SupportedCapabilities.AddTag(FWWTagLibrary::Camera_Aim_Aiming());  // Gameplay only
    // No shoulder - doesn't apply to top down
    // No hip/scope - not typical for this view
    
    // === SPRING ARM CONFIG (fixed overhead angle) ===
    SpringArmConfig.DefaultArmLength = 1200.f;  // High up
    SpringArmConfig.SocketOffset = FVector::ZeroVector;
    SpringArmConfig.TargetOffset = FVector(0.f, 0.f, 0.f);
    SpringArmConfig.bUsePawnControlRotation = false;  // Fixed angle, not mouse controlled
    SpringArmConfig.bEnableCameraLag = true;
    SpringArmConfig.CameraLagSpeed = 8.f;  // Smooth follow
    SpringArmConfig.bEnableCameraRotationLag = false;
    SpringArmConfig.CameraRotationLagSpeed = 0.f;
    
    // === ZOOM CONFIG (strategic zoom levels) ===
    ZoomConfig.MinArmLength = 600.f;   // Closest zoom
    ZoomConfig.MaxArmLength = 2000.f;  // Furthest zoom
    ZoomConfig.ZoomSpeed = 100.f;      // Faster zoom for RTS feel
    ZoomConfig.ZoomInterpSpeed = 6.f;
    
    // === AIM CONFIG (minimal - aim state affects gameplay, not camera) ===
    // All states use same camera settings
    AimConfig.HipFOV = 60.f;  // Slightly narrower for top-down
    AimConfig.HipArmLength = 1200.f;
    AimConfig.HipSocketOffset = FVector::ZeroVector;
    AimConfig.AimingFOV = 60.f;  // Same - aiming doesn't change camera
    AimConfig.AimingArmLength = 1200.f;
    AimConfig.AimingSocketOffset = FVector::ZeroVector;
    AimConfig.ScopeFOV = 60.f;
    AimConfig.ScopeArmLength = 1200.f;
    AimConfig.ScopeSocketOffset = FVector::ZeroVector;
    AimConfig.bScopeUsesRenderTarget = false;
    
    // === SHOULDER CONFIG (not used) ===
    ShoulderConfig.ShoulderOffsetY = 0.f;
    ShoulderConfig.CenterOffset = FVector::ZeroVector;
    
    // === INTERPOLATION CONFIG ===
    InterpConfig.FOVInterpSpeed = 5.f;
    InterpConfig.ArmLengthInterpSpeed = 6.f;  // Smooth zoom
    InterpConfig.SocketOffsetInterpSpeed = 5.f;
    InterpConfig.ShoulderInterpSpeed = 0.f;
    
    // === CHARACTER CONFIG ===
    // Top-down: Character rotates to face movement/cursor, not camera
    CharacterConfig.bUseControllerRotationYaw = false;
    CharacterConfig.bOrientRotationToMovement = true;
    
    // === BASE SETTINGS ===
    BaseFOV = 60.f;
    AttachSocketName = NAME_None;  // Uses SpringArm
}