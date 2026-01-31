// SpawnData.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SpawnData.generated.h"

USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FSpawnRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FTransform SpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FGameplayTagContainer SpawnTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TWeakObjectPtr<AActor> Owner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bUsePooling = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName PoolID = NAME_None;
};