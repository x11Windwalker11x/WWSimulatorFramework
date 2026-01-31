// CameraShakeModule_Combat.cpp
// Windwalker Framework - Combat Camera Shake Module

#include "Camera/Shake/CameraShakeModule_Combat.h"
#include "WW_TagLibrary.h"

UCameraShakeModule_Combat::UCameraShakeModule_Combat()
{
    // === IDENTITY ===
    ModuleTag = FWWTagLibrary::CameraShake_Combat();
    DisplayName = FText::FromString(TEXT("Combat Shakes"));
    
    // ========================================================================
    // EXPLOSION - Large, violent shake
    // ========================================================================
    {
        FCameraShakePreset Explosion;
        Explosion.ShakeTag = FWWTagLibrary::CameraShake_Combat_Explosion();
        Explosion.DisplayName = FText::FromString(TEXT("Explosion"));
        Explosion.Priority = 10;
        Explosion.Duration = 0.6f;
        Explosion.BlendInTime = 0.05f;
        Explosion.BlendOutTime = 0.3f;
        Explosion.Scale = 1.f;
        Explosion.bScaleWithDistance = true;
        Explosion.InnerRadius = 200.f;
        Explosion.OuterRadius = 2000.f;
        Explosion.bAllowMultipleInstances = true;
        
        // Strong location shake
        Explosion.LocationX.Amplitude = 15.f;
        Explosion.LocationX.Frequency = 20.f;
        Explosion.LocationY.Amplitude = 10.f;
        Explosion.LocationY.Frequency = 18.f;
        Explosion.LocationZ.Amplitude = 20.f;
        Explosion.LocationZ.Frequency = 22.f;
        
        // Rotation shake
        Explosion.RotationPitch.Amplitude = 3.f;
        Explosion.RotationPitch.Frequency = 15.f;
        Explosion.RotationYaw.Amplitude = 2.f;
        Explosion.RotationYaw.Frequency = 12.f;
        Explosion.RotationRoll.Amplitude = 4.f;
        Explosion.RotationRoll.Frequency = 18.f;
        
        // FOV punch
        Explosion.FOV.Amplitude = 5.f;
        Explosion.FOV.Frequency = 8.f;
        
        ShakePresets.Add(Explosion);
    }
    
    // ========================================================================
    // GRENADE - Medium explosion shake
    // ========================================================================
    {
        FCameraShakePreset Grenade;
        Grenade.ShakeTag = FWWTagLibrary::CameraShake_Combat_Grenade();
        Grenade.DisplayName = FText::FromString(TEXT("Grenade"));
        Grenade.Priority = 8;
        Grenade.Duration = 0.4f;
        Grenade.BlendInTime = 0.03f;
        Grenade.BlendOutTime = 0.25f;
        Grenade.Scale = 1.f;
        Grenade.bScaleWithDistance = true;
        Grenade.InnerRadius = 150.f;
        Grenade.OuterRadius = 1500.f;
        Grenade.bAllowMultipleInstances = true;
        
        Grenade.LocationX.Amplitude = 8.f;
        Grenade.LocationX.Frequency = 25.f;
        Grenade.LocationY.Amplitude = 6.f;
        Grenade.LocationY.Frequency = 22.f;
        Grenade.LocationZ.Amplitude = 12.f;
        Grenade.LocationZ.Frequency = 28.f;
        
        Grenade.RotationPitch.Amplitude = 2.f;
        Grenade.RotationPitch.Frequency = 20.f;
        Grenade.RotationRoll.Amplitude = 2.5f;
        Grenade.RotationRoll.Frequency = 22.f;
        
        Grenade.FOV.Amplitude = 3.f;
        Grenade.FOV.Frequency = 10.f;
        
        ShakePresets.Add(Grenade);
    }
    
    // ========================================================================
    // GUNFIRE - Short, snappy shake for shooting
    // ========================================================================
    {
        FCameraShakePreset Gunfire;
        Gunfire.ShakeTag = FWWTagLibrary::CameraShake_Combat_Gunfire();
        Gunfire.DisplayName = FText::FromString(TEXT("Gunfire"));
        Gunfire.Priority = 3;
        Gunfire.Duration = 0.08f;
        Gunfire.BlendInTime = 0.01f;
        Gunfire.BlendOutTime = 0.05f;
        Gunfire.Scale = 1.f;
        Gunfire.bScaleWithDistance = false;
        Gunfire.bAllowMultipleInstances = true;
        
        // Minimal location, mostly rotation
        Gunfire.LocationZ.Amplitude = 1.f;
        Gunfire.LocationZ.Frequency = 50.f;
        
        Gunfire.RotationPitch.Amplitude = 0.8f;
        Gunfire.RotationPitch.Frequency = 40.f;
        
        ShakePresets.Add(Gunfire);
    }
    
    // ========================================================================
    // HEAVY GUNFIRE - For larger weapons (LMG, shotgun)
    // ========================================================================
    {
        FCameraShakePreset HeavyGunfire;
        HeavyGunfire.ShakeTag = FWWTagLibrary::CameraShake_Combat_HeavyGunfire();
        HeavyGunfire.DisplayName = FText::FromString(TEXT("Heavy Gunfire"));
        HeavyGunfire.Priority = 4;
        HeavyGunfire.Duration = 0.12f;
        HeavyGunfire.BlendInTime = 0.01f;
        HeavyGunfire.BlendOutTime = 0.08f;
        HeavyGunfire.Scale = 1.f;
        HeavyGunfire.bScaleWithDistance = false;
        HeavyGunfire.bAllowMultipleInstances = true;
        
        HeavyGunfire.LocationX.Amplitude = 2.f;
        HeavyGunfire.LocationX.Frequency = 35.f;
        HeavyGunfire.LocationZ.Amplitude = 3.f;
        HeavyGunfire.LocationZ.Frequency = 40.f;
        
        HeavyGunfire.RotationPitch.Amplitude = 1.5f;
        HeavyGunfire.RotationPitch.Frequency = 35.f;
        HeavyGunfire.RotationRoll.Amplitude = 0.5f;
        HeavyGunfire.RotationRoll.Frequency = 30.f;
        
        ShakePresets.Add(HeavyGunfire);
    }
    
    // ========================================================================
    // PUNCH - Melee impact received
    // ========================================================================
    {
        FCameraShakePreset Punch;
        Punch.ShakeTag = FWWTagLibrary::CameraShake_Combat_Punch();
        Punch.DisplayName = FText::FromString(TEXT("Punch"));
        Punch.Priority = 5;
        Punch.Duration = 0.15f;
        Punch.BlendInTime = 0.02f;
        Punch.BlendOutTime = 0.1f;
        Punch.Scale = 1.f;
        Punch.bScaleWithDistance = false;
        Punch.bAllowMultipleInstances = false;
        
        // Directional shake
        Punch.LocationY.Amplitude = 5.f;
        Punch.LocationY.Frequency = 30.f;
        
        Punch.RotationPitch.Amplitude = 2.f;
        Punch.RotationPitch.Frequency = 25.f;
        Punch.RotationYaw.Amplitude = 3.f;
        Punch.RotationYaw.Frequency = 20.f;
        Punch.RotationRoll.Amplitude = 2.f;
        Punch.RotationRoll.Frequency = 28.f;
        
        ShakePresets.Add(Punch);
    }
    
    // ========================================================================
    // HEAVY HIT - Big melee impact (hammer, bat)
    // ========================================================================
    {
        FCameraShakePreset HeavyHit;
        HeavyHit.ShakeTag = FWWTagLibrary::CameraShake_Combat_HeavyHit();
        HeavyHit.DisplayName = FText::FromString(TEXT("Heavy Hit"));
        HeavyHit.Priority = 7;
        HeavyHit.Duration = 0.25f;
        HeavyHit.BlendInTime = 0.02f;
        HeavyHit.BlendOutTime = 0.15f;
        HeavyHit.Scale = 1.f;
        HeavyHit.bScaleWithDistance = false;
        HeavyHit.bAllowMultipleInstances = false;
        
        HeavyHit.LocationX.Amplitude = 8.f;
        HeavyHit.LocationX.Frequency = 20.f;
        HeavyHit.LocationY.Amplitude = 10.f;
        HeavyHit.LocationY.Frequency = 18.f;
        HeavyHit.LocationZ.Amplitude = 5.f;
        HeavyHit.LocationZ.Frequency = 22.f;
        
        HeavyHit.RotationPitch.Amplitude = 4.f;
        HeavyHit.RotationPitch.Frequency = 15.f;
        HeavyHit.RotationYaw.Amplitude = 5.f;
        HeavyHit.RotationYaw.Frequency = 12.f;
        HeavyHit.RotationRoll.Amplitude = 3.f;
        HeavyHit.RotationRoll.Frequency = 18.f;
        
        HeavyHit.FOV.Amplitude = 2.f;
        HeavyHit.FOV.Frequency = 12.f;
        
        ShakePresets.Add(HeavyHit);
    }
    
    // ========================================================================
    // BULLET IMPACT - Getting shot
    // ========================================================================
    {
        FCameraShakePreset BulletImpact;
        BulletImpact.ShakeTag = FWWTagLibrary::CameraShake_Combat_BulletImpact();
        BulletImpact.DisplayName = FText::FromString(TEXT("Bullet Impact"));
        BulletImpact.Priority = 6;
        BulletImpact.Duration = 0.1f;
        BulletImpact.BlendInTime = 0.01f;
        BulletImpact.BlendOutTime = 0.06f;
        BulletImpact.Scale = 1.f;
        BulletImpact.bScaleWithDistance = false;
        BulletImpact.bAllowMultipleInstances = true;
        
        BulletImpact.LocationX.Amplitude = 2.f;
        BulletImpact.LocationX.Frequency = 40.f;
        BulletImpact.LocationZ.Amplitude = 1.5f;
        BulletImpact.LocationZ.Frequency = 45.f;
        
        BulletImpact.RotationPitch.Amplitude = 1.f;
        BulletImpact.RotationPitch.Frequency = 35.f;
        BulletImpact.RotationRoll.Amplitude = 0.8f;
        BulletImpact.RotationRoll.Frequency = 38.f;
        
        ShakePresets.Add(BulletImpact);
    }
    
    // ========================================================================
    // DEATH - Final death shake
    // ========================================================================
    {
        FCameraShakePreset Death;
        Death.ShakeTag = FWWTagLibrary::CameraShake_Combat_Death();
        Death.DisplayName = FText::FromString(TEXT("Death"));
        Death.Priority = 15;
        Death.Duration = 0.5f;
        Death.BlendInTime = 0.02f;
        Death.BlendOutTime = 0.3f;
        Death.Scale = 1.f;
        Death.bScaleWithDistance = false;
        Death.bAllowMultipleInstances = false;
        
        Death.LocationX.Amplitude = 10.f;
        Death.LocationX.Frequency = 15.f;
        Death.LocationY.Amplitude = 8.f;
        Death.LocationY.Frequency = 12.f;
        Death.LocationZ.Amplitude = 15.f;
        Death.LocationZ.Frequency = 18.f;
        
        Death.RotationPitch.Amplitude = 5.f;
        Death.RotationPitch.Frequency = 10.f;
        Death.RotationYaw.Amplitude = 3.f;
        Death.RotationYaw.Frequency = 8.f;
        Death.RotationRoll.Amplitude = 8.f;
        Death.RotationRoll.Frequency = 12.f;
        
        Death.FOV.Amplitude = 4.f;
        Death.FOV.Frequency = 6.f;
        
        ShakePresets.Add(Death);
    }
    
    // Populate supported shakes from presets
    PopulateSupportedShakes();
}