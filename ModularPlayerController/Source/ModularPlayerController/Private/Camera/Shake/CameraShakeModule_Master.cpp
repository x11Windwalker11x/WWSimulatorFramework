// CameraShakeModule_Master.cpp
// Windwalker Framework - Base Camera Shake Module Implementation

#include "Camera/Shake/CameraShakeModule_Master.h"
#include "Camera/Manager/MPC_PlayerCameraManager.h"

UCameraShakeModule_Master::UCameraShakeModule_Master()
{
    ModuleTag = FGameplayTag::EmptyTag;
    DisplayName = FText::FromString(TEXT("Base Shake Module"));
    NextInstanceID = 1;
    OwnerManager = nullptr;
}

void UCameraShakeModule_Master::Initialize(AMPC_PlayerCameraManager* InManager)
{
    OwnerManager = InManager;
    PopulateSupportedShakes();
    
    UE_LOG(LogTemp, Log, TEXT("CameraShakeModule '%s' initialized with %d presets"), 
        *DisplayName.ToString(), ShakePresets.Num());
}

void UCameraShakeModule_Master::Deinitialize()
{
    StopAllShakes(true);
    OwnerManager = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("CameraShakeModule '%s' deinitialized"), *DisplayName.ToString());
}

// ============================================================================
// MAIN API
// ============================================================================

int32 UCameraShakeModule_Master::PlayShake(FGameplayTag ShakeTag, float Scale)
{
    if (!ShakeTag.IsValid()) return -1;
    
    int32 PresetIndex = FindPresetIndexByTag(ShakeTag);
    if (PresetIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraShakeModule: Preset not found for tag '%s'"), 
            *ShakeTag.ToString());
        return -1;
    }
    
    const FCameraShakePreset& Preset = ShakePresets[PresetIndex];
    
    // Check if already playing and doesn't allow multiple instances
    if (!Preset.bAllowMultipleInstances && IsShakeActive(ShakeTag))
    {
        return -1;
    }
    
    // Create new instance
    FCameraShakeInstance NewInstance;
    NewInstance.ShakeTag = ShakeTag;
    NewInstance.InstanceID = NextInstanceID++;
    NewInstance.PresetIndex = PresetIndex;
    NewInstance.CurrentTime = 0.f;
    NewInstance.CurrentScale = Scale * Preset.Scale;
    NewInstance.bIsActive = true;
    NewInstance.bIsBlendingOut = false;
    NewInstance.bHasSourceLocation = false;
    
    ActiveInstances.Add(NewInstance);
    
    // Broadcast
    OnShakeStarted.Broadcast(ShakeTag, NewInstance.InstanceID);
    
    UE_LOG(LogTemp, Verbose, TEXT("CameraShakeModule: Started shake '%s' (ID: %d, Scale: %.2f)"), 
        *Preset.DisplayName.ToString(), NewInstance.InstanceID, NewInstance.CurrentScale);
    
    return NewInstance.InstanceID;
}

int32 UCameraShakeModule_Master::PlayShakeAtLocation(FGameplayTag ShakeTag, FVector SourceLocation, float Scale)
{
    int32 InstanceID = PlayShake(ShakeTag, Scale);
    
    if (InstanceID != -1)
    {
        // Find the instance we just created and set location
        for (FCameraShakeInstance& Instance : ActiveInstances)
        {
            if (Instance.InstanceID == InstanceID)
            {
                Instance.SourceLocation = SourceLocation;
                Instance.bHasSourceLocation = true;
                break;
            }
        }
    }
    
    return InstanceID;
}

void UCameraShakeModule_Master::StopShake(int32 InstanceID, bool bImmediate)
{
    for (FCameraShakeInstance& Instance : ActiveInstances)
    {
        if (Instance.InstanceID == InstanceID && Instance.bIsActive)
        {
            if (bImmediate)
            {
                Instance.bIsActive = false;
                OnShakeEnded.Broadcast(Instance.ShakeTag, InstanceID);
            }
            else
            {
                Instance.bIsBlendingOut = true;
            }
            return;
        }
    }
}

void UCameraShakeModule_Master::StopShakeByTag(FGameplayTag ShakeTag, bool bImmediate)
{
    for (FCameraShakeInstance& Instance : ActiveInstances)
    {
        if (Instance.ShakeTag.MatchesTagExact(ShakeTag) && Instance.bIsActive)
        {
            if (bImmediate)
            {
                Instance.bIsActive = false;
                OnShakeEnded.Broadcast(Instance.ShakeTag, Instance.InstanceID);
            }
            else
            {
                Instance.bIsBlendingOut = true;
            }
        }
    }
}

