// Copyright Windwalker Productions. All Rights Reserved.

#include "Subsystems/UniversalSpawnManager.h"
#include "Utilities/Helpers/Spawn/SpawnHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Components/DurabilityComponent.h"
#include "Interfaces/ModularInteractionSystem/InteractableInterface.h"

// ============================================================================
// SUBSYSTEM LIFECYCLE
// ============================================================================

void UUniversalSpawnManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("UniversalSpawnManager initialized"));
}

void UUniversalSpawnManager::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CleanupTimerHandle);
	}
	
	ActiveActors.Empty();
	ActorPools.Empty();
	ActorsToCleanup.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("UniversalSpawnManager deinitialized"));
	Super::Deinitialize();
}

void UUniversalSpawnManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	if (CleanupInterval > 0.f)
	{
		InWorld.GetTimerManager().SetTimer(
			CleanupTimerHandle,
			this,
			&UUniversalSpawnManager::OnCleanupTimer,
			CleanupInterval,
			true
		);
	}
}

UUniversalSpawnManager* UUniversalSpawnManager::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}
	
	return World->GetSubsystem<UUniversalSpawnManager>();
}

// ============================================================================
// ACTOR SPAWNING
// ============================================================================

AActor* UUniversalSpawnManager::SpawnActor(
	const FVector& Location,
	const FRotator& Rotation,
	TSoftClassPtr<AActor> ActorClass,
	FName ItemID,
	int32 Quantity,
	float Durability,
	float Quality,
	bool bUsePooling)

{
	// Server authority check
	if (!IsServer())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnActor called on client - ignored"));
		return nullptr;
	}

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnActor - No valid world"));
		return nullptr;
	}
	
	if (!ActorClass.IsValid() && !ActorClass.LoadSynchronous())
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnActor - Invalid ActorClass"));
		return nullptr;
	}
	
	UClass* LoadedClass = ActorClass.LoadSynchronous();
	if (!LoadedClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnActor - Failed to load class"));
		return nullptr;
	}
	
	// Get from pool or spawn new
	AActor* SpawnedActor = nullptr;
	if (bUsePooling)
	{
		SpawnedActor = GetFromPoolOrSpawn(Location, Rotation, LoadedClass);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		SpawnedActor = GetWorld()->SpawnActor<AActor>(
			LoadedClass,
			Location,
			Rotation,
			SpawnParams
		);
	}
	
	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnActor - Failed to spawn actor"));
		return nullptr;
	}
	
	// Initialize with ItemID and Quantity via interface
	if (!ItemID.IsNone())
	{
        InitializeActor(SpawnedActor, ItemID, Quantity, Durability);
	}
	
	// Track as active
	ActiveActors.Add(SpawnedActor);

	// Update pool stats
	FPoolStats& Stats = PoolStatsMap.FindOrAdd(LoadedClass);
	Stats.TotalSpawned++;
	Stats.ActiveCount++;
	if (Stats.ActiveCount > Stats.PeakActive)
	{
		Stats.PeakActive = Stats.ActiveCount;
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned actor [%s] at %s"),
		*LoadedClass->GetName(), *Location.ToString());
	
	if (SpawnedActor)
	{
		// Set replication
		SpawnedActor->SetReplicates(true);
		SpawnedActor->bAlwaysRelevant = true;

		// Broadcast delegate
		OnActorSpawned.Broadcast(SpawnedActor, NAME_None);
	}

	return SpawnedActor;
}

AActor* UUniversalSpawnManager::SpawnActorAtCameraForward(
	APawn* OwnerPawn,
	TSoftClassPtr<AActor> ActorClass,
	FName ItemID,
	int32 Quantity,
	float DropDistance)
{
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnActorAtCameraForward - OwnerPawn is null"));
		return nullptr;
	}
	
	FVector DropLocation = CalculateDropLocation(OwnerPawn, DropDistance);
	FRotator DropRotation = FRotator::ZeroRotator;
	
	return SpawnActor(DropLocation, DropRotation, ActorClass, ItemID, Quantity, true);
}

