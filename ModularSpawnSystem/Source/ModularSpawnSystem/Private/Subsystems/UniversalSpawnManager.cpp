// Copyright Windwalker Productions. All Rights Reserved.

#include "Subsystems/UniversalSpawnManager.h"
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
	
	UE_LOG(LogTemp, Log, TEXT("✅ Spawned actor [%s] at %s"), 
		*LoadedClass->GetName(), *Location.ToString());
	
	if (SpawnedActor)
	{
		// Set replication
		SpawnedActor->SetReplicates(true);
		SpawnedActor->bAlwaysRelevant = true;
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
	
	// Get or create pool for this class
	TArray<FPooledActorData>& ClassPool = ActorPools.FindOrAdd(ActorClass).PooledActors;
	
	// Check if pool is full
	if (ClassPool.Num() >= MaxActorsPerClassInPool)
	{
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
	
	UE_LOG(LogTemp, Log, TEXT("Returned %s to pool (Size: %d/%d)"), 
		*ActorClass->GetName(), ClassPool.Num(), MaxActorsPerClassInPool);
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
