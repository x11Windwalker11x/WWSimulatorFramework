// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "InteractionSubsystem.h"
#include "ModularAIController_Master.generated.h"

UCLASS(Abstract)
class MODULARPLAYERCONTROLLER_API AModularAIController_Master : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AModularAIController_Master();
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
