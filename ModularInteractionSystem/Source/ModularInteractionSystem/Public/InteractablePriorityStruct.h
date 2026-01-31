#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "InteractablePriorityStruct.generated.h"

USTRUCT(BlueprintType)
struct FGameplayTagPriorityRow : public FTableRowBase
{
	GENERATED_BODY()

	// Tag to identify the interactable
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;

	// Priority value, higher = more important
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DevComment;

	//TODO: Create a new struct with complete info about items and interactables... rename them into InteractableStruct
};