TArray<AActor*> UUniversalSpawnManager::GetActorsInRadius(
	const FVector& Location,
	float Radius,
	TSubclassOf<AActor> ActorClass) const
{
	TArray<AActor*> ActorsInRadius;
	const float RadiusSq = Radius * Radius;
	
	for (AActor* Actor : ActiveActors)
	{
		if (!Actor || !IsValid(Actor))
		{
			continue;
		}
		
		// Filter by class if specified
		if (ActorClass && !Actor->IsA(ActorClass))
		{
			continue;
		}
		
		float DistanceSq = FVector::DistSquared(Location, Actor->GetActorLocation());
		if (DistanceSq <= RadiusSq)
		{
			ActorsInRadius.Add(Actor);
		}
	}
	
	return ActorsInRadius;
}

void UUniversalSpawnManager::ReturnActorToPool(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	if (!IsServer()) return;

	
	UClass* ActorClass = Actor->GetClass();

	// Remove from active list
	ActiveActors.Remove(Actor);

	// Update pool stats
	FPoolStats& Stats = PoolStatsMap.FindOrAdd(ActorClass);
	Stats.ActiveCount = FMath::Max(0, Stats.ActiveCount - 1);
	
	// Get or create pool for this class
	TArray<FPooledActorData>& ClassPool = ActorPools.FindOrAdd(ActorClass).PooledActors;
	
	// Check if pool is full
	if (ClassPool.Num() >= MaxActorsPerClassInPool)
	{
		OnActorDespawned.Broadcast(Actor, false);
		OnPoolExhausted.Broadcast(ActorClass->GetFName());
		Actor->Destroy();
		UE_LOG(LogTemp, Log, TEXT("Pool full for %s, destroyed actor"), *ActorClass->GetName());
		return;
	}
	
	// Deactivate and add to pool
	DeactivateActor(Actor);
	
	FPooledActorData PoolData;
	PoolData.Actor = Actor;
	PoolData.ReturnedToPoolTime = GetWorld()->GetTimeSeconds();
	PoolData.bInUse = false;
	
	ClassPool.Add(PoolData);
	
	OnActorDespawned.Broadcast(Actor, true);

	UE_LOG(LogTemp, Log, TEXT("Returned %s to pool (Size: %d/%d)"),
		*ActorClass->GetName(), ClassPool.Num(), MaxActorsPerClassInPool);
}

// ============================================================================
// POOL MANAGEMENT
// ============================================================================

void UUniversalSpawnManager::PrewarmPool(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!IsServer() || !ActorClass || !GetWorld() || Count <= 0)
	{
		return;
	}

	UClass* ClassPtr = ActorClass.Get();
	TArray<FPooledActorData>& ClassPool = ActorPools.FindOrAdd(ClassPtr).PooledActors;

	const int32 SpawnCount = FMath::Min(Count, MaxActorsPerClassInPool - ClassPool.Num());

	for (int32 i = 0; i < SpawnCount; i++)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* Actor = GetWorld()->SpawnActor<AActor>(ClassPtr, FVector(0, 0, -10000.f), FRotator::ZeroRotator, SpawnParams);
		if (Actor)
		{
			DeactivateActor(Actor);

			FPooledActorData PoolData;
			PoolData.Actor = Actor;
			PoolData.ReturnedToPoolTime = GetWorld()->GetTimeSeconds();
			PoolData.bInUse = false;
			ClassPool.Add(PoolData);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Prewarmed pool for %s with %d actors (Total: %d)"),
		*ClassPtr->GetName(), SpawnCount, ClassPool.Num());
}

