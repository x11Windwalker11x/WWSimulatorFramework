// Fill out your copyright notice in the Description page of Project Settings.

#include "ModularPlayerController_Master.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/ModularInteractionSystem/InteractableInterface.h"
#include "Interfaces/ModularInteractionSystem//InteractorInterface.h"
#include "Interfaces/ModularPlayerController/ModularInputReceiver.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Pawns/MPC_Character_Master.h"
#include "Pawns/MPC_Pawn_Master.h"
#include "Pawns/Vehicles/MPC_Vehicle_Master.h"
#include "Subsystems/InventoryWidgetManager.h"

AModularPlayerController_Master::AModularPlayerController_Master()
{
    bAutoManageActiveCameraTarget = true;
    CheatPermissionLevel = ECheatPermissionLevel::GodMode;
    MachineID = TEXT("");
    
    PlayerCameraManagerClass = AMPC_PlayerCameraManager::StaticClass();
}
    
void AModularPlayerController_Master::BeginPlay()
{
    Super::BeginPlay();

    LoadInputSettings();
    
    if (!Cast<AMPC_PlayerCameraManager>(PlayerCameraManager))
    {
        PlayerCameraManagerClass = AMPC_PlayerCameraManager::StaticClass();
        UE_LOG(LogInput, Log, TEXT("Re-Configured PlayerCameraManager to MPC_PlayerCameraManager runtime"));
    }
    
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(MPC_IMC_Character_Direct, 0);
    }
    
    
    RefreshInputBindings();
}

void AModularPlayerController_Master::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (!Cast<AMPC_PlayerCameraManager>(PlayerCameraManager))
    {
        UE_LOG(LogInput, Error, TEXT("AModularPlayerController_Master::OnPossess Error: Can't find MPC_CameraManager"));
        return;
    }
    
    if (GetPawn() && GetPawn()->GetClass()->ImplementsInterface(UInteractorInterface::StaticClass()))
    {
        AMPC_PlayerCameraManager* MPC_PlayerCameraManager = Cast<AMPC_PlayerCameraManager>(PlayerCameraManager);
        MPC_PlayerCameraManager->InitializeCameraModules();
    }
    else
    {
        UE_LOG(LogInput, Error, TEXT("AModularPlayerController_Master::OnPossess cant find pawn implementing interactor inteface. Controller:%s"), *GetNameSafe(this));
    }

    // ========================================================================
    // Register with subsystem
    // ========================================================================
    if (InPawn)
    {
        if (UInteractionSubsystem* Sys = UInteractionSubsystem::Get(GetWorld()))
        {
        
            Sys->RegisterPlayerPawn(InPawn);
            Sys->bUseBatchedTraces = true;
            Sys->bAutoUpdatePlayerFocus = true;
            UE_LOG(LogInteractableSubsystem, Log, TEXT("✅ PlayerController registered pawn: %s"), 
    *InPawn->GetName());

        }
    }

}

void AModularPlayerController_Master::OnUnPossess()
{
    // Unregister from subsystem (ONLY 1 LINE!)
    if (APawn* PossessedPawn = GetPawn())
    {
        if (UInteractionSubsystem* Sys = UInteractionSubsystem::Get(GetWorld()))
        {
            Sys->UnregisterPlayerPawn(PossessedPawn);
            UE_LOG(LogInteractableSubsystem, Log, TEXT("❌ PlayerController unregistered pawn: %s"), 
    *PossessedPawn->GetName());

        }
    }
    
    Super::OnUnPossess();
}


bool AModularPlayerController_Master::GetbCanInteract_Implementation()
{
    if (GetPawn() && GetPawn()->GetClass()->ImplementsInterface(UInteractorInterface::StaticClass()))
    {
        return IInteractorInterface::Execute_GetbCanInteract(GetPawn());
    }
    return false;
}

