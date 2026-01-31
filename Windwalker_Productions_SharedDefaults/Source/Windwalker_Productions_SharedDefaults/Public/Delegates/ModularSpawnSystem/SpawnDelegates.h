#pragma once

#include "CoreMinimal.h"
#include "SpawnDelegates.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FSpawnDelegateOnActorSpawned,
	AActor*, SpawnedActor,
	FName, PoolID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FSpawnDelegateOnActorDespawned,
	AActor*, SpawnedActor,
	bool, bReturnedToPool);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FSpawnDelegateOnPoolExhausted,
	FName, PoolID);

