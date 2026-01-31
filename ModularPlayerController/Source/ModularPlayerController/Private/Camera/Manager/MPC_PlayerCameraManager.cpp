// MPC_PlayerCameraManager.cpp
// Windwalker Framework - Modular Camera Manager Implementation

#include "Camera/Manager/MPC_PlayerCameraManager.h"
#include "Camera/CameraModule_Master.h"
#include "Camera/Shake/CameraShakeModule_Master.h"
#include "Camera/Shake/CameraShakeModule_Combat.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/Helpers/Tags/TagHelpers.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "MasterSaveSubsystem.h"
#include "ModularPlayerController_Master.h"
#include "UserSettingsSaveModule.h"


AMPC_PlayerCameraManager::AMPC_PlayerCameraManager()
{
    CurrentCamera = nullptr;
    CameraBoom = nullptr;
    ActiveCameraModule = nullptr;
    OwnerPlayerController = nullptr;
    OwnerPawn = nullptr;
    bIsCameraDetached = false;
    bCameraInitialized = false;
    UserFOV = 90.f;
}

void AMPC_PlayerCameraManager::InitializeFor(APlayerController* PC)
{
    Super::InitializeFor(PC);
    
    if (PC && PC->IsLocalPlayerController())
    {
        OwnerPlayerController = PC;
        UE_LOG(LogTemp, Log, TEXT("CameraManager::InitializeFor - Cached LOCAL PC: %s"), *PC->GetName());
    }
    else
    {
        OwnerPlayerController = nullptr;
    }
}

void AMPC_PlayerCameraManager::BeginPlay()
{
    Super::BeginPlay();
    LoadUserSettings();
    InitializeCameraModules();
}

// ============================================================================
// MAIN API
// ============================================================================

void AMPC_PlayerCameraManager::SetActiveCameraMode(FGameplayTag NewCameraModeTag)
{
    if (!NewCameraModeTag.IsValid()) return;
    
    if (ActiveCameraModule && ActiveCameraModule->GetCameraModeTag() == NewCameraModeTag) return;
    
    UCameraModule_Master* NewModule = FindModuleByTag(NewCameraModeTag);
    if (!NewModule)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetActiveCameraMode: No module found for tag %s"), 
            *NewCameraModeTag.ToString());
        return;
    }
    
    if (!OwnerPawn)
    {
        if (OwnerPlayerController)
        {
            OwnerPawn = OwnerPlayerController->GetPawn();
        }
        if (!OwnerPawn) return;
    }
    
    if (!bCameraInitialized)
    {
        InitializeCamera(OwnerPawn);
    }
    
    FGameplayTag OldMode = GetCurrentCameraModeTag();
    
    // Deactivate old module
    if (ActiveCameraModule)
    {
        UnbindFromModuleZoom(ActiveCameraModule);
        CacheCurrentTransformToModule();
        ActiveCameraModule->OnDeactivated();
    }
    
    // Attachment based on SpringArm usage
    const FCameraSpringArmConfig& SpringConfig = NewModule->GetSpringArmConfig();
    if (SpringConfig.DefaultArmLength > 0.f)
    {
        AttachToSpringArm();
    }
    else
    {
        DetachFromSpringArm();
    }
    
    // Apply config
    ApplyModuleConfig(NewModule);
    ApplyCharacterRotationSettings(NewModule);
    RestoreCachedTransformFromModule(NewModule);
    
    // Activate new module
    BindToModuleZoom(NewModule);
    ActiveCameraModule = NewModule;
    ActiveCameraModule->SetBaseFOV(UserFOV);
    NewModule->OnActivated();
    
    // Refresh view target
    if (OwnerPlayerController)
    {
        OwnerPlayerController->SetViewTarget(OwnerPawn);
    }
    
    // Broadcast
    OnCameraModeChanged.Broadcast(OldMode, NewCameraModeTag, ActiveCameraModule);
    
    UE_LOG(LogTemp, Log, TEXT("SetActiveCameraMode: Switched to '%s'"), 
        *NewModule->GetDisplayName().ToString());
}