FString AModularPlayerController_Master::GetPersistentPlayerID()
{
    if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
    {
        if (IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
        {
            int32 LocalUserNum = GetLocalPlayer() ? GetLocalPlayer()->GetControllerId() : 0;
            TSharedPtr<const FUniqueNetId> UniqueId = Identity->GetUniquePlayerId(LocalUserNum);
            
            if (UniqueId.IsValid())
            {
                FString PlatformID = UniqueId->ToString();
                UE_LOG(LogTemp, Warning, TEXT("Platform ID: %s"), *PlatformID);
                return PlatformID;
            }
        }
    }
    
    return GetHardwareBasedID();
}

FString AModularPlayerController_Master::GetHardwareBasedID()
{
    FString LoginId = FPlatformMisc::GetLoginId();
    
    if (!MachineID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Machine ID: %s"), *MachineID);
        return MachineID;
    }
    
    FString ComputerName = FPlatformProcess::ComputerName();
    FString UserName = FPlatformProcess::UserName();
    FString Combined = ComputerName + TEXT("_") + UserName;
    
    uint32 Hash = FCrc::StrCrc32(*Combined);
    
    UE_LOG(LogTemp, Warning, TEXT("Hardware Hash ID: %u"), Hash);
    
    return FString::FromInt(Hash);
}

FString AModularPlayerController_Master::GetOrCreatePersistentID()
{
    const FString SaveSlotName = TEXT("PersistentPlayerID");
    
    if (UPersistentPlayerData* SaveGame = Cast<UPersistentPlayerData>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
    {
        if (!SaveGame->PlayerID.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Loaded persistent ID: %s"), *SaveGame->PlayerID);
            return SaveGame->PlayerID;
        }
    }
    
    UPersistentPlayerData* NewSaveGame = Cast<UPersistentPlayerData>(
        UGameplayStatics::CreateSaveGameObject(UPersistentPlayerData::StaticClass()));
    
    NewSaveGame->PlayerID = FGuid::NewGuid().ToString();
    
    if (UGameplayStatics::SaveGameToSlot(NewSaveGame, SaveSlotName, 0))
    {
        UE_LOG(LogTemp, Warning, TEXT("Created new persistent ID: %s"), *NewSaveGame->PlayerID);
        return NewSaveGame->PlayerID;
    }
    
    return TEXT("FALLBACK_ID");
}

void AModularPlayerController_Master::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AModularPlayerController_Master, CheatPermissionLevel);
}

