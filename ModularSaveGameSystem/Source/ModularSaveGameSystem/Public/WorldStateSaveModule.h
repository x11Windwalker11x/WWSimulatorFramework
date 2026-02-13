// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MasterSaveGame.h"
#include "Lib/Data/ModularSaveGameSystem/ActorSaveData.h"
#include "WorldStateSaveModule.generated.h"

/**
 * Wrapper struct for TMap<FString, FActorSaveEnvelope> so it can be a UPROPERTY value type.
 * UE5 doesn't support TMap<K, TMap<K2,V2>> directly as UPROPERTY.
 */
USTRUCT()
struct FActorSaveEnvelopeMap
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TMap<FString, FActorSaveEnvelope> Actors;
};

/**
 * UWorldStateSaveModule — Dedicated per-level, per-actor save module.
 * Stores FActorSaveEnvelope data keyed by LevelName -> ActorSaveID.
 * Integrated into MasterSaveGame via the modular save data system.
 *
 * Phase B: Level-placed actors (identity via GetPathName).
 * Phase C (future): Runtime-spawned actors (identity via FGuid).
 */
UCLASS(BlueprintType)
class MODULARSAVEGAMESYSTEM_API UWorldStateSaveModule : public UModularSaveData
{
	GENERATED_BODY()

public:
	UWorldStateSaveModule();

	// ========== Actor State API ==========

	/** Store an actor's save envelope. Overwrites if ActorSaveID already exists for this level. */
	UFUNCTION(BlueprintCallable, Category = "World State Save")
	void SaveActorState(const FActorSaveEnvelope& Envelope);

	/** Load a specific actor's save envelope by level and actor ID.
	 *  @return Empty envelope (IsValid()==false) if not found. */
	UFUNCTION(BlueprintCallable, Category = "World State Save")
	FActorSaveEnvelope LoadActorState(const FString& LevelName, const FString& ActorSaveID) const;

	/** Get all saved actor envelopes for a specific level. */
	UFUNCTION(BlueprintCallable, Category = "World State Save")
	TArray<FActorSaveEnvelope> GetAllActorsForLevel(const FString& LevelName) const;

	/** Remove a specific actor's save data. */
	UFUNCTION(BlueprintCallable, Category = "World State Save")
	bool RemoveActorState(const FString& LevelName, const FString& ActorSaveID);

	/** Check if save data exists for this actor. */
	UFUNCTION(BlueprintPure, Category = "World State Save")
	bool HasActorState(const FString& LevelName, const FString& ActorSaveID) const;

	/** Clear all saved data for a specific level. */
	UFUNCTION(BlueprintCallable, Category = "World State Save")
	void ClearLevel(const FString& LevelName);

	/** Get all level names that have saved actor data. */
	UFUNCTION(BlueprintCallable, Category = "World State Save")
	TArray<FString> GetSavedLevelNames() const;

	// ========== UModularSaveData Overrides ==========

	virtual void ClearData() override;
	virtual bool ValidateData_Implementation() override;
	virtual void MigrateData_Implementation(int32 FromVersion, int32 ToVersion) override;

private:
	/** Outer key: LevelName -> Inner: ActorSaveID -> FActorSaveEnvelope */
	UPROPERTY(SaveGame)
	TMap<FString, FActorSaveEnvelopeMap> WorldActors;

	static const int32 CURRENT_MODULE_VERSION = 1;
};
