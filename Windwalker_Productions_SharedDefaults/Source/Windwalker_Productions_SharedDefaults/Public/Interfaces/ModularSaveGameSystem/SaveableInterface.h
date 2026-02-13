// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/ModularSaveGameSystem/SaveData.h"
#include "SaveableInterface.generated.h"

/**
 * ISaveableInterface — L0 contract for any object that participates in the save system.
 * Implemented by actors and components that need persistence.
 *
 * Golden Rules #37-40:
 * - #37: Two-tier delegate (Character vs World)
 * - #38: Unique SaveID per saveable
 * - #39: Priority-ordered load (lower number = loads first)
 * - #40: Dirty tracking — only save changed objects
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USaveableInterface : public UInterface
{
	GENERATED_BODY()
};

class ISaveableInterface
{
	GENERATED_BODY()

public:
	// ================================================================
	// IDENTITY
	// ================================================================

	/** Returns a unique persistent identifier for this saveable object.
	 *  Level-placed actors: GetPathName() (deterministic).
	 *  Runtime-spawned: FGuid (Phase C). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save System")
	FString GetSaveID() const;

	/** Returns the load priority. Lower = loads first.
	 *  Actors: 0-49, Components: 100-149. Rule #39. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save System")
	int32 GetSavePriority() const;

	/** Returns the save type tag (Save.Type.LevelPlaced, Save.Type.RuntimeSpawned, Save.Type.PlayerData). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save System")
	FGameplayTag GetSaveType() const;

	// ================================================================
	// SERIALIZATION (Binary via UE archive)
	// ================================================================

	/** Serialize this object's SaveGame properties into OutRecord.
	 *  Uses FMemoryWriter + FObjectAndNameAsStringProxyArchive with ArIsSaveGame=true. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save System")
	bool SaveState(FSaveRecord& OutRecord);

	/** Deserialize state from InRecord back into this object.
	 *  Calls OnSaveDataLoaded() after successful restore. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save System")
	bool LoadState(const FSaveRecord& InRecord);

	// ================================================================
	// DIRTY TRACKING (Rule #40)
	// ================================================================

	/** Returns true if this object has unsaved changes since last ClearDirty(). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save System")
	bool IsDirty() const;

	/** Clears the dirty flag. Called by save system after successful save. */
	UFUNCTION(BlueprintNativeEvent, Category = "Save System")
	void ClearDirty();

	// ================================================================
	// POST-LOAD CALLBACK (AAA pattern)
	// ================================================================

	/** Called after state has been fully restored from save data.
	 *  Use to update visuals, rebuild caches, re-enable systems. */
	UFUNCTION(BlueprintNativeEvent, Category = "Save System")
	void OnSaveDataLoaded();
};
