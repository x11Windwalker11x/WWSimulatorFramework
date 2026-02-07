// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/ModularSpawnSystem/SpawnDelegates.h"
#include "Lib/Data/ModularSpawnSystem/SpawnData.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "UniversalSpawnManager.generated.h"

/**
 * Pooled Actor Data
 * Stores information about pooled actors for reuse
 */
USTRUCT()
struct FPooledActorData
{
	GENERATED_BODY()

	/** The pooled actor */
	UPROPERTY()
	TObjectPtr<AActor> Actor;

	/** Time when actor was returned to pool */
	float ReturnedToPoolTime = 0.f;

	/** Is this actor currently in use? */
	bool bInUse = false;
};

// To be able to wrap FPooledActorData in a TMap
USTRUCT()
struct FActorPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPooledActorData> PooledActors;
};
/**
 * Actor Cleanup Data
 * Stores information about actors registered for automatic cleanup
 */
USTRUCT()
struct FActorCleanupData
{
	GENERATED_BODY()

	/** The actor to clean up */
	UPROPERTY()
	TObjectPtr<AActor> Actor;

	/** Time when actor should be cleaned up */
	float CleanupTime = 0.f;

	/** Should actor be returned to pool instead of destroyed? */
	bool bReturnToPool = false;
};

/**
 * Universal Spawn Manager
 * 
 * Centralized system for spawning and managing all world actors.
 * Provides object pooling, automatic cleanup, and spatial queries.
 * Component and actor agnostic - uses soft class references.
 */