void UCameraShakeModule_Master::StopAllShakes(bool bImmediate)
{
    for (FCameraShakeInstance& Instance : ActiveInstances)
    {
        if (Instance.bIsActive)
        {
            if (bImmediate)
            {
                Instance.bIsActive = false;
                OnShakeEnded.Broadcast(Instance.ShakeTag, Instance.InstanceID);
            }
            else
            {
                Instance.bIsBlendingOut = true;
            }
        }
    }
    
    if (bImmediate)
    {
        ActiveInstances.Empty();
    }
}

// ============================================================================
// UPDATE
// ============================================================================

FCameraShakeOutput UCameraShakeModule_Master::UpdateShakes(float DeltaTime, const FVector& CameraLocation)
{
    FCameraShakeOutput CombinedOutput;
    
    for (FCameraShakeInstance& Instance : ActiveInstances)
    {
        if (!Instance.bIsActive) continue;
        
        if (!ShakePresets.IsValidIndex(Instance.PresetIndex)) continue;
        
        const FCameraShakePreset& Preset = ShakePresets[Instance.PresetIndex];
        
        // Update time
        Instance.CurrentTime += DeltaTime;
        
        // Check if finished (non-looping)
        if (!Preset.bLooping && !Instance.bIsBlendingOut)
        {
            float TotalDuration = Preset.Duration;
            if (Instance.CurrentTime >= TotalDuration)
            {
                Instance.bIsBlendingOut = true;
                Instance.CurrentTime = TotalDuration;
            }
        }
        
        // Calculate output for this instance
        FCameraShakeOutput InstanceOutput = CalculateShakeOutput(Instance, Preset, CameraLocation);
        
        // Accumulate
        CombinedOutput.LocationOffset += InstanceOutput.LocationOffset;
        CombinedOutput.RotationOffset += InstanceOutput.RotationOffset;
        CombinedOutput.FOVOffset += InstanceOutput.FOVOffset;
        
        // Check if blend out complete
        if (Instance.bIsBlendingOut)
        {
            float BlendOutProgress = (Instance.CurrentTime - Preset.Duration) / Preset.BlendOutTime;
            if (BlendOutProgress >= 1.f)
            {
                Instance.bIsActive = false;
                OnShakeEnded.Broadcast(Instance.ShakeTag, Instance.InstanceID);
            }
        }
    }
    
    // Cleanup finished instances
    CleanupFinishedInstances();
    
    return CombinedOutput;
}

// ============================================================================
// GETTERS
// ============================================================================

int32 UCameraShakeModule_Master::GetActiveShakeCount() const
{
    int32 Count = 0;
    for (const FCameraShakeInstance& Instance : ActiveInstances)
    {
        if (Instance.bIsActive) Count++;
    }
    return Count;
}

bool UCameraShakeModule_Master::IsShakeActive(FGameplayTag ShakeTag) const
{
    for (const FCameraShakeInstance& Instance : ActiveInstances)
    {
        if (Instance.bIsActive && Instance.ShakeTag.MatchesTagExact(ShakeTag))
        {
            return true;
        }
    }
    return false;
}

// ============================================================================
// INTERNAL
// ============================================================================

const FCameraShakePreset* UCameraShakeModule_Master::FindPresetByTag(FGameplayTag ShakeTag) const
{
    for (const FCameraShakePreset& Preset : ShakePresets)
    {
        if (Preset.ShakeTag.MatchesTagExact(ShakeTag))
        {
            return &Preset;
        }
    }
    return nullptr;
}

int32 UCameraShakeModule_Master::FindPresetIndexByTag(FGameplayTag ShakeTag) const
{
    for (int32 i = 0; i < ShakePresets.Num(); ++i)
    {
        if (ShakePresets[i].ShakeTag.MatchesTagExact(ShakeTag))
        {
            return i;
        }
    }
    
    // Try hierarchical match
    for (int32 i = 0; i < ShakePresets.Num(); ++i)
    {
        if (ShakeTag.MatchesTag(ShakePresets[i].ShakeTag))
        {
            return i;
        }
    }
    
    return INDEX_NONE;
}