void AMPC_PlayerCameraManager::SetAimState(FGameplayTag NewAimState)
{
    if (!ActiveCameraModule) return;
    
    // Query capability
    if (!ActiveCameraModule->GetSupportedCapabilities().HasTag(NewAimState)) return;
    
    FGameplayTag OldAimState = ActiveCameraModule->GetCurrentAimState();
    if (OldAimState == NewAimState) return;
    
    // Route to module
    ActiveCameraModule->SetAimState(NewAimState);
    
    // Check scope render target
    const FCameraAimConfig& AimConfig = ActiveCameraModule->GetAimConfig();
    if (NewAimState.MatchesTagExact(FWWTagLibrary::Camera_Aim_Scope()) && AimConfig.bScopeUsesRenderTarget)
    {
        OnScopeRenderTargetToggled.Broadcast(true);
    }
    else if (OldAimState.MatchesTagExact(FWWTagLibrary::Camera_Aim_Scope()) && AimConfig.bScopeUsesRenderTarget)
    {
        OnScopeRenderTargetToggled.Broadcast(false);
    }
    
    // Broadcast
    OnAimModeChanged.Broadcast(OldAimState, NewAimState);
}

void AMPC_PlayerCameraManager::SetShoulderState(FGameplayTag NewShoulderState)
{
    if (!ActiveCameraModule) return;
    
    // Query capability
    if (!ActiveCameraModule->GetSupportedCapabilities().HasTag(NewShoulderState)) return;
    
    FGameplayTag OldShoulderState = ActiveCameraModule->GetCurrentShoulderState();
    if (OldShoulderState == NewShoulderState) return;
    
    // Route to module
    ActiveCameraModule->SetShoulderState(NewShoulderState);
    
    // Broadcast
    OnShoulderChanged.Broadcast(OldShoulderState, NewShoulderState);
}

void AMPC_PlayerCameraManager::ToggleShoulderSide()
{
    if (!ActiveCameraModule) return;
    
    FGameplayTag CurrentShoulder = ActiveCameraModule->GetCurrentShoulderState();
    
    FGameplayTag NewShoulder;
    if (CurrentShoulder.MatchesTagExact(FWWTagLibrary::Camera_Shoulder_Left()))
    {
        NewShoulder = FWWTagLibrary::Camera_Shoulder_Right();
    }
    else
    {
        NewShoulder = FWWTagLibrary::Camera_Shoulder_Left();
    }
    
    SetShoulderState(NewShoulder);
}

void AMPC_PlayerCameraManager::ToggleCameraMode()
{
    if (CameraModuleTags.Num() == 0) return;
    
    if (!ActiveCameraModule)
    {
        SetActiveCameraMode(CameraModuleTags[0]);
        return;
    }
    
    int32 CurrentIndex = CameraModuleTags.Find(ActiveCameraModule->GetCameraModeTag());
    if (CurrentIndex == INDEX_NONE)
    {
        CurrentIndex = 0;
    }
    
    int32 NextIndex = (CurrentIndex + 1) % CameraModuleTags.Num();
    SetActiveCameraMode(CameraModuleTags[NextIndex]);
}

void AMPC_PlayerCameraManager::AdjustCameraDistance(float ScrollDelta)
{
    if (!ActiveCameraModule) return;
    
    ActiveCameraModule->AdjustZoom(ScrollDelta);
}

void AMPC_PlayerCameraManager::SetUserFOV(float NewFOV)
{
    UserFOV = FMath::Clamp(NewFOV, 60.f, 120.f);
    
    if (ActiveCameraModule)
    {
        ActiveCameraModule->SetBaseFOV(UserFOV);
    }
    
    SaveUserSettings();
}

FGameplayTag AMPC_PlayerCameraManager::GetCurrentCameraModeTag() const
{
    return ActiveCameraModule ? ActiveCameraModule->GetCameraModeTag() : FGameplayTag::EmptyTag;
}

