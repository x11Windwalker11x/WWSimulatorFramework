// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//Native Compoments & Actors
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

//Comps
#include "Components/MPC_PlayerCameraManager.h"
#include "Components/InventoryComponent.h"
#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"
#include "Pawns/Components/InputActionMechanicComponent.h"
#include "Components/PawnComponentManager.h"

//Interfaces
#include "Interface/InventorySystem/InventoryInterface.h"
#include "Interface/InteractionSystem/InteractableInterface.h"
#include "Interface/InteractionSystem/InteractorInterface.h"
#include "Interface/PlayerControllerSystem/ModularInputReceiver.h"
//SaveGame
#include "MasterSaveSubsystem.h"
#include "CharacterSaveModule.h"
#include "InventorySaveModule.h"
#include "AbilitiesSaveModule.h"


#include "MPC_Vehicle_Master.generated.h"

UCLASS(Abstract)
class MODULARPLAYERCONTROLLER_API AMPC_Vehicle_Master : public APawn, public IModularInputReceiver, public IInteractorInterface, public IInteractableInterface, public IInventoryInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMPC_Vehicle_Master();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//SaveGame
	// Save/Load functions
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void SaveVehicleState(const FString& PlayerID);
    
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void LoadVehicleState(const FString& PlayerID);
	
	//Input
		//Input: Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
		//Input: Handbrake action
	//virtual void OnJumpActionPressed() override;
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnComponentManager* PawnComponentManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* InRootComponent;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* SK_GFX;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (ToolTip = "A Mediator between Player Controller and its Pawn. Any input other than press and release should go through this component."))
	UInputActionMechanicComponent* InputActionMechanicComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInteractorComponent* InteractorComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInteractableComponent* InteractableComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryComponent;
	//Components:Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (ToolTip = "A Mediator between Player Controller and its Pawn. Any input other than press and release should go through this component."))
	UCameraComponent* CameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (ToolTip = "A Mediator between Player Controller and its Pawn. Any input other than press and release should go through this component."))
	USpringArmComponent* CameraBoom;

	//Controller
	APlayerController* OwnerPlayerController;
	AAIController* OwnerAIController;
	//Camera Manager
	AMPC_PlayerCameraManager* OwnerPlayerCameraManager;
	
	//Possession
	virtual void PossessedBy(AController* NewController) override;
	
	

	// IInteractorInterface Implementations
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	 UClass* GetInteractorClass() ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	 AController* GetInteractorController() ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	 APlayerController* GetInteractorPlayerController() ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	 APawn* GetInteractorPawn() ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	 ACharacter* GetInteractorCharacter() ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	 UActorComponent* GetInteractorComponentAsActorComponent(TSubclassOf<UActorComponent> ComponentClass) ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UInventoryComponent* GetInventoryComponent() ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UActorComponent* GetInventoryComponentAsActorComponent();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input")
	void OnCameraToggle(APawn* ControlledPawn);
};

