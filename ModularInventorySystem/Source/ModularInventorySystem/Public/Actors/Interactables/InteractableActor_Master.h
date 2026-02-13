// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Debug/DebugSubsystem.h"
#include "InteractionSystem/InteractionDefaultsConstants.h"
#include "Components/ActorComponent.h"
#include "Components/InteractableComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Interfaces/ModularInteractionSystem//InteractableInterface.h"
#include "Interfaces/SimulatorFramework/PhysicalInteractionInterface.h"
#include "Interfaces/ModularSaveGameSystem/SaveableInterface.h"
#include "Lib/Data/ModularSaveGameSystem/ActorSaveData.h"
#include "InteractionSubsystem.h"
#include "InteractionSystem/InteractionPredictionState.h"
#include "Interfaces/SimulatorFramework/PhysicalInteractionInterface.h"
#include "Actors/Interactables/Item/Data/ItemData.h"
#include "Actors/Interactables/Item/Data/ItemData_Dynamic.h"
#include "Components/OutlineComponent.h"
#include "Components/DurabilityComponent.h"
#include "Subsystems/InventoryWidgetManager.h"
#include "Engine/StreamableManager.h"
#include "Lib/Data/ModularInteractionSystem/InteractionPredictionState.h"
#include "InteractableActor_Master.generated.h"

/**
 * Base class for all interactable actors in the world.
 * Supports proximity-based asset loading and interaction system integration.
 * This is abstract - extend in Blueprints or C++ for concrete interactables.
 */
UCLASS(Abstract, Blueprintable)
class MODULARINVENTORYSYSTEM_API AInteractableActor_Master : public AActor, public IInteractableInterface, public IPhysicalInteractionInterface, public ISaveableInterface
{
	GENERATED_BODY()

public:
	// ============================================================================
	// CONSTRUCTOR & LIFECYCLE
	// ============================================================================
	
	AInteractableActor_Master();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// PUBLIC INTERFACE - ASSET LOADING
	// ============================================================================
	
	UFUNCTION(BlueprintCallable, Category="Asset Loading")
	void LoadItemAssets();
    
	UFUNCTION(BlueprintCallable, Category="Asset Loading")
	bool AreAssetsReady() const { return AssetPackage.bIsLoaded; }
    
	UFUNCTION(BlueprintCallable, Category="Asset Package")
	const FItemData_Dynamic& GetAssetPackage() const { return AssetPackage; }

	// ============================================================================
	// PUBLIC INTERFACE - INTERACTION QUERIES
	// ============================================================================
	
	UFUNCTION(BlueprintPure, Category="Interaction|UI")
	float GetPreInteractionDistance() const;
	
	UFUNCTION(BlueprintPure, Category="Interaction|UI")
	float GetFullInteractionDistance() const;
	
	UFUNCTION(BlueprintPure, Category="Interaction|UI")
	float GetUIUpdateRate() const;
	
	UFUNCTION(BlueprintCallable, Category="Interaction|UI")
	float GetGlobalLoadDistance() const;

	UFUNCTION(BlueprintImplementableEvent, Category="UI")
	void OnInteractionStateChanged();

	// ============================================================================
	// IINTERACTABLEINTERFACE IMPLEMENTATION
	// ============================================================================
	
	virtual void OnInteract_Implementation(AController* InstigatorController) override;
	virtual FItemData_Dynamic GetPickupData_Implementation() const override;
	virtual void SetPickupData_Implementation(FItemData_Dynamic& InPickupData) override;
	virtual UActorComponent* GetInventoryComponentAsActorComponent_Implementation() override;
	virtual UActorComponent* GetDurabilityComponentAsActorComponent_Implementation() override;
	virtual bool IsCurrentlyInteractable_Implementation() override;
	virtual void OnPlayerNearbyInit_Implementation(APawn* PlayerPawn, bool bIsNearby) override;
	virtual void SetInteractionEnabled_Implementation(bool bIsEnabled) override;
	virtual bool GetInteractionEnabled_Implementation() override;
	virtual void SetInteractionNotifications_Implementation() override;
	virtual bool GetbInteractionNotificationsAllowManualHandling_Implementation() override;
	virtual void OnPreInteractionEntered_Implementation(APawn* OtherPawn) override;
	virtual void OnFullInteractionEntered_Implementation(APawn* OtherPawn) override;
	virtual void OnPreInteractionExited_Implementation(APawn* OtherPawn) override;
	virtual void OnFullInteractionExited_Implementation(APawn* OtherPawn) override;
	virtual void LoadAssets_Implementation() override;
	virtual void UnloadAssets_Implementation() override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	virtual UActorComponent* GetOutlineComponent_Implementation() override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	virtual void SetOutlineActive_Implementation(bool bActivate) override;

