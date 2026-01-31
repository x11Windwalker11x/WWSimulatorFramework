// SaveData.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SaveData.generated.h"

USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FSaveRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName RecordID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGameplayTag RecordType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString SerializedData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 Version = 1;
};