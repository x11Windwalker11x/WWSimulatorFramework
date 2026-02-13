// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SaveData.generated.h"

/**
 * FSaveRecord — Binary-first save data container (AAA-aligned).
 * Uses TArray<uint8> for UE Serialize() output (NOT JSON string).
 * Each saveable object produces one FSaveRecord via ISaveableInterface::SaveState().
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FSaveRecord
{
	GENERATED_BODY()

	/** Unique identifier for this record (matches ISaveableInterface::GetSaveID()) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName RecordID = NAME_None;

	/** Save type/category tag (Save.Type.*, Save.Category.*) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGameplayTag RecordType;

	/** Binary serialized data from FMemoryWriter + FObjectAndNameAsStringProxyArchive.
	 *  This is the UE standard: 10-50x faster than JSON, auto-discovers UPROPERTY(SaveGame). */
	UPROPERTY()
	TArray<uint8> BinaryData;

	/** Timestamp of when this record was created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FDateTime Timestamp;

	/** Data version for migration support */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 Version = 1;

	/** Load priority — lower values load first. Rule #39.
	 *  Actors: 0-49, Components: 100-149. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 Priority = 100;

	bool IsValid() const { return RecordID != NAME_None; }
};