void AModularPlayerController_Master::TryInteract()
{
    if (UWorld* World = GetWorld())
    {
        UInteractionSubsystem* InteractionSubsystem = UInteractionSubsystem::Get(World);
        if (!InteractionSubsystem) return;
        
        APawn* ControlledPawn = GetPawn();
        if (!ControlledPawn) return;
        
        // Get view parameters
        FVector ViewLocation = PlayerCameraManager->GetCameraLocation();
        FRotator ViewRotation = PlayerCameraManager->GetCameraRotation();
        
        if (HasAuthority())
        {
            // ========================================
            // SERVER: Direct execution (no prediction needed)
            // ========================================
            UE_LOG(LogInventoryInteractableSystem, Warning, 
                TEXT("🖥️ SERVER: Direct interaction"));
            
            InteractionSubsystem->TryInteract(this);
        }
        else
        {
            // ========================================
            // CLIENT: PREDICTION PATH
            // ========================================
            
            // Get InteractorComponent
            UInteractorComponent* InteractorComp = ControlledPawn->FindComponentByClass<UInteractorComponent>();
            if (!InteractorComp) return;
            
            // Check if prediction is enabled
            if (!InteractorComp->bEnableClientPrediction)
            {
                // Prediction disabled - use old path
                UE_LOG(LogInventoryInteractableSystem, Log, 
                    TEXT("📡 CLIENT: Sending to server (no prediction)"));
                
                // Compress data
                FVector ViewOffset = ViewLocation - ControlledPawn->GetActorLocation();
                FVector_NetQuantize10 CompressedOffset(ViewOffset);
                uint16 CompressedPitch = FRotator::CompressAxisToShort(ViewRotation.Pitch);
                uint16 CompressedYaw = FRotator::CompressAxisToShort(ViewRotation.Yaw);
                
                Server_TryInteract(CompressedOffset, CompressedPitch, CompressedYaw, 255); // 255 = no prediction
                return;
            }
            
            // ========================================
            // STEP 1: DISCOVERY - Find target using subsystem
            // ========================================
            
            AActor* PredictedTarget = InteractionSubsystem->GetBestInteractableForPlayerWithViewParams(
                ControlledPawn, 
                ViewLocation, 
                ViewRotation
            );
            
            if (!PredictedTarget)
            {
                UE_LOG(LogInventoryInteractableSystem, Log, 
                    TEXT("❌ CLIENT: No interactable found in trace"));
                return;
            }
            
            // ========================================
            // STEP 2: PREDICTION - Execute predicted effects (INSTANT!)
            // ========================================
            
            uint8 PredictionID = static_cast<uint8>(InteractorComp->NextPredictionID++ % 256);
            
            UE_LOG(LogInventoryInteractableSystem, Warning, 
                TEXT("🔮 CLIENT: Executing PREDICTED interaction (ID: %d) on %s"), 
                PredictionID, *PredictedTarget->GetName());
            
            InteractorComp->ExecutePredictedEffects(PredictedTarget, static_cast<int32>(PredictionID));
            
            // ========================================
            // STEP 3: COMPRESS AND SEND TO SERVER
            // ========================================
            
            // Compress view data for bandwidth efficiency
            FVector ViewOffset = ViewLocation - ControlledPawn->GetActorLocation();
            FVector_NetQuantize10 CompressedOffset(ViewOffset);
            
            uint16 CompressedPitch = FRotator::CompressAxisToShort(ViewRotation.Pitch);
            uint16 CompressedYaw = FRotator::CompressAxisToShort(ViewRotation.Yaw);
            
            UE_LOG(LogInventoryInteractableSystem, Log, 
                TEXT("📡 CLIENT: Sending predicted interaction to server (ID: %d)"), 
                PredictionID);
            
            Server_TryInteract(CompressedOffset, CompressedPitch, CompressedYaw, PredictionID);
        }
    }
}

void AModularPlayerController_Master::Server_TryInteract_Implementation(
    FVector_NetQuantize10 ViewOffset,
    uint16 CompressedPitch,
    uint16 CompressedYaw,
    uint8 PredictionID)
{
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("🖥️ SERVER: Received RPC - PredictionID: %d"), 
        PredictionID);
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    UInteractionSubsystem* InteractionSubsystem = UInteractionSubsystem::Get(World);
    if (!InteractionSubsystem) return;
    
    // ========================================
    // DECOMPRESS VIEW DATA
    // ========================================
    
    FVector ViewLocation = ControlledPawn->GetActorLocation() + FVector(ViewOffset);
    
    FRotator ViewRotation;
    ViewRotation.Pitch = FRotator::DecompressAxisFromShort(CompressedPitch);
    ViewRotation.Yaw = FRotator::DecompressAxisFromShort(CompressedYaw);
    ViewRotation.Roll = 0.f;
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("   Decompressed ViewLoc: %s, ViewRot: %s"), 
        *ViewLocation.ToString(), *ViewRotation.ToString());
    
    // ========================================
    // Check if this is a prediction or non-prediction interaction
    // ========================================
    
    if (PredictionID == 255)
    {
        // No prediction - just execute via subsystem
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("   No prediction - executing via subsystem"));
        InteractionSubsystem->TryInteract(this);
        return;
    }
    
    // ========================================
    // PREDICTION PATH: Validate with same discovery logic
    // ========================================
    
    // Get InteractorComponent
    UInteractorComponent* InteractorComp = ControlledPawn->FindComponentByClass<UInteractorComponent>();
    if (!InteractorComp) return;
    
    // Use subsystem to find target (same logic as client!)
    AActor* ServerTarget = InteractionSubsystem->GetBestInteractableForPlayerWithViewParams(
        ControlledPawn,
        ViewLocation,
        ViewRotation
    );
    
    // Validate and execute
    InteractorComp->ValidateAndExecutePredictedInteraction(
        ServerTarget,
        static_cast<int32>(PredictionID)
    );
}