void UCameraShakeModule_Master::PopulateSupportedShakes()
{
    SupportedShakes.Reset();
    
    for (const FCameraShakePreset& Preset : ShakePresets)
    {
        if (Preset.ShakeTag.IsValid())
        {
            SupportedShakes.AddTag(Preset.ShakeTag);
        }
    }
}

FCameraShakeOutput UCameraShakeModule_Master::CalculateShakeOutput(
    const FCameraShakeInstance& Instance, 
    const FCameraShakePreset& Preset, 
    const FVector& CameraLocation)
{
    FCameraShakeOutput Output;
    
    float BlendAlpha = CalculateBlendAlpha(Instance, Preset);
    float DistanceScale = CalculateDistanceScale(Instance, Preset, CameraLocation);
    float FinalScale = Instance.CurrentScale * BlendAlpha * DistanceScale;
    
    if (FinalScale <= KINDA_SMALL_NUMBER)
    {
        return Output;
    }
    
    float Time = Instance.CurrentTime;
    
    // Location oscillation
    Output.LocationOffset.X = CalculateOscillation(Preset.LocationX, Time, FinalScale);
    Output.LocationOffset.Y = CalculateOscillation(Preset.LocationY, Time, FinalScale);
    Output.LocationOffset.Z = CalculateOscillation(Preset.LocationZ, Time, FinalScale);
    
    // Rotation oscillation
    Output.RotationOffset.Pitch = CalculateOscillation(Preset.RotationPitch, Time, FinalScale);
    Output.RotationOffset.Yaw = CalculateOscillation(Preset.RotationYaw, Time, FinalScale);
    Output.RotationOffset.Roll = CalculateOscillation(Preset.RotationRoll, Time, FinalScale);
    
    // FOV oscillation
    Output.FOVOffset = CalculateOscillation(Preset.FOV, Time, FinalScale);
    
    return Output;
}

float UCameraShakeModule_Master::CalculateOscillation(const FCameraShakeOscillation& Osc, float Time, float BlendAlpha)
{
    if (Osc.Amplitude <= KINDA_SMALL_NUMBER || Osc.Frequency <= KINDA_SMALL_NUMBER)
    {
        return 0.f;
    }
    
    // Perlin noise-like oscillation using sin with phase offset
    float Phase1 = Time * Osc.Frequency * 2.f * PI;
    float Phase2 = Time * Osc.Frequency * 1.3f * 2.f * PI + 0.5f;  // Slightly different frequency
    
    float Value = FMath::Sin(Phase1) * 0.7f + FMath::Sin(Phase2) * 0.3f;
    
    return Value * Osc.Amplitude * BlendAlpha;
}

float UCameraShakeModule_Master::CalculateBlendAlpha(const FCameraShakeInstance& Instance, const FCameraShakePreset& Preset)
{
    float Time = Instance.CurrentTime;
    
    // Blend in
    if (Time < Preset.BlendInTime && Preset.BlendInTime > 0.f)
    {
        return FMath::Clamp(Time / Preset.BlendInTime, 0.f, 1.f);
    }
    
    // Blend out
    if (Instance.bIsBlendingOut && Preset.BlendOutTime > 0.f)
    {
        float BlendOutStart = Preset.Duration;
        float BlendOutProgress = (Time - BlendOutStart) / Preset.BlendOutTime;
        return FMath::Clamp(1.f - BlendOutProgress, 0.f, 1.f);
    }
    
    return 1.f;
}

float UCameraShakeModule_Master::CalculateDistanceScale(
    const FCameraShakeInstance& Instance, 
    const FCameraShakePreset& Preset, 
    const FVector& CameraLocation)
{
    if (!Preset.bScaleWithDistance || !Instance.bHasSourceLocation)
    {
        return 1.f;
    }
    
    float Distance = FVector::Dist(CameraLocation, Instance.SourceLocation);
    
    if (Distance <= Preset.InnerRadius)
    {
        return 1.f;
    }
    
    if (Distance >= Preset.OuterRadius)
    {
        return 0.f;
    }
    
    // Linear falloff between inner and outer
    float Range = Preset.OuterRadius - Preset.InnerRadius;
    return 1.f - ((Distance - Preset.InnerRadius) / Range);
}

void UCameraShakeModule_Master::CleanupFinishedInstances()
{
    ActiveInstances.RemoveAll([](const FCameraShakeInstance& Instance)
    {
        return !Instance.bIsActive;
    });
}