FGameplayTag AMPC_PlayerCameraManager::GetCurrentAimState() const
{
    return ActiveCameraModule ? ActiveCameraModule->GetCurrentAimState() : FGameplayTag::EmptyTag;
}

FGameplayTag AMPC_PlayerCameraManager::GetCurrentShoulderState() const
{
    return ActiveCameraModule ? ActiveCameraModule->GetCurrentShoulderState() : FGameplayTag::EmptyTag;
}

// ============================================================================
// MODULE ZOOM BINDING
// ============================================================================

void AMPC_PlayerCameraManager::BindToModuleZoom(UCameraModule_Master* Module)
{
    if (!Module) return;
    
    Module->OnZoomChanged.AddDynamic(this, &AMPC_PlayerCameraManager::HandleModuleZoomChanged);
}

void AMPC_PlayerCameraManager::UnbindFromModuleZoom(UCameraModule_Master* Module)
{
    if (!Module) return;
    
    Module->OnZoomChanged.RemoveDynamic(this, &AMPC_PlayerCameraManager::HandleModuleZoomChanged);
}

void AMPC_PlayerCameraManager::HandleModuleZoomChanged(float ArmLength, float ZoomPercent)
{
    OnCameraZoomChanged.Broadcast(ArmLength, ZoomPercent);
}

// ============================================================================
// MODULE REGISTRATION
// ============================================================================

void AMPC_PlayerCameraManager::RegisterCameraModule(UCameraModule_Master* Module)
{
    if (!Module) return;
    
    FGameplayTag ModuleTag = Module->GetCameraModeTag();
    if (!ModuleTag.IsValid()) return;
    
    if (CameraModules.Contains(ModuleTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("RegisterCameraModule: Replacing existing module for tag %s"),
            *ModuleTag.ToString());
    }
    
    CameraModules.Add(ModuleTag, Module);
    CameraModuleTags.AddUnique(ModuleTag);
    
    UE_LOG(LogTemp, Log, TEXT("RegisterCameraModule: Registered '%s' with tag '%s'"),
        *Module->GetDisplayName().ToString(), *ModuleTag.ToString());
}

