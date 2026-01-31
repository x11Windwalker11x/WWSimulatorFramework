// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//Components

#include "Pawns/Components/InputActionMechanicComponent.h"

#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MPC_PlayerCameraManager.h"
#include "Components/PawnComponentManager.h"

// Native Componnets
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"


// Interfaces
#include "Interface/InteractionSystem/InteractorInterface.h"
#include "Interface/PlayerControllerSystem/ModularInputReceiver.h"
#include "Interface/InteractionSystem/InteractableInterface.h"

//SaveGameSystem
#include "MasterSaveSubsystem.h"
#include "CharacterSaveModule.h"
#include "InventorySaveModule.h"
#include "AbilitiesSaveModule.h"
//Logging
#include "Logging/InteractableInventoryLogging.h"
#include "WW_TagLibrary.h"
#include "GameplayTagsManager.h"
#include "AIController.h"
#include "Interface/InventorySystem/InventoryInterface.h"
#include "MPC_Pawn_Master.generated.h"

UCLASS()
class MODULARPLAYERCONTROLLER_API AMPC_Pawn_Master : public APawn, public IModularInputReceiver, public IInteractorInterface, public IInteractableInterface, public IInventoryInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMPC_Pawn_Master();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every fram
	virtual void Tick(float DeltaTime) override;

	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnComponentManager* PawnComponentManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SM_GFX;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* SK_GFX;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* InRootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInputActionMechanicComponent* InputActionMechanicComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInteractorComponent* InteractorComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryCompopent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInteractableComponent* InteractableComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnMovementComponent* PawnMovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;
	//To fit in the framework, set to null
	
	//vars
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction states")
	bool bCanInteract;

	//Controller
	APlayerController* OwnerPlayerController;
	AAIController* OwnerAIController;

	//Savegame
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void SavePawnState(const FString& PlayerID);
    
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void LoadPawnState(const FString& PlayerID);


	//Input
	// Input: Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//Input: HandBrake
	virtual void OnJumpActionPressed_Implementation() override;

	// IInteractorInterface Implementations
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
	UActorComponent* GetInteractorComponentAsActorComponent(TSubclassOf<UActorComponent> ComponentClass) ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UActorComponent* GetInteractorComponent() ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UActorComponent* GetInventoryComponentActorComponent(TSubclassOf<UActorComponent> ComponentClass) ;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UInventoryComponent* GetInventoryComponent();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UInteractableComponent* GetInteractableComponent();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UActorComponent* GetInteractableComponentAsActorComponent(TSubclassOf<UActorComponent> ComponentClass);
	
	
	virtual bool GetbCanInteract_Implementation() override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	void OnCameraToggle(APawn* ControlledPawn);

	// IInteractableInterface Implementations
	virtual void OnInteract_Implementation(AController* InstigatorController) override;
	virtual void OnPreInteractionEntered_Implementation(APawn* OtherPawn) override;
	virtual void OnFullInteractionEntered_Implementation(APawn* OtherPawn) override;
	virtual void OnPreInteractionExited_Implementation(APawn* OtherPawn) override;
	virtual void OnFullInteractionExited_Implementation(APawn* OtherPawn) override;
	virtual bool IsCurrentlyInteractable_Implementation() override;
	virtual void SetInteractionEnabled_Implementation(bool bIsEnabled) override;
	virtual bool GetInteractionEnabled_Implementation() override;
	virtual void SetInteractionNotifications_Implementation() override;
	virtual bool GetbInteractionNotificationsAllowManualHandling_Implementation() override;
	virtual void LoadAssets_Implementation() override;
	virtual void UnloadAssets_Implementation() override;
	virtual FGameplayTagContainer GetGameplayTags_Implementation() override;
	virtual void OnPlayerNearbyInit_Implementation(APawn* PlayerPawn, const bool bIsNearby) override;

	
	

	//Bind CameraManager
	virtual void PossessedBy(AController* NewController) override;
};