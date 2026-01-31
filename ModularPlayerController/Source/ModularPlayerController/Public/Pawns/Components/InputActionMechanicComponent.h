// Fill out your copyright notice in the Description page of Project Settings.
/*
 * A component to process inputs into game mechanics like mash hold etc...
 * A mediator between ModularPlayerController and And its pawn...
 * All inputs if they have specific mechanic other than a simple press and release should
 * go through that component.
 */

/*
 *Mediator between ModularPlayerController and its pawns:
 * its job is to handle input mechanics like hold mash qte events,
 * and then take the result. to the controlled pawn.
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionMechanicComponent.generated.h"


UCLASS( ClassGroup=(Components, Input), meta=(BlueprintSpawnableComponent = "false", ToolTip="Handles input action mechanics like button mashing and holding",
	ShortToolTip="Input mechanics"))
class MODULARPLAYERCONTROLLER_API UInputActionMechanicComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInputActionMechanicComponent();
	// Every action can have mash and hold...
	UFUNCTION()
	void OnMash();
	UFUNCTION()
	void OnMashSuccessful();
	UFUNCTION()
	void OnMeshFailed();
	UFUNCTION()
	void OnHold();
	UFUNCTION()
	void OnHeld();
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
