// Fill out your copyright notice in the Description page of Project Settings.

#include "PickUpActor.h"
#include "../../../../../../../Program Files/Epic Games/UE_5.6/Engine/Source/Runtime/Engine/Classes/GameFramework/PlayerController.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

APickUpActor::APickUpActor()
{
	// Parent constructor (APickupActor_Master -> AInteractableActor_Master) handles:
	// - Creating all components (Collision, StaticMesh, SkeletalMesh, etc.)
	// - Setting up collision responses
	// - Configuring component attachments
	// - Initializing asset loading system
	
	// Add any PickUpActor-specific initialization here if needed
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void APickUpActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Parent's BeginPlay() already handles:
	// - Hiding preview meshes
	// - Registering with InteractionSubsystem
	// - Caching basic item info from DataTable
	// - Checking for existing player overlaps
	// - Setting up debug system
	
	// Additional replication settings for this specific actor
	SetReplicates(true);
	SetReplicateMovement(true);
}

void APickUpActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Parent's Tick() is available if needed
	// Most functionality is event-driven (proximity, interaction) rather than tick-based
}

// ============================================================================
// INTERACTION IMPLEMENTATION
// ============================================================================

void APickUpActor::OnInteract_Implementation(AController* InstigatorController) 
{
	if (!InstigatorController) return;
	
	// Get player name for logging (networked)
	FString PlayerName;
	if (APlayerState* PlayerState = InstigatorController->GetPlayerState<APlayerState>())
	{
		PlayerName = PlayerState->GetPlayerName();
	}
	else
	{
		PlayerName = InstigatorController->GetName();
	}
	
	UE_LOG(LogInteractableSubsystem, Warning, TEXT("Player %s interacted with: %s"), *PlayerName, *this->GetName());
	
	// Add your pickup logic here:
	// - Add item to inventory (use InteractableComponent or InventoryComponent)
	// - Play pickup sound (already loaded in AssetPackage.PickupSound if defined)
	// - Play pickup animation (already loaded in AssetPackage.PickupMontage if defined)
	// - Notify UI
	// - Destroy or hide actor
	// - Update quest/achievement systems
	
	// Example: Access the loaded asset package
	// const FItemData_Dynamic& LoadedAssets = GetAssetPackage();
	// if (LoadedAssets.bIsLoaded && LoadedAssets.PickupSound.IsValid())
	// {
	//     UGameplayStatics::PlaySoundAtLocation(this, LoadedAssets.PickupSound.Get(), GetActorLocation());
	// }
	
	// Call parent's interaction logic if needed
	// Super::OnInteract_Implementation(InstigatorController);
	
	// Example: Destroy actor after pickup
	// Destroy();
}

// ============================================================================
// REMOVED: SetupMeshAndCollision, SetupStaticMesh, SetupSkeletalMesh
// ============================================================================

/*
 * These functions have been REMOVED because the parent class (InteractableActor_Master)
 * already handles all mesh and collision setup automatically:
 * 
 * PARENT HANDLES:
 * - Reading mesh references from DataTable via ItemID
 * - Async loading meshes when player approaches (StartProximityAssetLoading)
 * - Setting meshes to components (ApplyLoadedMeshes)
 * - Showing/hiding appropriate mesh component based on asset type
 * - Configuring collision on CollisionComponent (SphereComponent)
 * - Unloading meshes when player leaves
 * 
 * WORKFLOW:
 * 1. Set ItemDataTable and ItemID in editor
 * 2. Parent's BeginPlay() calls CacheBasicItemInfo() - reads DataTable
 * 3. Player approaches - OnPlayerNearbyInit_Implementation() called
 * 4. Parent calls StartProximityAssetLoading() - async loads assets
 * 5. When loaded - OnAssetLoadingComplete() calls ApplyLoadedMeshes()
 * 6. Meshes are automatically applied to StaticMeshComponent or SkeletalMeshComponent
 * 7. Player leaves - UnloadAssetsAndMeshes() cleans up
 * 
 * You don't need to manually load or set meshes - it's all automatic!
 * Just configure the DataTable row with your mesh references.
 */