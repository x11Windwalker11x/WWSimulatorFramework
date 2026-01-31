// Fill out your copyright notice in the Description page of Project Settings.

/*
 *This interface was once in modular interaction system but since it is tied close with controller and pawns
 *it was better to put it here for logistical reasons
 * Handles all info that should be carried from pawn to interactable object. 
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Components/ActorComponent.h"
#include "Components/PhysicsGrabComponent.h"
#include "AIController.h"
#include "InteractorInterface.generated.h"

class UInteractorComponent;



// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API IInteractorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	virtual UClass* GetInteractorClass();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	AAIController* GetInteractorAIController();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	APlayerController* GetInteractorPlayerController();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	APawn* GetInteractorPawn();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	ACharacter* GetInteractorCharacter();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UActorComponent* GetInteractorComponentAsActorComponent(TSubclassOf<UActorComponent> ComponentClass = nullptr);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	UActorComponent* GetInteractorComponent();
	/* Handles PhysicsGrabComponent in simulator framework*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor | Specs")
	UActorComponent* GetPhysicsGrabComponentAsActorComponent();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor | Specs")
	UPhysicsGrabComponent* GetPhysicsGrabComponent();



	// Use BlueprintNativeEvent for both C++ and BP implementation
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor States")
	bool GetbCanInteract();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor Specs")
	void OnCameraToggle(APawn* ControlledPawn);

};
