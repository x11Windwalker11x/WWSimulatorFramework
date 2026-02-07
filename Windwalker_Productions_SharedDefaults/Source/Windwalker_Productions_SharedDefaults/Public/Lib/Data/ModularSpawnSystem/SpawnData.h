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

/**
 * Pool statistics for debugging and monitoring
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FPoolStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pool")
	int32 TotalSpawned = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pool")
	int32 ActiveCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pool")
	int32 PooledCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pool")
	int32 PeakActive = 0;
};

/**
 * Configuration for a spawn point actor
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FSpawnPointConfig
{
	GENERATED_BODY()

	/** Actor classes available to spawn at this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TArray<TSoftClassPtr<AActor>> SpawnClasses;

	/** Tags describing this spawn point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	FGameplayTagContainer SpawnTags;

	/** Use object pooling for spawned actors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	bool bUsePooling = true;

	/** Delay before respawning after actor is destroyed (0 = no respawn) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint", meta = (ClampMin = "0.0"))
	float RespawnDelay = 0.f;

	/** Maximum actors alive simultaneously from this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint", meta = (ClampMin = "1"))
	int32 MaxSimultaneous = 1;

	bool IsValid() const { return SpawnClasses.Num() > 0; }
};

/**
 * Configuration for a single wave in a wave spawner
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWaveConfig
{
	GENERATED_BODY()

	/** Actor classes to spawn in this wave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<TSoftClassPtr<AActor>> ActorsToSpawn;

	/** Total actors to spawn this wave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "1"))
	int32 SpawnCount = 1;

	/** Interval between individual spawns within this wave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "0.0"))
	float SpawnInterval = 0.5f;

	/** Tags for spawn categorization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	FGameplayTagContainer SpawnTags;

	bool IsValid() const { return ActorsToSpawn.Num() > 0 && SpawnCount > 0; }
};