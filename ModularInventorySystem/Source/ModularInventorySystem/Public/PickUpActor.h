// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactables/Item/Pickup/PickupActor_Master.h"
#include "Interface/InteractionSystem/InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h" // used in interface function implementations
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Logging/InteractableInventoryLogging.h" // logging in cpp

#include "PickUpActor.generated.h"

/**
 * Concrete pickup actor implementation.
 * Inherits all functionality from PickupActor_Master (which inherits from InteractableActor_Master).
 * 
 * INHERITED COMPONENTS (do not redeclare):
 * - CollisionComponent (USphereComponent*) - Already set up for overlaps
 * - StaticMeshComponent (UStaticMeshComponent*) - For static mesh items
 * - SkeletalMeshComponent (USkeletalMeshComponent*) - For skeletal mesh items
 * - InteractableComponent (UInteractableComponent*)
 * - InventoryComponent (UInventoryComponent*) - if bHasInventory is true
 * 
 * INHERITED ASSET LOADING:
 * - Parent handles all asset loading from DataTable via ItemID
 * - Parent handles proximity-based loading/unloading
 * - Assets are loaded automatically when player approaches
 * - Assets are unloaded when player leaves
 * 
 * INHERITED INTERACTION:
 * - OnInteract_Implementation() - Override to add pickup-specific behavior
 * - All other interface methods inherited and working
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API APickUpActor : public APickupActor_Master
{
	GENERATED_BODY()

public:
	// ============================================================================
	// CONSTRUCTOR & LIFECYCLE
	// ============================================================================
	
	APickUpActor();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// INTERACTION OVERRIDE
	// ============================================================================
	
	/**
	 * Override of interaction behavior - adds pickup-specific logic.
	 * Parent class handles all the asset loading and UI updates.
	 * This just handles the actual pickup action.
	 */
	virtual void OnInteract_Implementation(AController* InstigatorController) override;

	// ============================================================================
	// NOTE: All components and asset loading inherited from parent chain
	// ============================================================================
	
	/*
	 * Components are inherited and already created:
	 * - CollisionComponent: Already configured for Pawn overlaps
	 * - StaticMeshComponent: Already attached to root, visibility handled by parent
	 * - SkeletalMeshComponent: Already attached to root, visibility handled by parent
	 * - InteractableComponent: Already created
	 * - InventoryComponent: Created if bHasInventory is true
	 * 
	 * Asset loading is inherited:
	 * - Parent's CacheBasicItemInfo() loads data from DataTable
	 * - Parent's StartProximityAssetLoading() handles async loading
	 * - Parent's ApplyLoadedMeshes() sets meshes to components
	 * - Parent's UnloadAssetsAndMeshes() cleans up when player leaves
	 * 
	 * No need to manually call SetupMeshAndCollision() - parent handles it automatically!
	 */
};