FPoolStats UUniversalSpawnManager::GetPoolStats(TSubclassOf<AActor> ActorClass) const
{
	if (!ActorClass)
	{
		return FPoolStats();
	}

	UClass* ClassPtr = ActorClass.Get();

	FPoolStats Stats;
	if (const FPoolStats* Tracked = PoolStatsMap.Find(ClassPtr))
	{
		Stats = *Tracked;
	}

	// Update live counts
	Stats.PooledCount = 0;
	if (const FActorPool* Pool = ActorPools.Find(ClassPtr))
	{
		Stats.PooledCount = Pool->PooledActors.Num();
	}

	Stats.ActiveCount = 0;
	for (const TObjectPtr<AActor>& Actor : ActiveActors)
	{
		if (Actor && IsValid(Actor) && Actor->IsA(ClassPtr))
		{
			Stats.ActiveCount++;
		}
	}

	return Stats;
}

void UUniversalSpawnManager::LogAllPoolStats() const
{
	UE_LOG(LogTemp, Log, TEXT("=== Pool Stats ==="));
	for (const auto& Pair : PoolStatsMap)
	{
		if (Pair.Key)
		{
			FPoolStats Stats = GetPoolStats(Pair.Key);
			UE_LOG(LogTemp, Log, TEXT("  %s: Spawned=%d Active=%d Pooled=%d Peak=%d"),
				*Pair.Key->GetName(), Stats.TotalSpawned, Stats.ActiveCount, Stats.PooledCount, Stats.PeakActive);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("=================="));
}

// ============================================================================
// SPAWN FROM REQUEST
// ============================================================================

AActor* UUniversalSpawnManager::SpawnFromRequest(const FSpawnRequest& Request, FName ItemID, int32 Quantity)
{
	FVector Location = Request.SpawnTransform.GetLocation();
	FRotator Rotation = Request.SpawnTransform.GetRotation().Rotator();

	return SpawnActor(Location, Rotation, Request.ActorClass, ItemID, Quantity, 1.0f, 1.0f, Request.bUsePooling);
}

// ============================================================================
// LIFETIME CLEANUP REGISTRATION
// ============================================================================

void UUniversalSpawnManager::RegisterForCleanup(AActor* Actor, float Lifetime, bool bReturnToPool)
{
	if (!Actor || !GetWorld())
	{
		return;
	}

	FActorCleanupData CleanupData;
	CleanupData.Actor = Actor;
	CleanupData.CleanupTime = GetWorld()->GetTimeSeconds() + Lifetime;
	CleanupData.bReturnToPool = bReturnToPool;

	ActorsToCleanup.Add(CleanupData);
}

// ============================================================================
// AI & PROP SPAWNING
// ============================================================================

AActor* UUniversalSpawnManager::SpawnAI(TSoftClassPtr<AActor> ActorClass, FVector Origin, float SearchRadius)
{
	if (!IsServer())
	{
		return nullptr;
	}

	FVector SpawnLocation;
	if (!USpawnHelpers::FindValidSpawnLocation(this, Origin, SearchRadius, SpawnLocation))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnAI - No valid navmesh location near %s"), *Origin.ToString());
		return nullptr;
	}

	return SpawnActor(SpawnLocation, FRotator::ZeroRotator, ActorClass, NAME_None, 1, 1.0f, 1.0f, false);
}

AActor* UUniversalSpawnManager::SpawnProp(TSoftClassPtr<AActor> ActorClass, FVector Location, FRotator Rotation)
{
	if (!IsServer())
	{
		return nullptr;
	}

	FTransform PropTransform(Rotation, Location, FVector::OneVector);
	PropTransform = USpawnHelpers::SnapTransformToGround(this, PropTransform);

	return SpawnActor(
		PropTransform.GetLocation(),
		PropTransform.GetRotation().Rotator(),
		ActorClass, NAME_None, 1, 1.0f, 1.0f, false
	);
}

// ============================================================================
// DROP TABLE & SCATTER SPAWNING
// ============================================================================

