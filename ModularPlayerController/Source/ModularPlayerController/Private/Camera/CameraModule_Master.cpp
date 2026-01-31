// CameraModule_Master.cpp
// Windwalker Framework - Base Camera Module Implementation

#include "Camera/CameraModule_Master.h"
#include "WW_TagLibrary.h"

UCameraModule_Master::UCameraModule_Master()
{
    CameraModeTag = FGameplayTag::EmptyTag;
    DisplayName = FText::FromString(TEXT("Base Camera Module"));
    BaseFOV = 90.f;
    AttachSocketName = NAME_None;
    bIsActive = false;
    
    // Initialize runtime state
    InitializeRuntimeState();
}

void UCameraModule_Master::InitializeRuntimeState()
{
    RuntimeState.TargetFOV = BaseFOV;
    RuntimeState.TargetArmLength = SpringArmConfig.DefaultArmLength;
    RuntimeState.TargetSocketOffset = SpringArmConfig.SocketOffset;
    
    RuntimeState.CurrentFOV = BaseFOV;
    RuntimeState.CurrentArmLength = SpringArmConfig.DefaultArmLength;
    RuntimeState.CurrentSocketOffset = SpringArmConfig.SocketOffset;
    
    RuntimeState.ZoomTargetArmLength = SpringArmConfig.DefaultArmLength;
    
    RuntimeState.CachedTransform = FTransform::Identity;
    RuntimeState.CachedArmLength = SpringArmConfig.DefaultArmLength;
    RuntimeState.bHasCachedTransform = false;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UCameraModule_Master::OnActivated()
{
    bIsActive = true;
    
    // Restore cached state if available
    if (RuntimeState.bHasCachedTransform)
    {
        RuntimeState.CurrentArmLength = RuntimeState.CachedArmLength;
        RuntimeState.ZoomTargetArmLength = RuntimeState.CachedArmLength;
    }
    else
    {
        RuntimeState.CurrentArmLength = SpringArmConfig.DefaultArmLength;
        RuntimeState.ZoomTargetArmLength = SpringArmConfig.DefaultArmLength;
    }
    
    // Reset aim/shoulder to defaults
    CurrentAimState = FWWTagLibrary::Camera_Aim_None();
    CurrentShoulderState = FWWTagLibrary::Camera_Shoulder_Center();
    
    RecalculateTargets();
    
    UE_LOG(LogTemp, Log, TEXT("CameraModule '%s' activated"), *DisplayName.ToString());
}

void UCameraModule_Master::OnDeactivated()
{
    bIsActive = false;
    OnZoomChanged.Clear();
    
    UE_LOG(LogTemp, Log, TEXT("CameraModule '%s' deactivated"), *DisplayName.ToString());
}

// ============================================================================
// STATE SETTERS
// ============================================================================

void UCameraModule_Master::SetAimState(FGameplayTag NewAimState)
{
    if (CurrentAimState == NewAimState) return;
    
    CurrentAimState = NewAimState;
    RecalculateTargets();
    
    UE_LOG(LogTemp, Verbose, TEXT("CameraModule '%s' aim state: %s"), 
        *DisplayName.ToString(), *NewAimState.ToString());
}

void UCameraModule_Master::SetShoulderState(FGameplayTag NewShoulderState)
{
    if (CurrentShoulderState == NewShoulderState) return;
    
    CurrentShoulderState = NewShoulderState;
    RecalculateTargets();
    
    UE_LOG(LogTemp, Verbose, TEXT("CameraModule '%s' shoulder state: %s"), 
        *DisplayName.ToString(), *NewShoulderState.ToString());
}

// ============================================================================
// ZOOM CONTROL
// ============================================================================

void UCameraModule_Master::AdjustZoom(float ScrollDelta)
{
    float OldTarget = RuntimeState.ZoomTargetArmLength;
    
    RuntimeState.ZoomTargetArmLength -= ScrollDelta * ZoomConfig.ZoomSpeed;
    RuntimeState.ZoomTargetArmLength = FMath::Clamp(
        RuntimeState.ZoomTargetArmLength, 
        ZoomConfig.MinArmLength, 
        ZoomConfig.MaxArmLength
    );
    
    // Recalculate targets with new zoom
    RecalculateTargets();
    
    // Broadcast if changed significantly
    if (!FMath::IsNearlyEqual(OldTarget, RuntimeState.ZoomTargetArmLength, 0.1f))
    {
        float ZoomPercent = (RuntimeState.ZoomTargetArmLength - ZoomConfig.MinArmLength) / 
                            (ZoomConfig.MaxArmLength - ZoomConfig.MinArmLength);
        OnZoomChanged.Broadcast(RuntimeState.ZoomTargetArmLength, ZoomPercent);
    }
}

void UCameraModule_Master::SetTargetArmLength(float NewLength)
{
    float OldTarget = RuntimeState.ZoomTargetArmLength;
    
    RuntimeState.ZoomTargetArmLength = FMath::Clamp(
        NewLength, 
        ZoomConfig.MinArmLength, 
        ZoomConfig.MaxArmLength
    );
    
    RecalculateTargets();
    
    if (!FMath::IsNearlyEqual(OldTarget, RuntimeState.ZoomTargetArmLength, 0.1f))
    {
        float ZoomPercent = (RuntimeState.ZoomTargetArmLength - ZoomConfig.MinArmLength) / 
                            (ZoomConfig.MaxArmLength - ZoomConfig.MinArmLength);
        OnZoomChanged.Broadcast(RuntimeState.ZoomTargetArmLength, ZoomPercent);
    }
}

void UCameraModule_Master::UpdateInterpolation(float DeltaTime)
{
    // FOV
    RuntimeState.CurrentFOV = FMath::FInterpTo(
        RuntimeState.CurrentFOV, 
        RuntimeState.TargetFOV, 
        DeltaTime, 
        InterpConfig.FOVInterpSpeed
    );
    
    // Arm Length
    RuntimeState.CurrentArmLength = FMath::FInterpTo(
        RuntimeState.CurrentArmLength, 
        RuntimeState.TargetArmLength, 
        DeltaTime, 
        InterpConfig.ArmLengthInterpSpeed
    );
    
    // Socket Offset
    RuntimeState.CurrentSocketOffset = FMath::VInterpTo(
        RuntimeState.CurrentSocketOffset, 
        RuntimeState.TargetSocketOffset, 
        DeltaTime, 
        InterpConfig.SocketOffsetInterpSpeed
    );
}

// ============================================================================
// TRANSFORM CACHE
// ============================================================================

void UCameraModule_Master::CacheTransform(const FTransform& InTransform, float InArmLength)
{
    RuntimeState.CachedTransform = InTransform;
    RuntimeState.CachedArmLength = InArmLength;
    RuntimeState.bHasCachedTransform = true;
    
    UE_LOG(LogTemp, Verbose, TEXT("CameraModule '%s' cached transform (ArmLength: %.1f)"), 
        *DisplayName.ToString(), InArmLength);
}

void UCameraModule_Master::GetCachedTransform(FTransform& OutTransform, float& OutArmLength) const
{
    OutTransform = RuntimeState.CachedTransform;
    OutArmLength = RuntimeState.CachedArmLength;
}

void UCameraModule_Master::ClearCachedTransform()
{
    RuntimeState.CachedTransform = FTransform::Identity;
    RuntimeState.CachedArmLength = SpringArmConfig.DefaultArmLength;
    RuntimeState.bHasCachedTransform = false;
}

// ============================================================================
// INTERNAL
// ============================================================================

void UCameraModule_Master::RecalculateTargets()
{
    // === BASE VALUES ===
    RuntimeState.TargetFOV = BaseFOV;
    RuntimeState.TargetArmLength = RuntimeState.ZoomTargetArmLength;
    RuntimeState.TargetSocketOffset = SpringArmConfig.SocketOffset;
    
    // === APPLY AIM MODIFIER ===
    if (CurrentAimState.MatchesTagExact(FWWTagLibrary::Camera_Aim_Hip()))
    {
        RuntimeState.TargetFOV = AimConfig.HipFOV;
        RuntimeState.TargetArmLength = FMath::Min(RuntimeState.TargetArmLength, AimConfig.HipArmLength);
        RuntimeState.TargetSocketOffset = RuntimeState.TargetSocketOffset + AimConfig.HipSocketOffset;
    }
    else if (CurrentAimState.MatchesTagExact(FWWTagLibrary::Camera_Aim_Aiming()))
    {
        RuntimeState.TargetFOV = AimConfig.AimingFOV;
        RuntimeState.TargetArmLength = FMath::Min(RuntimeState.TargetArmLength, AimConfig.AimingArmLength);
        RuntimeState.TargetSocketOffset = RuntimeState.TargetSocketOffset + AimConfig.AimingSocketOffset;
    }
    else if (CurrentAimState.MatchesTagExact(FWWTagLibrary::Camera_Aim_Scope()))
    {
        if (!AimConfig.bScopeUsesRenderTarget)
        {
            RuntimeState.TargetFOV = AimConfig.ScopeFOV;
        }
        RuntimeState.TargetArmLength = FMath::Min(RuntimeState.TargetArmLength, AimConfig.ScopeArmLength);
        RuntimeState.TargetSocketOffset = RuntimeState.TargetSocketOffset + AimConfig.ScopeSocketOffset;
    }
    // Camera.Aim.None = use base values (no modification)
    
    // === APPLY SHOULDER MODIFIER ===
    if (CurrentShoulderState.MatchesTagExact(FWWTagLibrary::Camera_Shoulder_Left()))
    {
        RuntimeState.TargetSocketOffset.Y = -ShoulderConfig.ShoulderOffsetY;
    }
    else if (CurrentShoulderState.MatchesTagExact(FWWTagLibrary::Camera_Shoulder_Right()))
    {
        RuntimeState.TargetSocketOffset.Y = ShoulderConfig.ShoulderOffsetY;
    }
    else // Center
    {
        RuntimeState.TargetSocketOffset.Y = ShoulderConfig.CenterOffset.Y;
    }
}