void AModularPlayerController_Master::SetCheatPermission(ECheatPermissionLevel NewPermissionLevel)
{
    CheatPermissionLevel = NewPermissionLevel;
    UE_LOG(LogTemp, Warning, TEXT("Cheat Permission %s set for %s"), *GetName(), *UEnum::GetValueAsString(CheatPermissionLevel));
}

ECheatPermissionLevel AModularPlayerController_Master::GetCheatPermission()
{
    return CheatPermissionLevel;
}

void AModularPlayerController_Master::PossessChaosVehicle(AModularVehicleClusterPawn* Vehicle)
{
    if (Vehicle && HasAuthority())
    {
        Possess(Vehicle);
    }
}

bool AModularPlayerController_Master::ShouldUseCrouchToggle() const
{
    return InputModeTags.HasTag(FWWTagLibrary::Input_Mode_Crouch_Toggle());
}

void AModularPlayerController_Master::LoadInputSettings()
{
    if (InputSettingsAsset)
    {
        InputModeTags = InputSettingsAsset->InputModeTags;
        UE_LOG(LogInput, Log, TEXT("✅ Loaded input settings from asset: %s"), *InputSettingsAsset->ProfileName.ToString());
        UE_LOG(LogInput, Log, TEXT("   Input modes: %s"), *InputModeTags.ToStringSimple());
    }
    else
    {
        UE_LOG(LogInput, Warning, TEXT("⚠️ No InputSettingsAsset assigned! Using default input modes."));
        InputModeTags.AddTag(FWWTagLibrary::Input_Mode_Crouch_Toggle());
    }
}


void AModularPlayerController_Master::RefreshInputBindings()
{
    UE_LOG(LogInput, Log, TEXT("🔄 RefreshInputBindings called"));
    
    if (!InputComponent)
    {
        UE_LOG(LogInput, Error, TEXT("❌ InputComponent is null!"));
        return;
    }

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInputComponent)
    {
        UE_LOG(LogInput, Error, TEXT("❌ InputComponent is not UEnhancedInputComponent!"));
        return;
    }

    EnhancedInputComponent->ClearActionBindings();
    UE_LOG(LogInput, Log, TEXT("Cleared all action bindings"));

    if (MovementAction)
    {
        EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AModularPlayerController_Master::Move);
    }
    
    if (LookAction)
    {
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AModularPlayerController_Master::Look);
    }
    
    if (InteractionAction)
    {
        EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &AModularPlayerController_Master::OnInteractionPressed);
        EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Completed, this, &AModularPlayerController_Master::OnInteractionReleased);
    }
    
    if (CancelAction)
    {
        EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Completed, this, &AModularPlayerController_Master::OnCancelActionReleased);
    }
    
    if (JumpAction)
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AModularPlayerController_Master::OnJumpActionPressed);
    }
    
    if (CrouchAction)
    {
        if (ShouldUseCrouchToggle())
        {
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AModularPlayerController_Master::OnCrouchToggle);
            UE_LOG(LogInput, Log, TEXT("✅ Crouch set to TOGGLE mode"));
        }
        else
        {
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AModularPlayerController_Master::OnCrouchActionPressed);
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AModularPlayerController_Master::OnCrouchActionReleased);
            UE_LOG(LogInput, Log, TEXT("✅ Crouch set to HOLD mode"));
        }
    }
    
    if (SwitchCameraAction)
    {
        EnhancedInputComponent->BindAction(SwitchCameraAction, ETriggerEvent::Triggered, this, &AModularPlayerController_Master::OnSwitchCameraActionTriggered);
    }
    
    if (CameraZoomAction)
    {
        EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AModularPlayerController_Master::OnCameraZoomTriggered);
    }
    if (QuickSaveAction)
    {
        EnhancedInputComponent->BindAction(QuickSaveAction, ETriggerEvent::Triggered, this, &AModularPlayerController_Master::OnQuickSave);
    }
    if (QuickLoadAction)
    {
        EnhancedInputComponent->BindAction(QuickLoadAction, ETriggerEvent::Triggered, this, &AModularPlayerController_Master::OnQuickLoad);
    }
    if (InventoryAction)
    {
        EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Completed, this, &AModularPlayerController_Master::OnInventoryToggle);
    }
    if (ESCContextAction)
    {
        EnhancedInputComponent->BindAction(ESCContextAction, ETriggerEvent::Triggered, this, &AModularPlayerController_Master::OnESC_ContextAction);
    }
}

