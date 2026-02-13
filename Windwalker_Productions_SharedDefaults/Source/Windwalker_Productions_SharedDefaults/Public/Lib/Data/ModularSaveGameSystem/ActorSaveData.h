// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ActorSaveData.generated.h"

/**
 * FComponentSaveRecord — Per-component binary save data within an actor envelope.
 * Each component implementing ISaveableInterface produces one of these.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FComponentSaveRecord
{
	GENERATED_BODY()

	/** Component's SaveID (e.g. "InventoryComponent") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString ComponentSaveID;

	/** UClass name for validation on load */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName ComponentClass = NAME_None;

	/** Component's SaveGame properties serialized via UE binary archive */
	UPROPERTY()
	TArray<uint8> BinaryData;

	/** Data version for migration support */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 Version = 1;

	bool IsValid() const { return !ComponentSaveID.IsEmpty(); }
};

/**
 * FActorSaveEnvelope — Complete actor save package: actor data + all component data.
 * Used by WorldStateSaveModule to store per-actor state.
 *
 * Level-placed actors: ActorSaveID = GetPathName() (deterministic).
 * Runtime-spawned (Phase C): ActorSaveID = FGuid string.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FActorSaveEnvelope
{
	GENERATED_BODY()

	/** Unique actor identifier — GetPathName() for level-placed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString ActorSaveID;

	/** Actor class for runtime re-spawn (Phase C) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FSoftClassPath ActorClass;

	/** Actor world transform (position/rotation/scale) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FTransform ActorTransform;

	/** Owning level/sublevel name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString LevelName;

	/** Save type: Save.Type.LevelPlaced or Save.Type.RuntimeSpawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGameplayTag SaveType;

	/** Custom actor state tags (open/closed, enabled/disabled, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGameplayTagContainer ActorStateTags;

	/** Actor's own SaveGame properties serialized via UE binary archive */
	UPROPERTY()
	TArray<uint8> ActorBinaryData;

	/** Component save records keyed by component SaveID */
	UPROPERTY()
	TMap<FString, FComponentSaveRecord> ComponentRecords;

	/** True if this level-placed actor was destroyed by gameplay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bIsDestroyed = false;

	/** Timestamp of when this envelope was saved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FDateTime SaveTimestamp;

	bool IsValid() const { return !ActorSaveID.IsEmpty(); }
};
