// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h" 
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/SecureHash.h"
#include "GameplayTagContainer.h"
#include "Subsystems/InventoryWidgetManager.h"
#include "WW_TagLibrary.h"
#include "Input/InputSettingsDataAsset.h"
#include "Camera/Manager/MPC_PlayerCameraManager.h"
#include "Pawns/MPC_Pawn_Master.h"
#include "Pawns/MPC_Character_Master.h"
#include "PersistentPlayerData.h"
#include "InteractionSubsystem.h"
#include "EcheatPermissionLevel.h"
#include "Interface/InteractionSystem/InteractorInterface.h"
#include "ChaosModularVehicle/ModularVehicleClusterPawn.h"
#include "MPC_PlayerState.h"
#include "Net/UnrealNetwork.h"

#include "ModularPlayerController_Master.generated.h"

UCLASS()
class MODULARPLAYERCONTROLLER_API AModularPlayerController_Master : public APlayerController, public IInteractorInterface
{
    GENERATED_BODY()

public:
    AModularPlayerController_Master();
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;


    virtual bool GetbCanInteract_Implementation() override;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Cheats")
    ECheatPermissionLevel CheatPermissionLevel;
    
    UPROPERTY(VisibleAnywhere, Category = "Cheats")
    FString MachineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input | Config")
    FGameplayTagContainer InputModeTags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input | State")
    FGameplayTagContainer InputStateTags;

    UFUNCTION()
    void TryInteract();
    
    // Optimized RPC with compressed types for bandwidth efficiency
    UFUNCTION(Server, Reliable)
    void Server_TryInteract(FVector_NetQuantize10 ViewOffset, uint16 CompressedPitch, uint16 CompressedYaw, uint8 PredictionID);
    
    FString GetPersistentPlayerID();
    FString GetHardwareBasedID();
    FString GetOrCreatePersistentID();
    
    
    UFUNCTION(BlueprintCallable, Category = "Cheat")
    ECheatPermissionLevel GetCheatPermission();
    
    UFUNCTION(BlueprintCallable, Category = "Cheat")
    void SetCheatPermission(ECheatPermissionLevel NewCheatPermissionLevel);
    
    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    virtual void PossessChaosVehicle(AModularVehicleClusterPawn* Vehicle);
    
    virtual void SetupInputComponent() override;
    
    UFUNCTION(BlueprintCallable, Category = "Input")
    void RefreshInputBindings();
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* MPC_IMC_Character_Direct = nullptr;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* MPC_IMC_Vehicle_Direct = nullptr;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* MPC_IMC_Pawn_Direct = nullptr;
    
    void HandleInteract();
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input | Interaction")
    UInputAction* InteractionAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input | Config | Interaction");
    float InteractionActionRequiredHoldDuration = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input | Config | Interaction");
    int32 InteractionActionRequiredMashCount = 5;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input | Config | Interaction")
    float InteractionActionMashTimeWindow = 3.0f;

    bool bInteractionActionInMashingMode = false;
    bool bInteractionActionActionCancelled = false;
    
    void OnInteractionHold();
    void OnInteractionPressed(const FInputActionValue& Value);
    void OnInteractionReleased();
    void OnInteractionHeld();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input | SaveLoad")
    UInputAction* QuickSaveAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input | SaveLoad")
    UInputAction* QuickLoadAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input | Movement")
    UInputAction* MovementAction;
    
    UFUNCTION(BlueprintCallable, Category = "Input | Movement")
    void Move(const FInputActionValue& Value);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input | Look")
    UInputAction* LookAction;
    
    UFUNCTION(BlueprintCallable, Category = "Input | Look")
    void Look(const FInputActionValue& Value);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input | Interaction")
    UInputAction* CancelAction;
    
    UFUNCTION(BlueprintCallable, Category = "Input | Interaction")
    void OnCancelActionReleased();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input | Movement");
    UInputAction* JumpAction;
    
    UFUNCTION(BlueprintCallable, Category = "Input | Movement")
    void OnJumpActionPressed();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input | Movement");
    UInputAction* CrouchAction;
    
    UFUNCTION(BlueprintCallable, Category = "Input | Movement")
    void OnCrouchActionPressed();
    
    UFUNCTION(BlueprintCallable, Category = "Input | Movement")
    void OnCrouchActionReleased();
    
    UFUNCTION(BlueprintCallable, Category = "Input | Movement")
    void OnCrouchToggle();

    UFUNCTION(BlueprintCallable, Category = "Input | Movement")
    void OnInventoryToggle();
    
    UFUNCTION(BlueprintCallable, Category = "Input | Movement")
    void OnESC_ContextAction();
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input | Camera")
    UInputAction* SwitchCameraAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input | Camera")
    UInputAction* CameraZoomAction;

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void OnSwitchCameraActionTriggered(const FInputActionValue& Value);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void OnCameraZoomTriggered(const FInputActionValue& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Input")
    bool ShouldUseCrouchToggle() const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input | Inventory")
    UInputAction* InventoryAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input | Inventory")
    UInputAction* ESCContextAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings", Meta = (Tooltip = "Derived from DA_InputSettings, profile based, used for toggle and hold modes for inputs (the rest will be implemented)"))
    TObjectPtr<UInputSettingsDataAsset> InputSettingsAsset;
    
    UFUNCTION(BlueprintCallable, Category = "Input Settings")
    void LoadInputSettings();



protected:
    void OnQuickSave(const FInputActionValue& Value);
    void OnQuickLoad(const FInputActionValue& Value);
    UFUNCTION(Server, Reliable)
    void Server_QuickSave();
    
    UFUNCTION(Server, Reliable)
    void Server_QuickLoad();

    virtual void ProcessPlayerInput(const float DeltaTime, const bool bGamePaused) override;
    
    // Helper to get unique player ID
    FString GetPlayerSaveID() const;
    
    // Client feedback RPCs
    UFUNCTION(Client, Reliable)
    void Client_OnSaveComplete(bool bSuccess, const FString& SlotName);
    
    UFUNCTION(Client, Reliable)
    void Client_OnLoadComplete(bool bSuccess, const FString& SlotName);
private:
    bool InteractionActionIsHolding = false;
    float InteractionActionCurrentHoldTime = 0.0f;
    float InteractionActionHoldTimerDelay = 0.2f;
    FTimerHandle InteractionActionHoldTimerHandle;
    
    int32 InteractionActionCurrentMashCount = 0;
    FTimerHandle InteractionActionMashCountTimerHandle;

    void OnInteractionActionMashTimeExpired();
};