void AModularPlayerController_Master::SetupInputComponent()
{
    Super::SetupInputComponent();
    RefreshInputBindings();
}

void AModularPlayerController_Master::HandleInteract()
{
    if (UInteractionSubsystem* InteractionSubsystem = UInteractionSubsystem::Get(GetWorld()))
    {
        InteractionSubsystem->TryInteract(this);
    }
}

void AModularPlayerController_Master::OnInteractionPressed(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Log, TEXT("Interaction Started: Button Pressed"));
    AModularPlayerController_Master::TryInteract();
    
    if (GetPawn() && (GetPawn()->GetClass()->ImplementsInterface(UInteractorInterface::StaticClass()) || IInteractorInterface::Execute_GetbCanInteract(GetPawn()) || !bInteractionActionInMashingMode))
    {
        GetWorld()->GetTimerManager().SetTimer(
            InteractionActionHoldTimerHandle,
            this,
            &AModularPlayerController_Master::OnInteractionHold,
            GetWorld()->GetDeltaSeconds(),
            true,
            InteractionActionHoldTimerDelay            
        );
        
        if (bInteractionActionInMashingMode)
        {
            InteractionActionCurrentMashCount++;
            if(InteractionActionCurrentMashCount == 0)
            {
                float mashpercentage = float(float(InteractionActionCurrentMashCount) / InteractionActionRequiredMashCount) * 100;
                UE_LOG(LogTemp, Log, TEXT("Interaction Started - Mashing: %f percent complete"), mashpercentage);
                GetWorld()->GetTimerManager().SetTimer(
                    InteractionActionMashCountTimerHandle,
                    this,
                    &AModularPlayerController_Master::OnInteractionActionMashTimeExpired,
                    InteractionActionMashTimeWindow,
                    false
                );
            }
            
            if (InteractionActionCurrentMashCount >= InteractionActionRequiredMashCount)
            {
                UE_LOG(LogTemp, Log, TEXT("InteractionAction button press: Mash Complete!"));
                GetWorld()->GetTimerManager().ClearTimer(InteractionActionMashCountTimerHandle);
                InteractionActionCurrentMashCount = 0;
                bInteractionActionInMashingMode = false;
            }
            
            if (bInteractionActionActionCancelled)
            {
                UE_LOG(LogTemp, Log, TEXT("Interaction Action - button release: Mash cancelled by input!"));
                GetWorld()->GetTimerManager().ClearTimer(InteractionActionMashCountTimerHandle);
                InteractionActionCurrentMashCount = 0;
                bInteractionActionInMashingMode = false;
            }
        }
    }
    
    InteractionActionIsHolding = true;
    InteractionActionCurrentHoldTime = 0.0f;
}

void AModularPlayerController_Master::OnInteractionActionMashTimeExpired()
{
    UE_LOG(LogTemp, Log, TEXT("InteractionAction button press: Mash failed:Timeout!"));
    GetWorld()->GetTimerManager().ClearTimer(InteractionActionMashCountTimerHandle);
    InteractionActionCurrentMashCount = 0;
    bInteractionActionInMashingMode = false;
}

void AModularPlayerController_Master::OnInteractionReleased()
{
    if (!bInteractionActionInMashingMode)
    {
        UE_LOG(LogTemp, Log, TEXT("Interaction Released: Button Released"));
    }
    
    InteractionActionIsHolding = false;
    InteractionActionCurrentHoldTime = 0.0f;
    GetWorld()->GetTimerManager().ClearTimer(InteractionActionHoldTimerHandle);
}

