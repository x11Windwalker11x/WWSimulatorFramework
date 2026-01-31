// MPC_PlayerCameraManager.h
// Windwalker Framework - Modular Camera Manager
// ONE camera, modules provide config, Manager orchestrates

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/Config/CameraShakeConfigStructs.h"
#include "MPC_PlayerCameraManager.generated.h"

class UCameraModule_Master;
class UCameraComponent;
class USpringArmComponent;
class UUserSettingsSaveModule;
class UCameraShakeModule_Master;

// ============================================================================
// BROADCAST EVENT DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnCameraModeChanged,
    FGameplayTag, OldMode,
    FGameplayTag, NewMode,
    UCameraModule_Master*, NewModule);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnCameraInitialized,
    APawn*, OwnerPawn);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnAimModeChanged,
    FGameplayTag, OldAimState,
    FGameplayTag, NewAimState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnShoulderChanged,
    FGameplayTag, OldShoulderState,
    FGameplayTag, NewShoulderState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnCameraZoomChanged,
    float, ArmLength,
    float, ZoomPercent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnScopeRenderTargetToggled,
    bool, bActive);

/**
 * Modular PlayerCameraManager
 * 
 * Architecture:
 * - ONE camera component (CurrentCamera)
 * - Modules provide configuration
 * - Manager reads module config and applies to actual components
 * - Single API entry points: SetActiveCameraMode, SetAimState, SetShoulderState
 */
UCLASS()
class MODULARPLAYERCONTROLLER_API AMPC_PlayerCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()

public:
    AMPC_PlayerCameraManager();
    
    virtual void InitializeFor(APlayerController* PC) override;
    virtual void BeginPlay() override;
    // ========================================================================
    // BROADCAST EVENTS
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category="Camera|Events")
    FOnCameraModeChanged OnCameraModeChanged;
    
    UPROPERTY(BlueprintAssignable, Category="Camera|Events")
    FOnCameraInitialized OnCameraInitialized;

    UPROPERTY(BlueprintAssignable, Category="Camera|Events")
    FOnAimModeChanged OnAimModeChanged;
    
    UPROPERTY(BlueprintAssignable, Category="Camera|Events")
    FOnShoulderChanged OnShoulderChanged;
    
    UPROPERTY(BlueprintAssignable, Category="Camera|Events")
    FOnCameraZoomChanged OnCameraZoomChanged;
    
    UPROPERTY(BlueprintAssignable, Category="Camera|Events")
    FOnScopeRenderTargetToggled OnScopeRenderTargetToggled;

    // ========================================================================
    // MAIN API - Single Entry Points
    // ========================================================================
    
    /**
     * Switch to a camera mode by tag
     * Uses FindBestMatchingTag for hierarchical tag support
     */
    UFUNCTION(BlueprintCallable, Category="Camera")
    void SetActiveCameraMode(FGameplayTag NewCameraModeTag);
    
    /**
     * Set aim state - routes to active module
     * Checks module capabilities before routing
     */
    UFUNCTION(BlueprintCallable, Category="Camera")
    void SetAimState(FGameplayTag NewAimState);
    
    /**
     * Set shoulder side - routes to active module
     * Only works if module supports shoulder capability
     */
    UFUNCTION(BlueprintCallable, Category="Camera")
    void SetShoulderState(FGameplayTag NewShoulderState);
    
    /**
     * Toggle shoulder between left and right
     */
    UFUNCTION(BlueprintCallable, Category="Camera")
    void ToggleShoulderSide();
    
    /**
     * Toggle to next camera mode in registered order
     */
    UFUNCTION(BlueprintCallable, Category="Camera")
    void ToggleCameraMode();
    
    /**
     * Adjust camera zoom - routes to active module
     */
    UFUNCTION(BlueprintCallable, Category="Camera")
    void AdjustCameraDistance(float ScrollDelta);
    
    /**
     * Set user FOV - applies to active module and saves to user settings
     */
    UFUNCTION(BlueprintCallable, Category="Camera")
    void SetUserFOV(float NewFOV);

    // ========================================================================
    // READ-ONLY ACCESSORS
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category="Camera")
    UCameraComponent* GetActiveCamera() const { return CurrentCamera; }
    
    UFUNCTION(BlueprintPure, Category="Camera")
    USpringArmComponent* GetSpringArm() const { return CameraBoom; }
    
    UFUNCTION(BlueprintPure, Category="Camera|Modules")
    UCameraModule_Master* GetActiveCameraModule() const { return ActiveCameraModule; }
    
    UFUNCTION(BlueprintPure, Category="Camera|Modules")
    const TArray<FGameplayTag>& GetCameraModuleTags() const { return CameraModuleTags; }
    
    const TMap<FGameplayTag, UCameraModule_Master*>& GetCameraModules() const { return CameraModules; }
    
    UFUNCTION(BlueprintPure, Category="Camera|Modules")
    FGameplayTag GetCurrentCameraModeTag() const;
    
    UFUNCTION(BlueprintPure, Category="Camera|State")
    FGameplayTag GetCurrentAimState() const;
    
    UFUNCTION(BlueprintPure, Category="Camera|State")
    FGameplayTag GetCurrentShoulderState() const;
    
    UFUNCTION(BlueprintPure, Category="Camera")
    float GetUserFOV() const { return UserFOV; }

    // ========================================================================
    // MODULE REGISTRATION
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category="Camera|Modules")
    void RegisterCameraModule(UCameraModule_Master* Module);

