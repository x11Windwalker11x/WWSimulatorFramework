// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ModularInputReceiver.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UModularInputReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MODULARPLAYERCONTROLLER_API IModularInputReceiver
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input | States")
	void OnJumpActionPressed();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input | States")
	void OnCrouchActionPressed();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input | States")
	void OnCrouchActionReleased();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input")
	void OnCameraToggle(APawn* ControlledPawn);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input")
	void OnCrouchToggle();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input")
	void OnInventoryToggle();
};