	// Override prediction interface functions
	virtual void OnInteractPredicted_Implementation(AController* InstigatorController) override;
	virtual void CachePredictionState_Implementation(FInteractableState& OutState) override;
	virtual void RestorePredictionState_Implementation(const FInteractableState& State) override;

	//Physcial Interaction Interface
	virtual bool CanBeGrabbed_Implementation() const override;
	virtual void OnGrabbed_Implementation(AActor* GrabbingActor) override;
	virtual void OnReleased_Implementation(AActor* ReleasingActor) override;
	virtual FVector GetGrabOffset_Implementation() const override;
	virtual UPrimitiveComponent* GetGrabComponent_Implementation() const override;
	virtual float GetGrabMass_Implementation() const override;
	virtual bool IsBeingHeld_Implementation() const override;

	// ============================================================================
	// DELEGATES
	// ============================================================================
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionStateChanged, bool, bIsEnabled);
	
	UPROPERTY(BlueprintAssignable, Category="Interaction Events", meta=(EditCondition="bEnableInteractionNotifications"))
	FOnInteractionStateChanged InteractionStateChanged;

#if WITH_EDITOR
	// ============================================================================
	// EDITOR FUNCTIONALITY
	// ============================================================================
	
	UFUNCTION(CallInEditor, Category="Editor")
	void RefreshEditorMesh();
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ============================================================================
	// GETTERS & SETTERS
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	virtual void SetID_Implementation(FName NewID) override;
	
	/** Get the item ID */
	UFUNCTION(BlueprintPure, Category = "Interactable")
	FName GetID() const { return ID; }

	// Replication
	UFUNCTION()
	void OnRep_IsBeingHeld();

	UFUNCTION()
	void OnRep_HoldingActor();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Grab(AActor* GrabbingActor);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Release(AActor* ReleasingActor);


	
protected:
	// ============================================================================
	// COMPONENTS
	// ============================================================================
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	// USphereComponent* CollisionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* Root;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	UInteractableComponent* InteractableComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	UDurabilityComponent* DurabilityComponent;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	UOutlineComponent* OutlineComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|GFX")
	UStaticMeshComponent* StaticMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|GFX")
	USkeletalMeshComponent* SkeletalMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components|GFX|Preview")
	UStaticMeshComponent* PreviewMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components|GFX|Preview")
	USkeletalMeshComponent* PreviewSkeletalMesh;

	//--Physical Interaction
	UPROPERTY(ReplicatedUsing=OnRep_IsBeingHeld, BlueprintReadOnly, Category = "Physical Interaction")
	bool bIsBeingHeld = false;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Interaction")
	bool bCanBeGrabbed = true;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Interaction")
	FVector GrabOffset = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing=OnRep_HoldingActor, BlueprintReadOnly, Category = "Physical Interaction")
	TWeakObjectPtr<AActor> HoldingActor;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
	APlayerController* InteractableInstigator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	UDataTable* ItemDataTable;

	//ID to get the correspondant row
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	FName ID;

	//Config Data to pass on other modules
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Config|Pickup")
	FItemData_Dynamic PickupData;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	bool bHasInventory;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	float EditorPreviewMaxDistance = 2000.f;

	// ============================================================================
	// FALLBACK INTERACTION (when ModularInteractionSystem not present)
	// ============================================================================

	/** Fallback sphere for overlap interaction when InteractableComponent unavailable */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Fallback")
	USphereComponent* FallbackInteractionSphere;

	/** Is fallback mode active? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Fallback")
	bool bUsingFallbackInteraction = false;

	/** Fallback interaction radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Fallback")
	float FallbackInteractionRadius = 150.f;

	/** Setup fallback interaction if InteractableComponent missing */
	void SetupFallbackInteraction();

	/** Fallback overlap handlers */
	UFUNCTION()
	void OnFallbackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnFallbackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// ============================================================================
	// INTERACTION STATE
	// ============================================================================
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category="Interactable")
	bool bIsCurrentlyInteractable = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interactable")
	bool bIsPlayerNearby = false;
	
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category="Interactable")
	bool bInteractionEnabled = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bShowPreInteraction = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bShowFullInteraction = false;

	// ============================================================================
	// NOTIFICATIONS
	// ============================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Notifications")
	bool bEnableInteractionNotifications = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Notifications")
	bool bInteractionNotificationsAllowManualHandling = false;

	// ============================================================================
	// ASSET PACKAGE & LOADING
	// ============================================================================
	
	UPROPERTY(BlueprintReadOnly, Category="Asset Package")
	FItemData_Dynamic AssetPackage;
	
	UPROPERTY(BlueprintReadOnly, Category="Loading")
	bool bIsLoadingAssets = false;

	// ============================================================================
	// DEFAULT VALUES
	// ============================================================================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Config|Defaults")
	float DefaultLoadDistance = 500.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Config|Defaults")
	float DefaultPreInteractionDistance = 300.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Config|Defaults")
	float DefaultFullInteractionDistance = 100.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Config|Defaults")
	float DefaultUIUpdateRate = 0.1f;

	// ============================================================================
	// DEBUG HELPERS (Protected so derived classes can access)
	// ============================================================================
	
	/** Debug subsystem reference */
	UDebugSubsystem* DebugSubsystem;
	
	/** Debug tag for inventory interactions */
	FGameplayTag DebugTag_InventoryInteraction;
	
	/** Get or cache debug subsystem */
	UDebugSubsystem* GetDebugSubsystem();

	// ============================================================================
	// ASSET LOADING TRACKING (Protected for derived classes)
	// ============================================================================
	
	/** Total number of assets to load */
	int32 TotalAssetsToLoad = 0;
	
	/** Number of assets successfully loaded */
	int32 AssetsLoadedCount = 0;
	
	/** Active streamable handles for async loading */
	TArray<TSharedPtr<FStreamableHandle>> StreamableHandles;
	
	/** Cached reference to local player pawn */
	UPROPERTY(Transient)
	APawn* LocalPlayerPawn;

	/** Soft reference to static mesh */
	TSoftObjectPtr<UStaticMesh> SoftStaticMesh;
	
	/** Soft reference to skeletal mesh */
	TSoftObjectPtr<USkeletalMesh> SoftSkeletalMesh;

	// ============================================================================
	// HELPER FUNCTIONS (Protected for derived class access)
	// ============================================================================
	
	/** Check for existing overlaps with player */
	void CheckExistingOverlaps();
	
	/** Determine if pawn is a relevant player */
	bool IsRelevantPlayer(APawn* PlayerPawn);
	
	/** Cache basic item information from data table */
	void CacheBasicItemInfo();
	
	/** Start proximity-based asset loading */
	void StartProximityAssetLoading();
	
	/** Called when a single asset finishes loading */
	void OnSingleAssetLoaded(const FString& AssetType);
	
	/** Called when all assets finish loading */
	void OnAssetLoadingComplete();
	
	/** Called when asset loading is cancelled */
	void OnAssetLoadingCancelled();
	
	/** Apply loaded meshes to components */
	void ApplyLoadedMeshes();
	
	/** Apply soft mesh references */
	void ApplySoftMeshRefs();
	
	/** Unload assets and clear meshes */
	void UnloadAssetsAndMeshes();
	
	/** Ensure component is properly attached */
	void EnsureComponentAttachment(USceneComponent* Component, const FString& ComponentName);

