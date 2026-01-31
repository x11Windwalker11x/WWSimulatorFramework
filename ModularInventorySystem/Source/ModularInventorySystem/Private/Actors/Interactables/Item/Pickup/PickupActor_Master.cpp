// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactables/Item/Pickup/PickupActor_Master.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

APickupActor_Master::APickupActor_Master()
{
	// Call parent constructor - all initialization happens there
	// Add pickup-specific initialization here if needed in the future
	if (InteractableComponent)
	{
		InteractableComponent->InteractableTypeTag = FGameplayTag::RequestGameplayTag(FName("Interactable.Item"));
		InteractableComponent->OnInteractableTypeTagNotFound();
	}
}

// ============================================================================
// NOTE: All functionality is inherited from AInteractableActor_Master
// ============================================================================

/*
 * This class intentionally delegates all functionality to its parent class.
 * 
 * The parent class (AInteractableActor_Master) implements:
 * - Component creation and setup
 * - Lifecycle methods (BeginPlay, EndPlay, Tick, etc.)
 * - Asset loading and management
 * - Interaction interface implementation
 * - Debug functionality
 * - Editor preview functionality
 * 
 * Override methods in this class ONLY when pickup-specific behavior is required.
 * 
 * Example override pattern:
 * 
 * void APickupActor_Master::BeginPlay()
 * {
 *     Super::BeginPlay(); // Always call parent first
 *     
 *     // Add pickup-specific initialization here
 * }
 */