void AModularPlayerController_Master::OnInteractionHold()
{
    InteractionActionCurrentHoldTime += GetWorld()->GetDeltaSeconds();
    float const InteractionActionHoldProgress = InteractionActionCurrentHoldTime / InteractionActionRequiredHoldDuration;
    UE_LOG(LogTemp, Log, TEXT("InteractionAction Progress - Hold: %f"), InteractionActionHoldProgress);

    if (InteractionActionCurrentHoldTime >= InteractionActionRequiredHoldDuration)
    {
        OnInteractionHeld();
        GetWorld()->GetTimerManager().ClearTimer(InteractionActionHoldTimerHandle);
    }
}

void AModularPlayerController_Master::OnInteractionHeld()
{
    UE_LOG(LogTemp, Log, TEXT("Interaction Held"));    
}

void AModularPlayerController_Master::OnCancelActionReleased()
{
    GetWorld()->GetTimerManager().ClearTimer(InteractionActionMashCountTimerHandle);
    InteractionActionCurrentMashCount = 0;
    bInteractionActionInMashingMode = false;
    UE_LOG(LogTemp, Log, TEXT("InteractionAction: Action cancelled"));
}

void AModularPlayerController_Master::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (APawn* ControlledPawn = GetPawn())
    {
        const FRotator ControlledRotation = ControlledPawn->GetControlRotation();
        const FRotator YawRotation(0.f, ControlledRotation.Yaw, 0.f);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
        ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AModularPlayerController_Master::Look(const FInputActionValue& Value)
{
    const FVector2D LookDirection = Value.Get<FVector2D>();
    if (APawn* ControlledPawn = GetPawn())
    {
        ControlledPawn->AddControllerYawInput(LookDirection.X);
        ControlledPawn->AddControllerPitchInput(LookDirection.Y);
    }
}

void AModularPlayerController_Master::OnJumpActionPressed()
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (ControlledPawn->GetClass()->ImplementsInterface(UModularInputReceiver::StaticClass()))
        {
            IModularInputReceiver::Execute_OnJumpActionPressed(ControlledPawn);
        }
    }
}

void AModularPlayerController_Master::OnCrouchActionPressed()
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (ControlledPawn->GetClass()->ImplementsInterface(UModularInputReceiver::StaticClass()))
        {
            IModularInputReceiver::Execute_OnCrouchActionPressed(ControlledPawn);
        }
    }
}

void AModularPlayerController_Master::OnCrouchActionReleased()
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (ControlledPawn->GetClass()->ImplementsInterface(UModularInputReceiver::StaticClass()))
        {
            IModularInputReceiver::Execute_OnCrouchActionReleased(ControlledPawn);
        }
    }
}

void AModularPlayerController_Master::OnCrouchToggle()
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (ControlledPawn->GetClass()->ImplementsInterface(UModularInputReceiver::StaticClass()))
        {
            IModularInputReceiver::Execute_OnCrouchToggle(ControlledPawn);
        }
    }
}

void AModularPlayerController_Master::OnInventoryToggle()
{
    UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Input(),
            TEXT("OnInventoryToggle called from PlayerController"), false, EDebugVerbosity::Info);
    
    // Forward to pawn if it implements the interface
    if (APawn* ControlledPawn = GetPawn())
    {
        if (ControlledPawn->Implements<UModularInputReceiver>())
        {
            IModularInputReceiver::Execute_OnInventoryToggle(ControlledPawn);
        }
    }
}

void AModularPlayerController_Master::OnESC_ContextAction()
{
    if (IsLocalPlayerController())
    {
        if (UInventoryWidgetManager* WidgetManager = UInventoryWidgetManager::Get(this))
        {
            if (WidgetManager->ActiveContextMenu)
            {
                WidgetManager->CloseActiveContextMenu();
            }
            else
            {
                // Need a way to get non-hud elements in widget mananger.
                //TODO:
                
                // if (WidgetManager->GetWidget())
                // {
                //     return;
                // }
                return;
            }
        }
    }
}


void AModularPlayerController_Master::OnSwitchCameraActionTriggered(const FInputActionValue& Value)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    if (ControlledPawn->GetClass()->ImplementsInterface(UModularInputReceiver::StaticClass()))
    {
        IModularInputReceiver::Execute_OnCameraToggle(ControlledPawn, ControlledPawn);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Switch Camera Action Triggered, early exit: ModularInputRecevier is not implemented for Pawn:%s"), *GetPawn()->GetName());
    }
}