void AMPC_PlayerCameraManager::InitializeCameraModules()
{
    CameraModules.Empty();
    CameraModuleTags.Empty();
    ActiveCameraModule = nullptr;
    
    for (const TSubclassOf<UCameraModule_Master>& ModuleClass : DefaultModuleClasses)
    {
        if (!ModuleClass) continue;
        
        UCameraModule_Master* Module = NewObject<UCameraModule_Master>(this, ModuleClass);
        if (Module)
        {
            RegisterCameraModule(Module);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("InitializeCameraModules: Registered %d modules"), CameraModules.Num());
    
    if (CameraModuleTags.Num() > 0)
    {
        SetActiveCameraMode(CameraModuleTags[0]);
    }
}

// ============================================================================
// CAMERA INITIALIZATION
// ============================================================================

void AMPC_PlayerCameraManager::InitializeCamera(APawn* InPawn)
{
    if (!InPawn) return;
    
    ACharacter* Character = Cast<ACharacter>(InPawn);
    if (!Character) return;
    
    OwnerPawn = InPawn;
    
    // Create SpringArm
    CameraBoom = NewObject<USpringArmComponent>(Character, TEXT("CameraBoom"));
    if (!CameraBoom) return;
    
    CameraBoom->RegisterComponent();
    CameraBoom->AttachToComponent(Character->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    CameraBoom->TargetArmLength = 300.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = false;
    
    // Create Camera
    CurrentCamera = NewObject<UCameraComponent>(Character, TEXT("MainCamera"));
    if (!CurrentCamera) return;
    
    CurrentCamera->RegisterComponent();
    CurrentCamera->SetFieldOfView(UserFOV);
    
    // Attach to SpringArm
    CurrentCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    CurrentCamera->SetRelativeLocation(FVector::ZeroVector);
    CurrentCamera->SetRelativeRotation(FRotator::ZeroRotator);
    bIsCameraDetached = false;
    
    bCameraInitialized = true;
    
    // Broadcast
    OnCameraInitialized.Broadcast(InPawn);
    
    UE_LOG(LogTemp, Log, TEXT("InitializeCamera: Camera system initialized for %s"), *Character->GetName());
}

// ============================================================================
// USER SETTINGS
// ============================================================================

void AMPC_PlayerCameraManager::LoadUserSettings()
{
    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    if (!GI) return;
    
    UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>();
    if (!SaveSubsystem) return;
    
    UUserSettingsSaveModule* UserSettings = SaveSubsystem->GetOrCreateModule<UUserSettingsSaveModule>();
    if (UserSettings)
    {
        UserFOV = UserSettings->GetFOV();
    }
}

void AMPC_PlayerCameraManager::SaveUserSettings()
{
    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    if (!GI) return;
    
    UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>();
    if (!SaveSubsystem) return;
    
    UUserSettingsSaveModule* UserSettings = SaveSubsystem->GetOrCreateModule<UUserSettingsSaveModule>();
    if (UserSettings)
    {
        UserSettings->SetFOV(UserFOV);
        SaveSubsystem->QuickSave(true);
    }
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

UCameraModule_Master* AMPC_PlayerCameraManager::FindModuleByTag(FGameplayTag Tag) const
{
    if (!Tag.IsValid() || CameraModules.Num() == 0) return nullptr;
    
    // Exact match first
    if (UCameraModule_Master* const* FoundModule = CameraModules.Find(Tag))
    {
        return *FoundModule;
    }
    
    // Hierarchical match
    FGameplayTag BestMatch = UWWSharedFunctionLibrary::FindBestMatchingTag(this, CameraModuleTags, Tag);
    if (BestMatch.IsValid())
    {
        if (UCameraModule_Master* const* FoundModule = CameraModules.Find(BestMatch))
        {
            return *FoundModule;
        }
    }
    
    return nullptr;
}

void AMPC_PlayerCameraManager::AttachToSpringArm()
{
    if (!CurrentCamera || !CameraBoom) return;
    if (!bIsCameraDetached) return;
    
    CurrentCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    CurrentCamera->SetRelativeLocation(FVector::ZeroVector);
    CurrentCamera->SetRelativeRotation(FRotator::ZeroRotator);
    bIsCameraDetached = false;
}

void AMPC_PlayerCameraManager::DetachFromSpringArm()
{
    if (!CurrentCamera) return;
    if (bIsCameraDetached) return;
    
    CurrentCamera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    bIsCameraDetached = true;
}

void AMPC_PlayerCameraManager::ApplyModuleConfig(UCameraModule_Master* Module)
{
    if (!Module) return;
    
    // SpringArm config
    if (CameraBoom)
    {
        const FCameraSpringArmConfig& SpringConfig = Module->GetSpringArmConfig();
        CameraBoom->TargetArmLength = Module->GetEffectiveArmLength();
        CameraBoom->SocketOffset = Module->GetEffectiveSocketOffset();
        CameraBoom->TargetOffset = SpringConfig.TargetOffset;
        CameraBoom->bUsePawnControlRotation = SpringConfig.bUsePawnControlRotation;
        CameraBoom->bEnableCameraLag = SpringConfig.bEnableCameraLag;
        CameraBoom->CameraLagSpeed = SpringConfig.CameraLagSpeed;
        CameraBoom->bEnableCameraRotationLag = SpringConfig.bEnableCameraRotationLag;
        CameraBoom->CameraRotationLagSpeed = SpringConfig.CameraRotationLagSpeed;
    }
    
    // Camera config
    if (CurrentCamera)
    {
        CurrentCamera->SetFieldOfView(Module->GetEffectiveFOV());
    }
}

void AMPC_PlayerCameraManager::ApplyCharacterRotationSettings(UCameraModule_Master* Module)
{
    if (!Module || !OwnerPawn) return;
    
    ACharacter* Character = Cast<ACharacter>(OwnerPawn);
    if (!Character) return;
    
    const FCameraCharacterConfig& CharConfig = Module->GetCharacterConfig();
    
    Character->bUseControllerRotationYaw = CharConfig.bUseControllerRotationYaw;
    Character->bUseControllerRotationPitch = false;
    Character->bUseControllerRotationRoll = false;
    
    if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = CharConfig.bOrientRotationToMovement;
    }
}

void AMPC_PlayerCameraManager::CacheCurrentTransformToModule()
{
    if (!ActiveCameraModule || !CurrentCamera) return;
    
    float CurrentArmLength = ActiveCameraModule->GetEffectiveArmLength();
    ActiveCameraModule->CacheTransform(CurrentCamera->GetComponentTransform(), CurrentArmLength);
}

void AMPC_PlayerCameraManager::RestoreCachedTransformFromModule(UCameraModule_Master* Module)
{
    if (!Module || !CurrentCamera || !CameraBoom) return;
    
    const FCameraRuntimeState& RuntimeState = Module->GetRuntimeState();
    if (RuntimeState.bHasCachedTransform)
    {
        CameraBoom->TargetArmLength = RuntimeState.CachedArmLength;
    }
}

// ============================================================================
// UPDATE
// ============================================================================

void AMPC_PlayerCameraManager::UpdateCamera(float DeltaTime)
{
    Super::UpdateCamera(DeltaTime);
    
    if (PCOwner && PCOwner->IsLocalPlayerController())
    {
        OwnerPlayerController = PCOwner;
        OwnerPawn = OwnerPlayerController->GetPawn();
    }
}

void AMPC_PlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
    if (!CurrentCamera || !OwnerPawn || !ActiveCameraModule)
    {
        Super::UpdateViewTarget(OutVT, DeltaTime);
        return;
    }
    
    // Update interpolation
    ActiveCameraModule->UpdateInterpolation(DeltaTime);
    
    const FCameraSpringArmConfig& SpringConfig = ActiveCameraModule->GetSpringArmConfig();
    
    if (SpringConfig.DefaultArmLength > 0.f && !bIsCameraDetached)
    {
        // SpringArm mode
        if (CameraBoom)
        {
            CameraBoom->TargetArmLength = ActiveCameraModule->GetEffectiveArmLength();
            CameraBoom->SocketOffset = ActiveCameraModule->GetEffectiveSocketOffset();
        }
        
        if (CurrentCamera)
        {
            CurrentCamera->SetFieldOfView(ActiveCameraModule->GetEffectiveFOV());
        }
    }
    else
    {
        // Manual positioning mode (FPS)
        ACharacter* Character = Cast<ACharacter>(OwnerPawn);
        if (Character && Character->GetMesh())
        {
            FName SocketName = ActiveCameraModule->GetAttachSocketName();
            
            if (!SocketName.IsNone())
            {
                FVector SocketLocation = Character->GetMesh()->GetSocketLocation(SocketName);
                FRotator ControlRotation = OwnerPawn->GetControlRotation();
                
                CurrentCamera->SetWorldLocation(SocketLocation);
                CurrentCamera->SetWorldRotation(ControlRotation);
                CurrentCamera->SetFieldOfView(ActiveCameraModule->GetEffectiveFOV());
            }
        }
    }
    // Add to UpdateViewTarget() before setting OutVT.POV:
    // Update shakes
    UpdateAllShakes(DeltaTime);
    
    // Apply shake to output
    OutVT.POV.Location += CurrentShakeOutput.LocationOffset;
    OutVT.POV.Rotation += CurrentShakeOutput.RotationOffset;
    OutVT.POV.FOV += CurrentShakeOutput.FOVOffset;
    
    // Set output POV
    OutVT.POV.Location = CurrentCamera->GetComponentLocation();
    OutVT.POV.Rotation = CurrentCamera->GetComponentRotation();
    OutVT.POV.FOV = CurrentCamera->FieldOfView;
}

// ============================================================================
// DEBUG
// ============================================================================

void AMPC_PlayerCameraManager::DebugCamera()
{
    UE_LOG(LogTemp, Warning, TEXT("========== CAMERA DEBUG =========="));
    UE_LOG(LogTemp, Warning, TEXT("OwnerPlayerController: %s"), OwnerPlayerController ? *OwnerPlayerController->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("OwnerPawn: %s"), OwnerPawn ? *OwnerPawn->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("bCameraInitialized: %s"), bCameraInitialized ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("bIsCameraDetached: %s"), bIsCameraDetached ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("UserFOV: %.1f"), UserFOV);
    
    if (ActiveCameraModule)
    {
        UE_LOG(LogTemp, Warning, TEXT("ActiveModule: %s (Tag: %s)"), 
            *ActiveCameraModule->GetDisplayName().ToString(),
            *ActiveCameraModule->GetCameraModeTag().ToString());
        UE_LOG(LogTemp, Warning, TEXT("  AimState: %s"), *ActiveCameraModule->GetCurrentAimState().ToString());
        UE_LOG(LogTemp, Warning, TEXT("  ShoulderState: %s"), *ActiveCameraModule->GetCurrentShoulderState().ToString());
        UE_LOG(LogTemp, Warning, TEXT("  EffectiveFOV: %.1f"), ActiveCameraModule->GetEffectiveFOV());
        UE_LOG(LogTemp, Warning, TEXT("  EffectiveArmLength: %.1f"), ActiveCameraModule->GetEffectiveArmLength());
        UE_LOG(LogTemp, Warning, TEXT("  EffectiveSocketOffset: %s"), *ActiveCameraModule->GetEffectiveSocketOffset().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ActiveModule: NULL"));
    }
    
    if (CurrentCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("CurrentCamera:"));
        UE_LOG(LogTemp, Warning, TEXT("  Location: %s"), *CurrentCamera->GetComponentLocation().ToString());
        UE_LOG(LogTemp, Warning, TEXT("  FOV: %.1f"), CurrentCamera->FieldOfView);
    }
    
    if (CameraBoom)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraBoom:"));
        UE_LOG(LogTemp, Warning, TEXT("  ArmLength: %.1f"), CameraBoom->TargetArmLength);
        UE_LOG(LogTemp, Warning, TEXT("  SocketOffset: %s"), *CameraBoom->SocketOffset.ToString());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Registered Modules (%d):"), CameraModules.Num());
    for (const FGameplayTag& Tag : CameraModuleTags)
    {
        if (UCameraModule_Master* const* Module = CameraModules.Find(Tag))
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s (%s)"), *(*Module)->GetDisplayName().ToString(), *Tag.ToString());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("==================================="));
}


// Add implementations:

void AMPC_PlayerCameraManager::InitializeShakeModules()
{
    ShakeModules.Empty();
    ShakeModuleTags.Empty();
    
    for (const TSubclassOf<UCameraShakeModule_Master>& ModuleClass : DefaultShakeModuleClasses)
    {
        if (!ModuleClass) continue; 
        
        UCameraShakeModule_Master* Module = NewObject<UCameraShakeModule_Master>(this, ModuleClass);
        if (Module)
        {
            RegisterShakeModule(Module);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("InitializeShakeModules: Registered %d shake modules"), ShakeModules.Num());
}

void AMPC_PlayerCameraManager::RegisterShakeModule(UCameraShakeModule_Master* Module)
{
    if (!Module) return;
    
    FGameplayTag ModuleTag = Module->GetModuleTag();
    if (!ModuleTag.IsValid()) return;
    
    Module->Initialize(this);
    
    ShakeModules.Add(ModuleTag, Module);
    ShakeModuleTags.AddUnique(ModuleTag);
    
    UE_LOG(LogTemp, Log, TEXT("RegisterShakeModule: Registered '%s'"), 
        *Module->GetDisplayName().ToString());
}

UCameraShakeModule_Master* AMPC_PlayerCameraManager::GetShakeModule(FGameplayTag ModuleTag) const
{
    if (UCameraShakeModule_Master* const* Found = ShakeModules.Find(ModuleTag))
    {
        return *Found;
    }
    return nullptr;
}

UCameraShakeModule_Master* AMPC_PlayerCameraManager::FindShakeModuleForTag(FGameplayTag ShakeTag) const
{
    // Find which module supports this shake tag
    for (const auto& Pair : ShakeModules)
    {
        if (Pair.Value->GetSupportedShakes().HasTag(ShakeTag))
        {
            return Pair.Value;
        }
    }
    return nullptr;
}

int32 AMPC_PlayerCameraManager::PlayCameraShake(FGameplayTag ShakeTag, float Scale)
{
    UCameraShakeModule_Master* Module = FindShakeModuleForTag(ShakeTag);
    if (!Module)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayCameraShake: No module found for tag '%s'"), 
            *ShakeTag.ToString());
        return -1;
    }
    
    int32 InstanceID = Module->PlayShake(ShakeTag, Scale);
    
    if (InstanceID != -1)
    {
        OnCameraShakePlayed.Broadcast(ShakeTag, InstanceID);
    }
    
    return InstanceID;
}

int32 AMPC_PlayerCameraManager::PlayCameraShakeAtLocation(FGameplayTag ShakeTag, FVector SourceLocation, float Scale)
{
    UCameraShakeModule_Master* Module = FindShakeModuleForTag(ShakeTag);
    if (!Module)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayCameraShakeAtLocation: No module found for tag '%s'"), 
            *ShakeTag.ToString());
        return -1;
    }
    
    int32 InstanceID = Module->PlayShakeAtLocation(ShakeTag, SourceLocation, Scale);
    
    if (InstanceID != -1)
    {
        OnCameraShakePlayed.Broadcast(ShakeTag, InstanceID);
    }
    
    return InstanceID;
}

