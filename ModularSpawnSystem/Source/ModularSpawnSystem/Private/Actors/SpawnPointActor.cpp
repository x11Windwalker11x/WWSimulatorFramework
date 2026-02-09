// Copyright Windwalker Productions. All Rights Reserved.

#include "Actors/SpawnPointActor.h"
#include "Subsystems/UniversalSpawnManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpawnPoint, Log, All);

DECLARE_STATS_GROUP(TEXT("SpawnManager"), STATGROUP_SpawnManagerPoint, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("TryRespawn"), STAT_SpawnPoint_TryRespawn, STATGROUP_SpawnManagerPoint);

ASpawnPointActor::ASpawnPointActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

#if WITH_EDITORONLY_DATA
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
#endif
}

void ASpawnPointActor::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay && HasAuthority())
	{
		SpawnActors();
	}
}

void ASpawnPointActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ASpawnPointActor::SpawnActors()
{
	if (!HasAuthority() || !Config.IsValid())
	{
		return;
	}

	UUniversalSpawnManager* SpawnManager = UUniversalSpawnManager::Get(this);
	if (!SpawnManager)
	{
		return;
	}

	// Clean stale refs
	SpawnedActors.RemoveAll([](const TWeakObjectPtr<AActor>& Ref) { return !Ref.IsValid(); });

	const int32 CanSpawn = Config.MaxSimultaneous - SpawnedActors.Num();
	if (CanSpawn <= 0)
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation();
	const FRotator SpawnRotation = GetActorRotation();

	for (int32 i = 0; i < CanSpawn && i < Config.SpawnClasses.Num(); i++)
	{
		const int32 ClassIndex = i % Config.SpawnClasses.Num();

		AActor* SpawnedActor = SpawnManager->SpawnActor(
			SpawnLocation, SpawnRotation,
			Config.SpawnClasses[ClassIndex],
			ItemID, 1, 1.0f, 1.0f, Config.bUsePooling
		);

		if (SpawnedActor)
		{
			SpawnedActors.Add(SpawnedActor);
			SpawnedActor->OnDestroyed.AddDynamic(this, &ASpawnPointActor::OnSpawnedActorDestroyed);
		}
	}
}

TArray<AActor*> ASpawnPointActor::GetAliveActors() const
{
	TArray<AActor*> Alive;
	for (const TWeakObjectPtr<AActor>& Ref : SpawnedActors)
	{
		if (Ref.IsValid())
		{
			Alive.Add(Ref.Get());
		}
	}
	return Alive;
}

int32 ASpawnPointActor::GetAliveCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AActor>& Ref : SpawnedActors)
	{
		if (Ref.IsValid())
		{
			Count++;
		}
	}
	return Count;
}

void ASpawnPointActor::OnSpawnedActorDestroyed(AActor* DestroyedActor)
{
	SpawnedActors.RemoveAll([DestroyedActor](const TWeakObjectPtr<AActor>& Ref)
	{
		return !Ref.IsValid() || Ref.Get() == DestroyedActor;
	});

	// Schedule respawn if configured - guard against overwriting pending timer
	if (Config.RespawnDelay > 0.f && HasAuthority() && GetWorld())
	{
		PendingRespawnCount++;

		if (!GetWorld()->GetTimerManager().IsTimerActive(RespawnTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(
				RespawnTimerHandle, this,
				&ASpawnPointActor::TryRespawn,
				Config.RespawnDelay, false
			);
		}
	}
}

void ASpawnPointActor::TryRespawn()
{
	SCOPE_CYCLE_COUNTER(STAT_SpawnPoint_TryRespawn);
	const int32 Pending = PendingRespawnCount;
	PendingRespawnCount = 0;

	if (GetAliveCount() < Config.MaxSimultaneous)
	{
		UE_LOG(LogSpawnPoint, Log, TEXT("SpawnPoint [%s]: TryRespawn (Pending=%d, Alive=%d/%d)"),
			*GetName(), Pending, GetAliveCount(), Config.MaxSimultaneous);
		SpawnActors();
	}
}