UCLASS()
class MODULARSPAWNSYSTEM_API UUniversalSpawnManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	bool IsServer() const;

	// ============================================================================
	// SUBSYSTEM INTERFACE
	// ============================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/**
	 * Get the UniversalSpawnManager for a given world
	 */
	UFUNCTION(BlueprintPure, Category = "Spawn Manager", meta = (WorldContext = "WorldContextObject"))
	static UUniversalSpawnManager* Get(const UObject* WorldContextObject);

	// ============================================================================
	// DELEGATES
	// ============================================================================

	/** Broadcast when an actor is spawned */
	UPROPERTY(BlueprintAssignable, Category = "Spawn Manager|Delegates")
	FSpawnDelegateOnActorSpawned OnActorSpawned;

	/** Broadcast when an actor is despawned or returned to pool */
	UPROPERTY(BlueprintAssignable, Category = "Spawn Manager|Delegates")
	FSpawnDelegateOnActorDespawned OnActorDespawned;

	/** Broadcast when a pool is exhausted (full, actor destroyed instead) */
	UPROPERTY(BlueprintAssignable, Category = "Spawn Manager|Delegates")
	FSpawnDelegateOnPoolExhausted OnPoolExhausted;

	// ============================================================================
	// GENERIC ACTOR SPAWNING - COMPONENT AGNOSTIC
	// ============================================================================

	/**
	 * Spawn any actor in the world (generic, component-agnostic)
	 * @param Location - World location to spawn at
	 * @param Rotation - World rotation to spawn with
	 * @param ActorClass - Soft class reference to actor to spawn
	 * @param ItemID - Optional ItemID to set via interface
	 * @param Quantity - Optional quantity to set via interface
	 * @param bUsePooling - Should we try to get from pool?
	 * @return The spawned actor, or nullptr if spawn failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	AActor* SpawnActor(
		const FVector& Location,
		const FRotator& Rotation,
		TSoftClassPtr<AActor> ActorClass,
		FName ItemID,
		int32 Quantity,
		float Durability = 1.0f,
		float Quality = 1.0f,
		bool bUsePooling = true
	);


	/**
	 * Spawn actor at camera forward direction
	 * @param OwnerPawn - The pawn for camera direction
	 * @param ActorClass - Soft class reference to spawn
	 * @param ItemID - Optional ItemID
	 * @param Quantity - Optional quantity
	 * @param DropDistance - Distance in front of camera
	 * @return The spawned actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	AActor* SpawnActorAtCameraForward(
		APawn* OwnerPawn,
		TSoftClassPtr<AActor> ActorClass,
		FName ItemID = NAME_None,
		int32 Quantity = 1,
		float DropDistance = 200.f
	);

	/**
	 * Get all actors of a specific class within radius
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	TArray<AActor*> GetActorsInRadius(
		const FVector& Location,
		float Radius,
		TSubclassOf<AActor> ActorClass = nullptr
	) const;

	/**
	 * Return actor to pool for reuse
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	void ReturnActorToPool(AActor* Actor);

	/**
	 * Spawn actor from a FSpawnRequest struct
	 * @param Request - Spawn configuration
	 * @param ItemID - Optional ItemID for interface initialization
	 * @param Quantity - Optional quantity for interface initialization
	 * @return The spawned actor, or nullptr if spawn failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	AActor* SpawnFromRequest(const FSpawnRequest& Request, FName ItemID = NAME_None, int32 Quantity = 1);

	/**
	 * Register an actor for automatic cleanup after a lifetime duration
	 * @param Actor - The actor to register
	 * @param Lifetime - Seconds until cleanup
	 * @param bReturnToPool - Return to pool instead of destroying
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	void RegisterForCleanup(AActor* Actor, float Lifetime, bool bReturnToPool = true);

	// ============================================================================
	// POOL MANAGEMENT
	// ============================================================================

	/**
	 * Pre-spawn actors into the pool for a class to avoid spawn hitches
	 * @param ActorClass - Class to prewarm
	 * @param Count - Number of actors to pre-spawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager|Pool")
	void PrewarmPool(TSubclassOf<AActor> ActorClass, int32 Count);

	/**
	 * Get pool statistics for a specific actor class
	 */
	UFUNCTION(BlueprintPure, Category = "Spawn Manager|Pool")
	FPoolStats GetPoolStats(TSubclassOf<AActor> ActorClass) const;

	/** Log all pool stats to output log */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager|Pool")
	void LogAllPoolStats() const;

	// ============================================================================
	// AI & PROP SPAWNING
	// ============================================================================

	/**
	 * Spawn an AI actor at a navmesh-valid location near Origin
	 * @param ActorClass - AI actor class to spawn
	 * @param Origin - Desired spawn location
	 * @param SearchRadius - Navmesh search radius for valid location
	 * @return The spawned actor, or nullptr if no valid location
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	AActor* SpawnAI(TSoftClassPtr<AActor> ActorClass, FVector Origin, float SearchRadius = 500.f);

	/**
	 * Spawn a static prop/interactable, snapped to ground
	 * @param ActorClass - Prop actor class to spawn
	 * @param Location - Desired spawn location
	 * @param Rotation - Spawn rotation
	 * @return The spawned actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	AActor* SpawnProp(TSoftClassPtr<AActor> ActorClass, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

	// ============================================================================
	// DROP TABLE & SCATTER SPAWNING
	// ============================================================================

	/**
	 * Process a drop table and spawn resulting items scattered around an origin
	 * @param DropTable - Array of drop table entries to process
	 * @param Origin - Center point for scattered spawning
	 * @param ScatterRadius - Max radius for scatter positioning
	 * @param ItemActorClass - Actor class for spawned items
	 * @param Looter - Optional actor to check drop requirements against
	 * @return Array of spawned actors
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	TArray<AActor*> SpawnFromDropTable(
		const TArray<FDropTableEntry>& DropTable,
		FVector Origin,
		float ScatterRadius,
		TSoftClassPtr<AActor> ItemActorClass,
		AActor* Looter = nullptr
	);

	/**
	 * Spawn multiple actors of the same class scattered around an origin
	 * @param ActorClass - Class to spawn
	 * @param Origin - Center point
	 * @param Count - Number to spawn
	 * @param MinRadius - Minimum scatter distance
	 * @param MaxRadius - Maximum scatter distance
	 * @return Array of spawned actors
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawn Manager")
	TArray<AActor*> SpawnScattered(
		TSoftClassPtr<AActor> ActorClass,
		FVector Origin,
		int32 Count,
		float MinRadius,
		float MaxRadius
	);

protected:
	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Maximum actors per class to keep in pool */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Manager|Config")
	int32 MaxActorsPerClassInPool = 50;

	/** Auto-cleanup interval (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Manager|Config")
	float CleanupInterval = 5.f;

	// ============================================================================
	// POOLING SYSTEM
	// ============================================================================

	/** Actor pools organized by class */
	UPROPERTY()
	TMap<UClass*, FActorPool> ActorPools;

	/** All active spawned actors */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveActors;

	/** Per-class pool statistics */
	TMap<UClass*, FPoolStats> PoolStatsMap;

	// ============================================================================
	// CLEANUP SYSTEM
	// ============================================================================

	/** Actors registered for automatic cleanup */
	UPROPERTY()
	TArray<FActorCleanupData> ActorsToCleanup;

	/** Cleanup timer handle */
	FTimerHandle CleanupTimerHandle;

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	/** Get actor from pool or spawn new */
	AActor* GetFromPoolOrSpawn(
		const FVector& Location,
		const FRotator& Rotation,
		UClass* ActorClass
	);

	/** Initialize actor with ItemID and Quantity via interface */
	void InitializeActor(AActor* Actor, FName ItemID, int32 Quantity, float Durability = 1.0f);

	/** Calculate drop location from pawn's camera */
	FVector CalculateDropLocation(APawn* OwnerPawn, float DropDistance) const;

	/** Deactivate actor and return to pool */
	void DeactivateActor(AActor* Actor);

	/** Cleanup timer callback */
	void OnCleanupTimer();
};