TArray<AActor*> UUniversalSpawnManager::SpawnFromDropTable(
	const TArray<FDropTableEntry>& DropTable,
	FVector Origin,
	float ScatterRadius,
	TSoftClassPtr<AActor> ItemActorClass,
	AActor* Looter)
{
	TArray<AActor*> SpawnedActors;

	if (!IsServer())
	{
		return SpawnedActors;
	}

	// Process drop table via helpers
	TArray<FDropResult> Drops = USpawnHelpers::ProcessDropTable(DropTable, Looter);
	if (Drops.Num() == 0)
	{
		return SpawnedActors;
	}

	// Build spawn requests with scatter locations
	TArray<FSpawnRequest> Requests = USpawnHelpers::BuildSpawnRequestsFromDrops(Drops, Origin, ScatterRadius, ItemActorClass);

	// Snap each to ground and spawn
	for (const FSpawnRequest& Request : Requests)
	{
		FSpawnRequest SnappedRequest = Request;
		SnappedRequest.SpawnTransform = USpawnHelpers::SnapTransformToGround(this, Request.SpawnTransform);

		AActor* Actor = SpawnFromRequest(SnappedRequest, Request.PoolID, 1);
		if (Actor)
		{
			SpawnedActors.Add(Actor);
		}
	}

	return SpawnedActors;
}

TArray<AActor*> UUniversalSpawnManager::SpawnScattered(
	TSoftClassPtr<AActor> ActorClass,
	FVector Origin,
	int32 Count,
	float MinRadius,
	float MaxRadius)
{
	TArray<AActor*> SpawnedActors;

	if (!IsServer() || Count <= 0)
	{
		return SpawnedActors;
	}

	TArray<FVector> Locations = USpawnHelpers::CalculateScatterLocations(Origin, Count, MinRadius, MaxRadius);

	for (const FVector& Location : Locations)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, Location, FVector::OneVector);
		SpawnTransform = USpawnHelpers::SnapTransformToGround(this, SpawnTransform);

		AActor* Actor = SpawnActor(
			SpawnTransform.GetLocation(),
			SpawnTransform.GetRotation().Rotator(),
			ActorClass,
			NAME_None, 1, 1.0f, 1.0f, true
		);

		if (Actor)
		{
			SpawnedActors.Add(Actor);
		}
	}

	return SpawnedActors;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

AActor* UUniversalSpawnManager::GetFromPoolOrSpawn(
	const FVector& Location,
	const FRotator& Rotation,
	UClass* ActorClass)
{
	// Try to get from pool first
	FActorPool* FoundPool = ActorPools.Find(ActorClass);
    
	if (FoundPool && FoundPool->PooledActors.Num() > 0)
	{
		// Get the last element (O(1) operation)
		FPooledActorData PoolData = FoundPool->PooledActors.Pop();
        
		if (IsValid(PoolData.Actor))
		{
			PoolData.Actor->SetActorLocationAndRotation(Location, Rotation);
			PoolData.Actor->SetActorHiddenInGame(false);
			PoolData.Actor->SetActorEnableCollision(true);
            
			UE_LOG(LogTemp, Log, TEXT("Reused %s from pool"), *ActorClass->GetName());
			return PoolData.Actor;
		}
	}
	
	// Pool empty - spawn new
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	AActor* NewActor = GetWorld()->SpawnActor<AActor>(
		ActorClass,
		Location,
		Rotation,
		SpawnParams
	);
	
	if (NewActor)
	{
		UE_LOG(LogTemp, Log, TEXT("Created new %s (pool was empty)"), *ActorClass->GetName());
	}
	
	return NewActor;
}