void AModularPlayerController_Master::OnCameraZoomTriggered(const FInputActionValue& Value)
{
    float ScrollValue = Value.Get<float>();
    
    AMPC_PlayerCameraManager* CameraManager = Cast<AMPC_PlayerCameraManager>(PlayerCameraManager);
    if (CameraManager)
    {
        CameraManager->AdjustCameraDistance(ScrollValue);
    }
}

FString AModularPlayerController_Master::GetPlayerSaveID() const
{
    // Generate unique ID per player in multiplayer
    // Option 1: Use PlayerState UniqueId
    if (PlayerState)
    {
        return FString::Printf(TEXT("Player_%d"), PlayerState->GetPlayerId());
    }
    
    // Option 2: Use net player index
    return FString::Printf(TEXT("Player_%d"), NetPlayerIndex);
}

void AModularPlayerController_Master::OnQuickSave(const FInputActionValue& Value)
{
    // Only allow local player to trigger save
    if (!IsLocalController())
    {
        return;
    }
    
    UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_SaveSystem(), 
        TEXT("🎮 Local player requesting quick save..."), true, EDebugVerbosity::Info);
    
    // Call server RPC
    Server_QuickSave();
}

void AModularPlayerController_Master::Server_QuickSave_Implementation()
{
    // This runs on server for each player
    UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_SaveSystem(), 
        FString::Printf(TEXT("🖥️ Server saving for player: %s"), *GetPlayerSaveID()), 
        false, EDebugVerbosity::Info);
    
    // Get save subsystem (server-side only for authoritative saves)
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>())
        {
            FString PlayerID = GetPlayerSaveID();
            
            // Save current pawn state
            if (APawn* CurrentPawn = GetPawn())
            {
                if (AMPC_Character_Master* SavedCharacter = Cast<AMPC_Character_Master>(CurrentPawn))
                {
                    SavedCharacter->SaveCharacterState(PlayerID);
                }
                else if (AMPC_Pawn_Master* SavedPawn = Cast<AMPC_Pawn_Master>(CurrentPawn))
                {
                    SavedPawn->SavePawnState(PlayerID);
                }
                else if (AMPC_Vehicle_Master* Vehicle = Cast<AMPC_Vehicle_Master>(CurrentPawn))
                {
                    Vehicle->SaveVehicleState(PlayerID);
                }
            }

            
            // Update metadata with player-specific info
            SaveSubsystem->UpdateSaveMetadata(
                FString::Printf(TEXT("Player %s Quick Save"), *PlayerID),
                GetWorld()->GetMapName(),
                0.0f
            );
            
            // Use player-specific save slot
            FString SaveSlotName = FString::Printf(TEXT("QuickSave_%s"), *PlayerID);
            SaveSubsystem->SetCurrentSaveSlot(SaveSlotName);
            
            bool bSuccess = SaveSubsystem->SaveGame(SaveSlotName, false);
            
            if (bSuccess)
            {
                // Notify client
                Client_OnSaveComplete(true, SaveSlotName);
            }
            else
            {
                Client_OnSaveComplete(false, SaveSlotName);
            }
        }
    }
}

void AModularPlayerController_Master::OnQuickLoad(const FInputActionValue& Value)
{
    if (!IsLocalController())
    {
        return;
    }
    
    UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_SaveSystem(), 
        TEXT("🎮 Local player requesting quick load..."), true, EDebugVerbosity::Info);
    
    Server_QuickLoad();
}

