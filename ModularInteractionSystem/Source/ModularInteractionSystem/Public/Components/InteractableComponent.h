// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Windwalker_Productions_SharedDefaults.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"
#include "InteractionSystem/InteractionDefaultsConstants.h"
#include "Debug/DebugSubsystem.h"
#include "Interfaces/ModularSaveGameSystem/SaveableInterface.h"
#include "InteractableComponent.generated.h"

class UWidgetManagerBase;

//TODO: we have to set initial collision response for world static, dynamic and pawn to block
//TODO: we have to set object type to either world static or dynamic by default.
//TODO: make those variables read only and visible anywhere if possible.
/*These updates are mandatory for interaction system to detect interactables.
 *I added in version in interactable component of BP_Pickup_Master
 * However, it should be added to this component at cpp level to make it default. 
*/

/**
 * INTERACTABLE COMPONENT
 * 
 * Handles interaction logic with overlap detection and widget management
 * NOW WITH SPATIAL HASH MOVEMENT TRACKING!
 * 
 * FEATURES:
 * - Automatic overlap sphere for detection
 * - Widget management through WidgetManager subsystem
 * - Shows interaction prompt on overlap begin
 * - Hides interaction prompt on overlap end
 * - Tracks current interacting instigator
 * - AUTOMATIC SPATIAL HASH POSITION UPDATES for moving actors
 * 
 * USAGE:
 * 1. Add to any actor that should be interactable
 * 2. Set InteractionPromptClass in blueprint
 * 3. Set InteractionRadius for overlap detection
 * 4. For MOVING actors: Set bTrackMovementForSpatialHash = true
 * 5. Implement interaction logic in OnInteract event
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARINTERACTIONSYSTEM_API UInteractableComponent : public USphereComponent, public ISaveableInterface
{
	GENERATED_BODY()

public:
	// ========================================================================
	// CONSTRUCTION
	// ========================================================================
	
	UInteractableComponent();

	// ========================================================================
	// LIFECYCLE
	// ========================================================================
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitProperties() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========================================================================
	// CONFIGURATION
	// ========================================================================
	
	/** Widget class to show as interaction prompt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	TSubclassOf<UUserWidget> InteractionPromptClass;
	
	/** Radius for interaction detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	float InteractionRadius = InteractionDefaultsConstants::DefaultInteractableOverlapRadius;
	
	/** Z-order for the interaction prompt widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	int32 PromptZOrder = 10;
	

	
	/** Text to display on the interaction prompt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FText InteractionText = FText::FromString("Press E to interact");

	// ========================================================================
	// SPATIAL HASH MOVEMENT TRACKING - NEW
	// ========================================================================
	
	/** 
	 * Enable spatial hash position updates for moving actors
	 * Set TRUE for: NPCs, moving platforms, elevators, vehicles
	 * Leave FALSE for: Static items, chests, doors, pickups
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Spatial Hash")
	bool bTrackMovementForSpatialHash = false;
	
	/** 
	 * How far actor must move before updating spatial grid (in cm)
	 * Higher = better performance, but less precision
	 * Lower = more updates, but better accuracy
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Spatial Hash", meta = (EditCondition = "bTrackMovementForSpatialHash", ClampMin = "10", ClampMax = "500"))
	float MovementThreshold = 100.f;
	
	/** 
	 * How often to check for movement (in seconds)
	 * 0 = every frame, 0.1 = 10 times per second, 0.2 = 5 times per second
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Spatial Hash", meta = (EditCondition = "bTrackMovementForSpatialHash", ClampMin = "0.0", ClampMax = "1.0"))
	float MovementCheckInterval = 0.1f;
	
	/**
	 * Auto-detect if this actor is likely to move (checks for movement components)
	 * Called automatically in BeginPlay if bTrackMovementForSpatialHash is false
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactable|Spatial Hash")
	void AutoDetectMovement();
	
	/**
	 * Manually force spatial grid update (call this if you teleport the actor)
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactable|Spatial Hash")
	void ForceUpdateSpatialGrid();

	// ========================================================================
	// INTERACTION
	// ========================================================================
	
	FGameplayTag DebugTag_InventoryInteraction = FGameplayTag::EmptyTag;
	FGameplayTag DebugTag_Interaction = FGameplayTag::EmptyTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable| Tags")
	FGameplayTag InteractableTypeTag = FGameplayTag::EmptyTag;
	FGameplayTag DebugTag = WWDebugTags::Debug();
	FGameplayTagContainer  SearchQuery_Tag;

	UFUNCTION(BlueprintPure, Category = "Interactable| Config")
	bool GetIsEnabled() const;
	//Sets the values from debugdataasset according to component tag set on constructor
	void SetDebugTagsOnAnyVisualisation();
	/**
	 * Attempt to interact with this object
	 * @param Instigator - Actor attempting to interact
	 * @return True if interaction was successful
	 */
	void SetupInteractableTypeTag();
	UFUNCTION(BlueprintCallable, Category = "Interactable")
	FGameplayTag GetInteractableDebugTag();
	
	void SetupInitialShapeColor();
	FDebugLayerSettings* LayerSettings;

	
	bool Interact(AActor* Instigator);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interactable")
	void OnInteract(AActor* Instigator);
	
	/**
	 * Get the current actor that is in range to interact
	 */
	UFUNCTION(BlueprintPure, Category = "Interactable")
	AActor* GetInteractableInstigator() const { return InteractableInstigator; }
	
	/**
	 * Check if someone is currently in range to interact
	 */
	UFUNCTION(BlueprintPure, Category = "Interactable")
	bool IsInstigatorInRange() const { return InteractableInstigator != nullptr; }
	
	/**
	 * Enable or disable this interactable
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void SetEnabled(bool bEnabled);

	// ========================================================================
	// WIDGET MANAGEMENT
	// ========================================================================

	/** Current actor in range to interact - INITIALLY NULL */
	UPROPERTY()
	TObjectPtr<AActor> InteractableInstigator = nullptr;
	
	/**
	 * Get the current interaction prompt widget instance
	 */
	UFUNCTION(BlueprintPure, Category = "Interactable")
	UUserWidget* GetPromptWidget() const { return PromptWidgetInstance; }
	
	/**
	 * Update the interaction prompt text
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void UpdatePromptText(const FText& NewText);

	void OnInteractableTypeTagNotFound();

protected:
	
	UFUNCTION()
	void OnRep_IsEnabled();

	// ========================================================================
	// Config
	// ========================================================================
	
	/** Can this interactable be used right now? */
	UPROPERTY(SaveGame, ReplicatedUsing=OnRep_IsEnabled, EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	bool bIsEnabled = true;

	// ========================================================================
	// OVERLAP EVENTS
	// ========================================================================
	
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// ========================================================================
	// SPATIAL HASH MOVEMENT TRACKING - NEW
	// ========================================================================
	
	/** Last known location for movement detection */
	FVector LastKnownLocation;
	
	/** Time accumulator for movement checks */
	float MovementCheckTimeAccumulator;
	
	/** Update spatial grid if actor has moved significantly */
	void UpdateSpatialGridPosition();
	
	/** Check if owner has movement-related components */
	bool HasMovementComponents() const;

	// ========================================================================
	// HELPERS
	// ========================================================================
	
	/** Get the widget manager for the local player */
	UWidgetManagerBase* GetWidgetManager() const;
	
	/** Check if the actor can interact (e.g., is it a player?) */
	virtual bool CanActorInteract(AActor* Actor) const;
	
	/** Show the interaction prompt */
	void ShowPrompt();
	
	/** Hide the interaction prompt */
	void HidePrompt();

	UPROPERTY(Transient)
	AActor* Owner = nullptr; 

private:

	
	/** Current prompt widget instance */
	UPROPERTY()
	TObjectPtr<UUserWidget> PromptWidgetInstance = nullptr;
	
	/** Cached widget manager reference */
	UPROPERTY()
	TObjectPtr<UWidgetManagerBase> CachedWidgetManager = nullptr;

	// ============================================================================
	// SAVE SYSTEM (ISaveableInterface)
	// ============================================================================

	virtual FString GetSaveID_Implementation() const override;
	virtual int32 GetSavePriority_Implementation() const override;
	virtual FGameplayTag GetSaveType_Implementation() const override;
	virtual bool SaveState_Implementation(FSaveRecord& OutRecord) override;
	virtual bool LoadState_Implementation(const FSaveRecord& InRecord) override;
	virtual bool IsDirty_Implementation() const override;
	virtual void ClearDirty_Implementation() override;
	virtual void OnSaveDataLoaded_Implementation() override;

	bool bSaveDirty = false;
	void MarkSaveDirty();
};