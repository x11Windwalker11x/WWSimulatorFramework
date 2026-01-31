// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactables/InteractableActor_Master.h"
#include "PickupActor_Master.generated.h"

/**
 * Pickup actor class - inherits all functionality from InteractableActor_Master.
 * Specialized for item pickups in the world.
 * This is abstract - extend in Blueprints or C++ for concrete pickup types.
 * 
 * NOTE: This class inherits all functionality from InteractableActor_Master.
 * Override specific methods only when pickup-specific behavior is needed.
 */
UCLASS(Abstract, Blueprintable)
class MODULARINVENTORYSYSTEM_API APickupActor_Master : public AInteractableActor_Master
{
	GENERATED_BODY()

public:
	// ============================================================================
	// CONSTRUCTOR
	// ============================================================================
	
	/**
	 * Constructor - delegates to parent class
	 * Override if pickup-specific initialization is needed
	 */
	APickupActor_Master();

	// ============================================================================
	// NOTE: All other functionality is inherited from AInteractableActor_Master
	// ============================================================================
	
	/*
	 * Inherited Public Interface:
	 * - LoadItemAssets()
	 * - AreAssetsReady()
	 * - GetAssetPackage()
	 * - GetPreInteractionDistance()
	 * - GetFullInteractionDistance()
	 * - GetUIUpdateRate()
	 * - GetGlobalLoadDistance()
	 * - OnInteractionStateChanged() [BlueprintImplementableEvent]
	 * 
	 * Inherited IInteractableInterface:
	 * - OnInteract_Implementation()
	 * - IsCurrentlyInteractable_Implementation()
	 * - OnPlayerNearbyInit_Implementation()
	 * - SetInteractionEnabled_Implementation()
	 * - GetInteractionEnabled_Implementation()
	 * - SetInteractionNotifications_Implementation()
	 * - GetbInteractionNotificationsAllowManualHandling_Implementation()
	 * - OnPreInteractionEntered_Implementation()
	 * - OnFullInteractionEntered_Implementation()
	 * - OnPreInteractionExited_Implementation()
	 * - OnFullInteractionExited_Implementation()
	 * - LoadAssets_Implementation()
	 * - UnloadAssets_Implementation()
	 * 
	 * Inherited Delegates:
	 * - FOnInteractionStateChanged InteractionStateChanged
	 * 
	 * Inherited Components:
	 * - CollisionComponent
	 * - InteractableComponent
	 * - InventoryComponent
	 * - StaticMeshComponent
	 * - SkeletalMeshComponent
	 * - PreviewMesh
	 * - PreviewSkeletalMesh
	 * 
	 * Inherited Configuration:
	 * - ItemDataTable
	 * - ItemID
	 * - bHasInventory
	 * - EditorPreviewMaxDistance
	 * 
	 * Inherited State:
	 * - bIsCurrentlyInteractable
	 * - bIsPlayerNearby
	 * - bInteractionEnabled
	 * - bShowPreInteraction
	 * - bShowFullInteraction
	 * - bEnableInteractionNotifications
	 * - bInteractionNotificationsAllowManualHandling
	 * 
	 * Inherited Asset Management:
	 * - AssetPackage
	 * - bIsLoadingAssets
	 * 
	 * Inherited Helper Functions:
	 * - All protected helper functions from parent class
	 */
};