#if WITH_EDITOR
	// ============================================================================
	// EDITOR HELPERS (Protected for derived classes)
	// ============================================================================
	
	/** Get active data table for editor operations */
	UDataTable* GetActiveDataTable();
	
	/** Try loading static mesh preview in editor */
	bool TryLoadStaticMeshPreview(const FItemData* Row, FStreamableManager& StreamableManager);
	
	/** Try loading skeletal mesh preview in editor */
	bool TryLoadSkeletalMeshPreview(const FItemData* Row, FStreamableManager& StreamableManager);
	
	/** Load static mesh asynchronously */
	bool LoadStaticMeshAsync(const TSoftObjectPtr<UStaticMesh>& MeshRef, FStreamableManager& StreamableManager, const FString& MeshType);
	
	/** Load skeletal mesh asynchronously */
	bool LoadSkeletalMeshAsync(const TSoftObjectPtr<USkeletalMesh>& MeshRef, FStreamableManager& StreamableManager, const FString& MeshType);
#endif

	// Internals
	bool Internal_Grab(AActor* GrabbingActor);
	bool Internal_Release(AActor* ReleasingActor);

	// ============================================================================
	// SAVE SYSTEM (ISaveableInterface) — ORCHESTRATOR PATTERN
	// ============================================================================

	virtual FString GetSaveID_Implementation() const override;
	virtual int32 GetSavePriority_Implementation() const override;
	virtual FGameplayTag GetSaveType_Implementation() const override;
	virtual bool SaveState_Implementation(FSaveRecord& OutRecord) override;
	virtual bool LoadState_Implementation(const FSaveRecord& InRecord) override;
	virtual bool IsDirty_Implementation() const override;
	virtual void ClearDirty_Implementation() override;
	virtual void OnSaveDataLoaded_Implementation() override;

	/** Build complete actor + component save envelope */
	FActorSaveEnvelope SaveActorWithComponents() const;

	/** Restore actor + component state from envelope */
	bool LoadActorWithComponents(const FActorSaveEnvelope& Envelope);

	bool bSaveDirty = false;
	void MarkSaveDirty();
};