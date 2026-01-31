// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularAIController_Master.h"

#include "Logging/InteractableInventoryLogging.h"


// Sets default values
AModularAIController_Master::AModularAIController_Master()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AModularAIController_Master::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AModularAIController_Master::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AModularAIController_Master::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
    
	if (UWorld* World = GetWorld())
	{
		if (UInteractionSubsystem* Sys = UInteractionSubsystem::Get(World))
		{
			Sys->RegisterAIPawn(InPawn);
			// 🆕 ADD THIS DEBUG LOG
			UE_LOG(LogInteractableSubsystem, Log, TEXT("✅ AI: %s registered with InteractableSubsystem"), 
 				*InPawn->GetName());

		}
		else
			UE_LOG(LogInteractableSubsystem, Log, TEXT("Failed to get InteractableSubsystem"));
	}
}

void AModularAIController_Master::OnUnPossess()
{
	if (APawn* PossessedPawn = GetPawn())
	{
		if (UWorld* World = GetWorld())
		{
			if (UInteractionSubsystem* Sys = UInteractionSubsystem::Get(World))
			{
				Sys->UnregisterAIPawn(PossessedPawn);
			}
		}
	}
    
	Super::OnUnPossess();
}

