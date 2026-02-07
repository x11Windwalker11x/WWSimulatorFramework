// Copyright Windwalker Productions. All Rights Reserved.

#include "Components/WaveSpawnerComponent.h"
#include "Subsystems/UniversalSpawnManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

UWaveSpawnerComponent::UWaveSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWaveSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart && GetOwner() && GetOwner()->HasAuthority())
	{
		StartWaves();
	}
}

void UWaveSpawnerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWaveSpawnerComponent, CurrentWaveIndex);
	DOREPLIFETIME(UWaveSpawnerComponent, bIsSpawning);
}

void UWaveSpawnerComponent::StartWaves()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (Waves.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveSpawner: No waves configured"));
		return;
	}

	bIsSpawning = true;
	CurrentWaveSpawnedCount = 0;

	// Start spawning first actor in current wave
	SpawnNextInWave();
}

void UWaveSpawnerComponent::StopWaves()
{
	bIsSpawning = false;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTickHandle);
		GetWorld()->GetTimerManager().ClearTimer(WaveDelayHandle);
	}
}

void UWaveSpawnerComponent::SpawnNextInWave()
{
	if (!bIsSpawning || !GetOwner() || !GetWorld())
	{
		return;
	}

	if (!Waves.IsValidIndex(CurrentWaveIndex))
	{
		StopWaves();
		return;
	}

	const FWaveConfig& Wave = Waves[CurrentWaveIndex];

	UUniversalSpawnManager* SpawnManager = UUniversalSpawnManager::Get(this);
	if (!SpawnManager)
	{
		return;
	}

	// Pick class (cycle through available classes)
	const int32 ClassIndex = CurrentWaveSpawnedCount % Wave.ActorsToSpawn.Num();
	const FVector SpawnLocation = GetOwner()->GetActorLocation();

	SpawnManager->SpawnActor(
		SpawnLocation, GetOwner()->GetActorRotation(),
		Wave.ActorsToSpawn[ClassIndex],
		NAME_None, 1, 1.0f, 1.0f, true
	);

	CurrentWaveSpawnedCount++;

	// Check if wave is complete
	if (CurrentWaveSpawnedCount >= Wave.SpawnCount)
	{
		OnWaveComplete.Broadcast(CurrentWaveIndex, Waves.Num());

		UE_LOG(LogTemp, Log, TEXT("WaveSpawner: Wave %d/%d complete"), CurrentWaveIndex + 1, Waves.Num());

		// Advance to next wave
		AdvanceWave();
	}
	else
	{
		// Schedule next spawn in this wave
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTickHandle, this,
			&UWaveSpawnerComponent::SpawnNextInWave,
			Wave.SpawnInterval, false
		);
	}
}

void UWaveSpawnerComponent::AdvanceWave()
{
	CurrentWaveIndex++;

	if (CurrentWaveIndex >= Waves.Num())
	{
		if (bLoop)
		{
			CurrentWaveIndex = 0;
		}
		else
		{
			bIsSpawning = false;
			UE_LOG(LogTemp, Log, TEXT("WaveSpawner: All waves complete"));
			return;
		}
	}

	CurrentWaveSpawnedCount = 0;

	// Start next wave after delay
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			WaveDelayHandle, this,
			&UWaveSpawnerComponent::SpawnNextInWave,
			WaveDelay, false
		);
	}
}
