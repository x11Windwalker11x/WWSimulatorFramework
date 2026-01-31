// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PersistentPlayerData.generated.h"

/**
 * 
 */
UCLASS()
class MODULARPLAYERCONTROLLER_API UPersistentPlayerData : public USaveGame
{
	GENERATED_BODY()
public:
	UPersistentPlayerData();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Data")
	FString PlayerID; 
};