void AModularPlayerController_Master::Server_QuickLoad_Implementation()
{
    UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_SaveSystem(), 
        FString::Printf(TEXT("🖥️ Server loading for player: %s"), *GetPlayerSaveID()), 
        false, EDebugVerbosity::Info);
    
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>())
        {
            FString PlayerID = GetPlayerSaveID();
            FString SaveSlotName = FString::Printf(TEXT("QuickSave_%s"), *PlayerID);
            
            // Check if player's save exists
            if (!SaveSubsystem->DoesSaveExist(SaveSlotName))
            {
                Client_OnLoadComplete(false, SaveSlotName);
                return;
            }
            
            // Load the save
            SaveSubsystem->SetCurrentSaveSlot(SaveSlotName);
            bool bSuccess = SaveSubsystem->LoadGame(SaveSlotName);
            
            if (bSuccess)
            {
                // Load pawn state
                if (APawn* CurrentPawn = GetPawn())
                {
                    if (AMPC_Character_Master* LoadedCharacter = Cast<AMPC_Character_Master>(CurrentPawn))
                    {
                        LoadedCharacter->LoadCharacterState(PlayerID);
                    }
                    else if (AMPC_Pawn_Master* LoadedPawn = Cast<AMPC_Pawn_Master>(CurrentPawn))
                    {
                        LoadedPawn->LoadPawnState(PlayerID);
                    }
                    else if (AMPC_Vehicle_Master* Vehicle = Cast<AMPC_Vehicle_Master>(CurrentPawn))
                    {
                        Vehicle->LoadVehicleState(PlayerID);
                    }
                }

                
                Client_OnLoadComplete(true, SaveSlotName);
            }
            else
            {
                Client_OnLoadComplete(false, SaveSlotName);
            }
        }
    }
}

// Client RPCs for feedback
void AModularPlayerController_Master::Client_OnSaveComplete_Implementation(bool bSuccess, const FString& SlotName)
{
    if (bSuccess)
    {
        UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_SaveSystem(), 
            FString::Printf(TEXT("✅ Quick Save Complete! Slot: %s"), *SlotName), 
            true, EDebugVerbosity::Info);
    }
    else
    {
        UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_SaveSystem(), 
            TEXT("❌ Quick Save Failed!"), true, EDebugVerbosity::Error);
    }
}

void AModularPlayerController_Master::Client_OnLoadComplete_Implementation(bool bSuccess, const FString& SlotName)
{
    if (bSuccess)
    {
        UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_SaveSystem(), 
            FString::Printf(TEXT("✅ Quick Load Complete! Slot: %s"), *SlotName), 
            true, EDebugVerbosity::Info);
    }
    else
    {
        UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_SaveSystem(), 
            TEXT("❌ Quick Load Failed! No save found."), true, EDebugVerbosity::Warning);
    }
}

void AModularPlayerController_Master::ProcessPlayerInput(const float DeltaTime, const bool bGamePaused)
{
    Super::ProcessPlayerInput(DeltaTime, bGamePaused);
    
    // Check if left mouse button was just pressed
    if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
    {
        if (UInventoryWidgetManager* WidgetManager = UInventoryWidgetManager::Get(this))
        {
            if (UUserWidget* ActiveMenu = WidgetManager->GetActiveContextMenu())
            {
                // CRITICAL: Check if widget is in viewport and geometry is valid
                if (!ActiveMenu->IsInViewport())
                {
                    return;
                }
                
                // Get cached geometry and validate it
                FGeometry MenuGeometry = ActiveMenu->GetCachedGeometry();
                if (!MenuGeometry.HasRenderTransform())
                {
                    // Geometry not ready yet (widget just added to viewport)
                    return;
                }
                
                // Get mouse position
                FVector2D MousePos;
                GetMousePosition(MousePos.X, MousePos.Y);
                
                // Convert to local space
                FVector2D LocalMousePos = MenuGeometry.AbsoluteToLocal(MousePos);
                FVector2D MenuSize = MenuGeometry.GetLocalSize();
                
                // Check if size is valid (not 0,0)
                if (MenuSize.X <= 0 || MenuSize.Y <= 0)
                {
                    // Widget hasn't been laid out yet
                    return;
                }
                
                // Check if click is OUTSIDE the menu
                if (LocalMousePos.X < 0 || LocalMousePos.X > MenuSize.X ||
                    LocalMousePos.Y < 0 || LocalMousePos.Y > MenuSize.Y)
                {
                    WidgetManager->CloseActiveContextMenu();
                }
            }
        }
    }
}