void AMPC_PlayerCameraManager::StopCameraShake(int32 InstanceID, bool bImmediate)
{
    // Try all modules since we don't track which module owns which instance
    for (const auto& Pair : ShakeModules)
    {
        Pair.Value->StopShake(InstanceID, bImmediate);
    }
}

void AMPC_PlayerCameraManager::StopCameraShakeByTag(FGameplayTag ShakeTag, bool bImmediate)
{
    UCameraShakeModule_Master* Module = FindShakeModuleForTag(ShakeTag);
    if (Module)
    {
        Module->StopShakeByTag(ShakeTag, bImmediate);
    }
}

void AMPC_PlayerCameraManager::StopAllCameraShakes(bool bImmediate)
{
    for (const auto& Pair : ShakeModules)
    {
        Pair.Value->StopAllShakes(bImmediate);
    }
    
    CurrentShakeOutput = FCameraShakeOutput();
}

void AMPC_PlayerCameraManager::UpdateAllShakes(float DeltaTime)
{
    CurrentShakeOutput = FCameraShakeOutput();
    
    FVector CameraLocation = CurrentCamera ? CurrentCamera->GetComponentLocation() : FVector::ZeroVector;
    
    for (const auto& Pair : ShakeModules)
    {
        FCameraShakeOutput ModuleOutput = Pair.Value->UpdateShakes(DeltaTime, CameraLocation);
        
        // Accumulate
        CurrentShakeOutput.LocationOffset += ModuleOutput.LocationOffset;
        CurrentShakeOutput.RotationOffset += ModuleOutput.RotationOffset;
        CurrentShakeOutput.FOVOffset += ModuleOutput.FOVOffset;
    }
}

