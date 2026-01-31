// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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