protected:
    virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
    virtual void UpdateCamera(float DeltaTime) override;

    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    
    void InitializeCameraModules();
    void InitializeCamera(APawn* InPawn);
    void InitializeShakeModules();
    void LoadUserSettings();
    void SaveUserSettings();

    // ========================================================================
    // INTERNAL HELPERS
    // ========================================================================
    
    void AttachToSpringArm();
    void DetachFromSpringArm();
    void ApplyModuleConfig(UCameraModule_Master* Module);
    void ApplyCharacterRotationSettings(UCameraModule_Master* Module);
    void CacheCurrentTransformToModule();
    void RestoreCachedTransformFromModule(UCameraModule_Master* Module);
    UCameraModule_Master* FindModuleByTag(FGameplayTag Tag) const;
    
    void BindToModuleZoom(UCameraModule_Master* Module);
    void UnbindFromModuleZoom(UCameraModule_Master* Module);
    
    UFUNCTION()
    void HandleModuleZoomChanged(float ArmLength, float ZoomPercent);

    // ========================================================================
    // COMPONENTS
    // ========================================================================
    
    UPROPERTY(Transient)
    UCameraComponent* CurrentCamera = nullptr;
    
    UPROPERTY(Transient)
    USpringArmComponent* CameraBoom = nullptr;

    // ========================================================================
    // MODULE SYSTEM
    // ========================================================================
    
    UPROPERTY(Transient)
    UCameraModule_Master* ActiveCameraModule = nullptr;
    
    UPROPERTY()
    TMap<FGameplayTag, UCameraModule_Master*> CameraModules;
    
    UPROPERTY()
    TArray<FGameplayTag> CameraModuleTags;
    
    UPROPERTY(EditDefaultsOnly, Category="Camera|Modules")
    TArray<TSubclassOf<UCameraModule_Master>> DefaultModuleClasses;

    // ========================================================================
    // REFERENCES
    // ========================================================================
    
    UPROPERTY(Transient)
    APlayerController* OwnerPlayerController = nullptr;
    
    UPROPERTY(Transient)
    APawn* OwnerPawn = nullptr;

    // ========================================================================
    // STATE
    // ========================================================================
    
    bool bIsCameraDetached = false;
    bool bCameraInitialized = false;
    
    /** User-configurable FOV (saved to user settings) */
    float UserFOV = 90.f;

    // ========================================================================
    // DEBUG
    // ========================================================================
    
    UFUNCTION(Exec)
    void DebugCamera();

    // ============================================================================
    // ADD TO MPC_PlayerCameraManager.h
    // ============================================================================
    
    // Add forward declaration
    class UCameraShakeModule_Master;
    
    // Add delegate (before UCLASS)
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
        FOnCameraShakePlayed,
        FGameplayTag, ShakeTag,
        int32, InstanceID);
    
    // Add to public section:
    
        // ========================================================================
        // CAMERA SHAKE - Single Entry Point API
        // ========================================================================
        
        /**
         * Play a camera shake by tag
         * Finds the appropriate shake module and plays the shake
         * @param ShakeTag - Tag identifying the shake (CameraShake.Combat.Explosion, etc.)
         * @param Scale - Optional intensity scale
         * @return Instance ID (-1 if failed)
         */
        UFUNCTION(BlueprintCallable, Category="Camera|Shake")
        int32 PlayCameraShake(FGameplayTag ShakeTag, float Scale = 1.f);
        
        /**
         * Play a camera shake at world location with distance falloff
         * @param ShakeTag - Tag identifying the shake
         * @param SourceLocation - World location of shake source
         * @param Scale - Optional intensity scale
         * @return Instance ID (-1 if failed)
         */
        UFUNCTION(BlueprintCallable, Category="Camera|Shake")
        int32 PlayCameraShakeAtLocation(FGameplayTag ShakeTag, FVector SourceLocation, float Scale = 1.f);
        
        /**
         * Stop a specific shake instance
         * @param InstanceID - ID returned from PlayCameraShake
         * @param bImmediate - Skip blend out
         */
        UFUNCTION(BlueprintCallable, Category="Camera|Shake")
        void StopCameraShake(int32 InstanceID, bool bImmediate = false);
        
        /**
         * Stop all shakes with a specific tag
         */
        UFUNCTION(BlueprintCallable, Category="Camera|Shake")
        void StopCameraShakeByTag(FGameplayTag ShakeTag, bool bImmediate = false);
        
        /**
         * Stop all camera shakes
         */
        UFUNCTION(BlueprintCallable, Category="Camera|Shake")
        void StopAllCameraShakes(bool bImmediate = false);
        
        /**
         * Register a shake module
         */
        UFUNCTION(BlueprintCallable, Category="Camera|Shake")
        void RegisterShakeModule(UCameraShakeModule_Master* Module);
        
        /**
         * Get shake module by tag
         */
        UFUNCTION(BlueprintPure, Category="Camera|Shake")
        UCameraShakeModule_Master* GetShakeModule(FGameplayTag ModuleTag) const;
    
        // Shake event
        UPROPERTY(BlueprintAssignable, Category="Camera|Events")
        FOnCameraShakePlayed OnCameraShakePlayed;
    
    
    // ========================================================================
    // SHAKE MODULES
    // ========================================================================
        
        UPROPERTY()
        TMap<FGameplayTag, UCameraShakeModule_Master*> ShakeModules;
        
        UPROPERTY()
        TArray<FGameplayTag> ShakeModuleTags;
        
        /** Default shake module classes to instantiate */
        UPROPERTY(EditDefaultsOnly, Category="Camera|Shake")
        TArray<TSubclassOf<UCameraShakeModule_Master>> DefaultShakeModuleClasses;
        
        /** Current combined shake output */
        UPROPERTY(Transient)
        FCameraShakeOutput CurrentShakeOutput;
    
        // Internal
        UCameraShakeModule_Master* FindShakeModuleForTag(FGameplayTag ShakeTag) const;
        void UpdateAllShakes(float DeltaTime);

};