void UUniversalSpawnManager::InitializeActor(AActor* Actor, FName ItemID, int32 Quantity, float Durability)
{
	if (!Actor)
	{
		return;
	}
	
	// Try to set ItemID via interface
	if (Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		IInteractableInterface::Execute_SetItemID(Actor, ItemID);
		// Try to call LoadAssets via interface
		IInteractableInterface::Execute_LoadAssets(Actor);

		// Try to get inventory component and add item
		UActorComponent* InventoryComp = IInteractableInterface::Execute_GetInventoryComponentAsActorComponent(Actor);
		if (InventoryComp)
		{
			// TODO: Add item to inventory component
			// You'll need to cast to UInventoryComponent and call AddItem()
			UE_LOG(LogTemp, Log, TEXT("Actor has inventory - TODO: Add ItemID %s x%d"), 
				*ItemID.ToString(), Quantity);
		}
		//Try getting Durability Component, try updating durability if valid.
		if (UActorComponent* DurabilityComponent = IInteractableInterface::Execute_GetDurabilityComponentAsActorComponent(Actor))
		{
			if (DurabilityComponent->GetClass()->ImplementsInterface(UDurabilityInterface::StaticClass()))
			{
				IDurabilityInterface::Execute_SetDurability(DurabilityComponent, Durability);
			}
		}
	}
	
	if (Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
	}
}

FVector UUniversalSpawnManager::CalculateDropLocation(APawn* OwnerPawn, float DropDistance) const
{
	if (!OwnerPawn)
	{
		return FVector::ZeroVector;
	}
	
	FVector ViewLocation;
	FRotator ViewRotation;
	OwnerPawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	
	FVector ForwardVector = ViewRotation.Vector();
	FVector DropLocation = ViewLocation + (ForwardVector * DropDistance);
	
	// Line trace to find ground
	FHitResult HitResult;
	FVector TraceStart = DropLocation;
	FVector TraceEnd = DropLocation - FVector(0, 0, 500.f);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		DropLocation = HitResult.Location + FVector(0, 0, 10.f);
	}
	
	return DropLocation;
}

void UUniversalSpawnManager::DeactivateActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}
	
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorLocation(FVector(0, 0, -10000.f));
}

void UUniversalSpawnManager::OnCleanupTimer()
{
	int32 RemovedCount = 0;

	// Process lifetime-based cleanup
	if (GetWorld())
	{
		const float CurrentTime = GetWorld()->GetTimeSeconds();

		for (int32 i = ActorsToCleanup.Num() - 1; i >= 0; --i)
		{
			FActorCleanupData& CleanupData = ActorsToCleanup[i];

			// Remove invalid entries
			if (!CleanupData.Actor || !IsValid(CleanupData.Actor))
			{
				ActorsToCleanup.RemoveAt(i);
				RemovedCount++;
				continue;
			}

			// Check if lifetime expired
			if (CurrentTime >= CleanupData.CleanupTime)
			{
				if (CleanupData.bReturnToPool)
				{
					ReturnActorToPool(CleanupData.Actor);
				}
				else
				{
					ActiveActors.Remove(CleanupData.Actor);
					OnActorDespawned.Broadcast(CleanupData.Actor, false);
					CleanupData.Actor->Destroy();
				}

				ActorsToCleanup.RemoveAt(i);
				RemovedCount++;
			}
		}
	}

	// Clean up invalid active actors
	for (int32 i = ActiveActors.Num() - 1; i >= 0; --i)
	{
		if (!ActiveActors[i] || !IsValid(ActiveActors[i]))
		{
			ActiveActors.RemoveAt(i);
			RemovedCount++;
		}
	}

	// Clean up invalid pooled actors
	for (auto& PoolPair : ActorPools)
	{
		TArray<FPooledActorData>& ClassPool = PoolPair.Value.PooledActors;

		for (int32 i = ClassPool.Num() - 1; i >= 0; --i)
		{
			if (!ClassPool[i].Actor || !IsValid(ClassPool[i].Actor))
			{
				ClassPool.RemoveAt(i);
				RemovedCount++;
			}
		}
	}

	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Cleanup removed %d invalid actors"), RemovedCount);
	}
}

bool UUniversalSpawnManager::IsServer() const
{
	UWorld* World = GetWorld();
	return World && (World->GetNetMode() == NM_DedicatedServer || World->GetNetMode() == NM_ListenServer);
}
