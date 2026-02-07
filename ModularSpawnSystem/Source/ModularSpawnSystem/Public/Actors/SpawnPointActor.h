// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/ModularSpawnSystem/SpawnData.h"
#include "GameFramework/Actor.h"
#include "SpawnPointActor.generated.h"

/**
 * Level-placed spawn point actor.
 * Spawns configured actors on BeginPlay or on-demand, with optional respawn.
 */
UCLASS()
class MODULARSPAWNSYSTEM_API ASpawnPointActor : public AActor
{
	GENERATED_BODY()

public:
	ASpawnPointActor();

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Spawn point configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point")
	FSpawnPointConfig Config;

	/** Spawn actors when play begins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point")
	bool bSpawnOnBeginPlay = true;

	/** ItemID to initialize spawned actors with (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point")
	FName ItemID = NAME_None;

	// ============================================================================
	// API
	// ============================================================================

	/** Manually trigger spawning */
	UFUNCTION(BlueprintCallable, Category = "Spawn Point")
	void SpawnActors();

	/** Get currently alive actors from this spawn point */
	UFUNCTION(BlueprintPure, Category = "Spawn Point")
	TArray<AActor*> GetAliveActors() const;

	/** Get number of currently alive actors */
	UFUNCTION(BlueprintPure, Category = "Spawn Point")
	int32 GetAliveCount() const;

protected:
	virtual void BeginPlay() override;

	/** Track spawned actors */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SpawnedActors;

	/** Respawn timer handle */
	FTimerHandle RespawnTimerHandle;

	/** Called when a spawned actor is destroyed */
	UFUNCTION()
	void OnSpawnedActorDestroyed(AActor* DestroyedActor);

	/** Respawn check after delay */
	void TryRespawn();
};
