// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//Native Comps
#include "GameplayTagsManager.h"
#include "GameplayTags.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/CrafterComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

//Controllers 
#include "AIController.h"

//Comps
#include "Camera/Manager/MPC_PlayerCameraManager.h"

#include "Pawns/Components/InputActionMechanicComponent.h"

#include "Components/InteractorComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/PhysicsGrabComponent.h"
#include "Components/PawnComponentManager.h"

//Interfaces
#include "Interfaces/ModularInteractionSystem/InteractorInterface.h"
#include "Interfaces/ModularInventorySystem/InventoryInterface.h"
#include "Interfaces/ModularInventorySystem/CraftingInterface.h"
#include "Interfaces/ModularPlayerController/ModularInputReceiver.h"

//Logging
#include "Logging/InteractableInventoryLogging.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

//SaveGame
#include "MasterSaveSubsystem.h"
#include "CharacterSaveModule.h"
#include "InventorySaveModule.h"
#include "AbilitiesSaveModule.h"
#include "Interfaces/ModularInteractionSystem/InteractorInterface.h"
#include "Interfaces/ModularInventorySystem/CraftingInterface.h"
#include "Interfaces/ModularInventorySystem/InventoryInterface.h"
#include "Interfaces/ModularPlayerController/ModularInputReceiver.h"
#include "MPC_Character_Master.generated.h"

UCLASS(Abstract)
class MODULARPLAYERCONTROLLER_API AMPC_Character_Master : public ACharacter, public IModularInputReceiver, public IInteractorInterface, public IInventoryInterface, public ICraftingInterface
{
	GENERATED_BODY()

	

protected:
	virtual void BeginPlay() override;
	
	void ToggleCrouch();
	void StartCrouch();
	void StopCrouch();
	bool IsCrouching() const;

public:
	AMPC_Character_Master();
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	
	virtual void Tick(float DeltaTime) override;
	
	// Save/Load functions
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void SaveCharacterState(const FString& PlayerID);
    
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void LoadCharacterState(const FString& PlayerID);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Specs")
	bool bCanInteract;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Stance")
	FGameplayTagContainer MovementGameplayTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UCrafterComponent* CrafterComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UInteractorComponent* InteractorComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UPhysicsGrabComponent* PhysicsGrabComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (ToolTip = "A Mediator between Player Controller and its Pawn. Any input other than press and release should go through this component."))
	UInputActionMechanicComponent* InputActionMechanicComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	APlayerController* OwnerPlayerController;
	
	//TODO: make owneraicontroller from the property ai controller class in class defaults
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	AAIController* OwnerAIController;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Interface Funcs
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UClass* GetInteractorClass();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	AController* GetInteractorController();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	APlayerController* GetInteractorPlayerController();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	APawn* GetInteractorPawn();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	ACharacter* GetInteractorCharacter();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UActorComponent* GetInteractorComponentAsActorComponent(TSubclassOf<UActorComponent> ComponentClass);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UInventoryComponent* GetInventoryComponent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UActorComponent* GetCrafterComponent();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	FName GetCrafterID();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	FGameplayTagContainer GetCrafterTags();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	bool GetbCanInteract();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	void OnCameraToggle(APawn* ControlledPawn);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor | Specs")
	UActorComponent* GetPhysicsGrabComponentAsActorComponent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor | Specs")
	UPhysicsGrabComponent* GetPhysicsGrabComponent();
	
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnComponentManager* PawnComponentManager;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	AAIController* GetInteractorAIController();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UActorComponent* GetInteractorComponent();

	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	UActorComponent* GetInventoryComponentAsActorComponent(TSubclassOf<UActorComponent> ComponentClass);

	// Input Receive Interface overrides
	virtual void OnJumpActionPressed_Implementation() override;
	virtual void OnCrouchActionPressed_Implementation() override;
	virtual void OnCrouchActionReleased_Implementation() override;
	virtual void OnCrouchToggle_Implementation() override;
	virtual void OnInventoryToggle_Implementation() override;



	
};