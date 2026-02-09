// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/ModularSpawnSystem/SpawnDelegates.h"
#include "Lib/Data/ModularSpawnSystem/SpawnData.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "WaveSpawnerComponent.generated.h"

class UUniversalSpawnManager;

/**
 * Wave Spawner Component
 * Attach to any actor to enable timed wave spawning.
 * Uses UUniversalSpawnManager for actual spawning.
 */
UCLASS(ClassGroup = (Spawn), meta = (BlueprintSpawnableComponent))
class MODULARSPAWNSYSTEM_API UWaveSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWaveSpawnerComponent();

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Wave configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Spawner")
	TArray<FWaveConfig> Waves;

	/** Delay between waves (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Spawner", meta = (ClampMin = "0.0"))
	float WaveDelay = 5.f;

	/** Start spawning automatically on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Spawner")
	bool bAutoStart = false;

	/** Loop back to wave 0 after all waves complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Spawner")
	bool bLoop = false;

	// ============================================================================
	// DELEGATES
	// ============================================================================

	/** Broadcast when a wave completes */
	UPROPERTY(BlueprintAssignable, Category = "Wave Spawner|Delegates")
	FSpawnDelegateOnWaveComplete OnWaveComplete;

	// ============================================================================
	// API
	// ============================================================================

	/** Start wave spawning from current wave */
	UFUNCTION(BlueprintCallable, Category = "Wave Spawner")
	void StartWaves();

	/** Stop wave spawning */
	UFUNCTION(BlueprintCallable, Category = "Wave Spawner")
	void StopWaves();

	/** Get current wave index */
	UFUNCTION(BlueprintPure, Category = "Wave Spawner")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	/** Check if currently spawning */
	UFUNCTION(BlueprintPure, Category = "Wave Spawner")
	bool IsSpawning() const { return bIsSpawning; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Current wave index */
	UPROPERTY(Replicated)
	int32 CurrentWaveIndex = 0;

	/** Actors spawned in current wave so far */
	int32 CurrentWaveSpawnedCount = 0;

	/** Is currently spawning */
	UPROPERTY(Replicated)
	bool bIsSpawning = false;

	/** Cached spawn manager reference (Rule #41) */
	TWeakObjectPtr<UUniversalSpawnManager> CachedSpawnManager;

	/** Timer for individual spawn ticks within a wave */
	FTimerHandle SpawnTickHandle;

	/** Timer for delay between waves */
	FTimerHandle WaveDelayHandle;

	/** Spawn one actor in current wave */
	void SpawnNextInWave();

	/** Advance to next wave after delay */
	void AdvanceWave();
};
