// InteractionSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsModule.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InteractionSystem/InteractionDefaultsConstants.h"
#include "Components/InteractableComponent.h"
#include "InteractionSystem/InteractionPredictionState.h"
#include "WW_Enums.h"
#include "SpatialHashGrid.h"  // ✅ Complete definition!
#include "InteractionSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableFocused, AActor*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableUnfocused, AActor*, Interactable);

UCLASS(Config = Game)
class MODULARINTERACTIONSYSTEM_API UInteractionSubsystem : public UGameInstanceSubsystem,  public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Singleton-like accessor
	static UInteractionSubsystem* Get(const UWorld* World);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Get spatial hash grid pointer for console commands */
	FSpatialHashGrid* GetSpatialGrid() const { return SpatialGrid.Get(); }
	
	/** Enable/disable spatial hashing (for console commands) */
	void SetUseSpatialHashing(bool bEnabled) { bUseSpatialHashing = bEnabled; }


	// Tickable interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr; }

	/** Enable automatic player focus updates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance")
	bool bAutoUpdatePlayerFocus = true;
    
	/** How often to update player focus (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float PlayerFocusUpdateInterval = InteractionDefaultsConstants::GlobalInteractionFocusUpdateRate_Player;
    
	/** Register a player pawn for automatic focus updates */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RegisterPlayerPawn(APawn* PlayerPawn);
    
	/** Unregister a player pawn */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UnregisterPlayerPawn(APawn* PlayerPawn);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetAllPlayersUpdateRate(float NewRate);
	


	


	 /** Register/unregister interactables in world */
    void RegisterInteractable(AActor* Actor);
    void UnregisterInteractable(AActor* Actor);
	void RegisterNearbyItembyPawn(AActor* Actor, APawn* Pawn);
	
	/** SYNC VERSION - Updates item priorities on game thread (legacy) */
	void UpdateNearbyItemPriorities(float UpdateInterval = 0.5f);
	
	/** ASYNC VERSION - Updates item priorities on background thread (preferred) */
	UFUNCTION(BlueprintCallable, Category = "Interaction | AI | Async")
	void UpdateNearbyItemPrioritiesAsync();

	// ========================================================================
	// SPATIAL HASH GRID API - NEW
	// ========================================================================
	
	/** Get nearby interactables using spatial hash (10-40x faster than brute force) */
	UFUNCTION(BlueprintCallable, Category = "Interaction | Spatial Hash")
	TArray<AActor*> GetNearbyInteractables(const FVector& Location, float Radius = 1000.f) const;
	
	/** Notify spatial grid that an actor has moved (call only for dynamic actors) */
	UFUNCTION(BlueprintCallable, Category = "Interaction | Spatial Hash")
	void NotifyActorMoved(AActor* Actor, const FVector& OldLocation);
	
	/** Debug: Draw spatial grid visualization */
	UFUNCTION(BlueprintCallable, Category = "Interaction | Spatial Hash | Debug")
	void DebugDrawSpatialGrid(const FVector& Center, int32 Radius = 5);
	
	/** Debug: Print spatial grid statistics to log */
	UFUNCTION(BlueprintCallable, Category = "Interaction | Spatial Hash | Debug")
	void PrintSpatialGridStats();

	/** Events for UI / outline */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractableFocused OnInteractableFocused;

	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractableUnfocused OnInteractableUnfocused;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Interaction Distances")
	float PreInteractionDistanceLoad = InteractionDefaultsConstants::PreInteractionDistanceLoad;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Interaction Distances")
	float PreInteractionDistanceUI = InteractionDefaultsConstants::InteractionDistanceUI;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Interaction Distances")
	float InteractionDistanceUI = InteractionDefaultsConstants::PreInteractionDistanceUI;



//Funcs:
	/** Update focused actor for UI/outline */
	UFUNCTION(BlueprintCallable, Category = "ItemInteraction", meta = (ToolTip = "Simple update focus, one time for ui, works for every type of controller"))
	void UpdateFocus(APawn* Pawn, AActor* NewFocus);
	
	// Called when player tries to interact with something
	UFUNCTION(BlueprintCallable, Category = "ItemInteraction")
	bool TryInteract(APlayerController* InstigatorController);
	
	// New overload with explicit view parameters for accurate client interaction
	bool TryInteract(APlayerController* InstigatorController, FVector ViewLocation, FRotator ViewRotation);
	
	/** ASYNC VERSION - For player controllers only, updates very frequently, uses async traces */
	UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (ToolTip = "For player controllers only, updates very frequently, uses ASYNC trace."))
	void UpdatePlayerFocusAsync(APawn* Pawn, float SweepRadius, float SweepHalfHeight, float SweepDistance);
	
	/** LEGACY SYNC VERSION - Still available for compatibility */
	UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (ToolTip = "LEGACY: For player controllers only, updates very frequently, uses SYNC trace."))
	void UpdatePlayerFocus(APawn* Pawn, float SweepRadius, float SweepHalfHeight, float SweepDistance);
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetBestInteractableForPlayer(APawn* Pawn) const;
	
	// Version with explicit view parameters for accurate client-server interaction
	AActor* GetBestInteractableForPlayerWithViewParams(APawn* Pawn, FVector ViewLocation, FRotator ViewRotation) const;
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetBestInteractableForAI(APawn* AIPawn) const;
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetFocusedActorForPawn(APawn* Pawn) const;
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	int32 GetGameplayTagPriority(AActor* Actor, bool bGetMax = true) const;
	
	UFUNCTION(BlueprintPure, Category = "Interaction | Config")
	float GetPreInteractionDistance() const { return GlobalLoadingDistance_Items; }
	
	UFUNCTION(BlueprintPure, Category = "Interaction | Config")
	float GetFullInteractionDistance() const { return GlobalFullInteractionDistanceUI; }
	
	UFUNCTION(BlueprintPure, Category = "Interaction | Config")
	float GetUIUpdateRate() const { return GlobalUIUpdateRate; }
	
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetGlobalInteractionNotifications(bool bEnabled);
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	static bool IsInteractableItem(AActor* Actor);
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	static bool IsInteractableActor(AActor* Actor);

	// ========================================================================
	// ASYNC TRACE SYSTEM - NEW
	// ========================================================================
	
	/** Request async focus update for a pawn (preferred over UpdatePlayerFocusAsync for manual calls) */
	UFUNCTION(BlueprintCallable, Category = "Interaction | Async")
	void RequestAsyncFocusUpdate(APawn* Pawn);
	
	/** Register AI pawn for staggered async trace updates */
	UFUNCTION(BlueprintCallable, Category = "Interaction | AI")
	void RegisterAIPawn(APawn* AIPawn);
	
	/** Unregister AI pawn from staggered updates */
	UFUNCTION(BlueprintCallable, Category = "Interaction | AI")
	void UnregisterAIPawn(APawn* AIPawn);

	// ========================================================================
	// BATCHED TRACE FUNCTIONS
	// ========================================================================

	/** Update focus for ALL registered players in a single batch (most efficient!) */
	UFUNCTION(BlueprintCallable, Category = "Interaction | Batched")
	void UpdateAllPlayersFocus_Batched();

	/** Launch the batch of collected traces */
	void LaunchBatchTraces();

	/** Callback when a batched trace completes */
	void OnBatchedTraceComplete(const FTraceHandle& Handle, FTraceDatum& Data, int32 RequestID);

	/** Called when all traces in batch are complete */
	void OnBatchComplete();

	/** Server-side: Request validation for client interaction (called by InteractorComponent) */
	void RequestServerValidation(APawn* ClientPawn, AActor* TargetActor);

	
//Vars:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
	UDataTable* InteractableTagPriorityTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Config")
	float GlobalLoadingDistance_Items= InteractionDefaultsConstants::PreInteractionDistanceLoad;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Config")
	float GlobalLoadingDistance_Interactables = InteractionDefaultsConstants::PreInteractionDistanceLoad;
	/** Enable batched traces for better performance with multiple players (requires bUseAsyncTraces = true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Async")
	bool bUseBatchedTraces = InteractionDefaultsConstants::bUseBatchedTraces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Config | UI")
    float GlobalPreInteractionDistanceUI = InteractionDefaultsConstants::PreInteractionDistanceUI;  // Shows simple icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Config | UI")
	float GlobalFullInteractionDistanceUI = InteractionDefaultsConstants::InteractionDistanceUI; 
    float GlobalUIUpdateRate = InteractionDefaultsConstants::GlobalUIUpdateRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance")
	bool bEnableGlobalInteractionNotifications = InteractionDefaultsConstants::bEnableGlobalInteractionNotifications;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FGameplayTag InteractableTypeTag;

	// ========================================================================
	// PLAY MODE DETECTION
	// ========================================================================

	/** Cached play mode for optimization */
	UPROPERTY()
	EPlayMode CachedPlayMode;

	/** Detect current play mode */
	EPlayMode DetectPlayMode() const;

	/** Should we use batched traces for current play mode? */
	bool ShouldUseBatchedTraces() const;

	/** Get local player pawns only (for split-screen batching) */
	void GetLocalPlayerPawns(TArray<APawn*>& OutLocalPawns) const;
	
	// ========================================================================
	// ASYNC TRACE CONFIGURATION
	// ========================================================================
	
	/** Enable async traces (set to false to use legacy sync traces) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Async")
	bool bUseAsyncTraces = true;
	
	/** Enable async AI priority calculations (set to false for legacy sync processing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Async")
	bool bUseAsyncAIProcessing = true;
	
	/** How often to update AI item priorities (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Async", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float AIUpdateInterval = InteractionDefaultsConstants::GlobalInteractionFocusUpdateRate;
	
	/** Capsule radius for player interaction traces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Async")
	float PlayerTraceCapsuleRadius = 30.f;
	
	/** Capsule half-height for player interaction traces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Async")
	float PlayerTraceCapsuleHalfHeight = 30.f;
	
	/** Maximum angle from view center for valid interactions (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Async")
	float MaxInteractionAngle = 45.f;

	// ========================================================================
	// SPATIAL HASH GRID CONFIGURATION - NEW
	// ========================================================================
	
	/** Enable spatial hash grid for optimized actor queries (10-40x faster in dense areas) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Spatial Hash")
	bool bUseSpatialHashing = true;
	
	/** Grid cell size in world units (1000 = 10 meters, Fortnite standard) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Spatial Hash", meta = (ClampMin = "500", ClampMax = "5000"))
	float SpatialGridCellSize = 1000.f;
	
	/** How many surrounding cells to check (1 = 3x3 grid, 2 = 5x5 grid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Spatial Hash", meta = (ClampMin = "1", ClampMax = "3"))
	int32 SpatialQueryCellRadius = 1;
	
	/** Time before stale traces are cleaned up (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Async")
	float TraceTimeoutDuration = 1.f;
	
	/** How many AI pawns to update per frame (tune based on AI count) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance | Async")
	int32 AITracesPerFrame = 5;
	
	//Optimisation
	//Distance based Item Prioritization
	struct FNearbyItemInfo
	{
		TWeakObjectPtr<AActor> Item;
		float DistanceSquared;
		bool bIsLoaded;
	};
	TMap<TWeakObjectPtr<APawn>, TArray<FNearbyItemInfo>> NearbyItemsbyPawn;

	//Only load top N closest items
	UPROPERTY(EditAnywhere, Category = "Interaction | Performance")
	int32 MaxSimultaneouslyLoadedItems = InteractionDefaultsConstants::MaxSimultaneouslyLoadedItems;
	
	//Optimisation for ai. Note: if false, Ai can pick up items through walls. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	bool bEnableAILineOfSight = true; // Toggle in editor!

	
protected:
	/** All interactables in the world */
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> Interactables;
	
	// ========================================================================
	// SPATIAL HASH GRID INSTANCE - NEW
	// ========================================================================
	
	/** Spatial hash grid for optimized actor queries (10-40x faster!) */
	TUniquePtr<FSpatialHashGrid> SpatialGrid;
	
	// Internal helper to perform interaction logic
	bool PerformInteraction(AController* InstigatorController);

	/** Track focused actor per pawn */
	TMap<TWeakObjectPtr<APawn>, TWeakObjectPtr<AActor>> FocusedActors;

	// ========================================================================
	// ASYNC TRACE INTERNALS
	// ========================================================================
	
	/** Active async traces for player focus updates */
	struct FActivePlayerTrace
	{
		TWeakObjectPtr<APawn> Pawn;
		FTraceHandle Handle;
		float StartTime;
		float SweepRadius;
		float SweepHalfHeight;
		float SweepDistance;
		
		FActivePlayerTrace() : StartTime(0.f), SweepRadius(30.f), SweepHalfHeight(30.f), SweepDistance(500.f) {}
		FActivePlayerTrace(APawn* InPawn, const FTraceHandle& InHandle, float InRadius, float InHalfHeight, float InDistance)
			: Pawn(InPawn), Handle(InHandle), StartTime(0.f), SweepRadius(InRadius), SweepHalfHeight(InHalfHeight), SweepDistance(InDistance) {}
	};

	// ========================================================================
	// MODE-SPECIFIC UPDATE FUNCTIONS
	// ========================================================================

	/** Update for single player mode (1 local player, no network) */
	void UpdateSinglePlayer();

	/** Update for split-screen mode (2-4 local players, same machine) */
	void UpdateSplitScreen();

	/** Update for listen server mode (host + remote clients) */
	void UpdateListenServer();

	/** Update for client mode (remote player connected to server) */
	void UpdateClient();

	/** Fallback update (individual async traces) */
	void UpdateFallback();	

	// ========================================================================
	// BATCHED TRACE SYSTEM - NEW
	// ========================================================================

	/** Request data for batched player focus traces */
	struct FBatchTraceRequest
	{
		TWeakObjectPtr<APawn> Pawn;
		FVector Start;
		FVector End;
		FQuat Rotation;
		FCollisionShape Shape;
		FTraceHandle Handle;
		int32 RequestID;
		float SweepRadius;
		float SweepHalfHeight;
		float SweepDistance;
    
		FBatchTraceRequest()
			: Rotation(FQuat::Identity)
			, RequestID(-1)
			, SweepRadius(30.f)
			, SweepHalfHeight(30.f)
			, SweepDistance(500.f)
		{}
	};

	/** Pending batch trace requests */
	TArray<FBatchTraceRequest> PendingBatchTraces;

	/** Count of completed traces in current batch */
	int32 CompletedBatchTraces;

	/** Total traces launched in current batch */
	int32 TotalBatchTraces;

	
	TArray<FActivePlayerTrace> ActivePlayerTraces;
	
	/** Launch async trace for player focus update */
	void LaunchAsyncPlayerFocusTrace(APawn* Pawn, float SweepRadius, float SweepHalfHeight, float SweepDistance);
	
	/** Callback when player focus trace completes */
	void OnPlayerFocusTraceComplete(const FTraceHandle& Handle, FTraceDatum& Data, TWeakObjectPtr<APawn> Pawn);
	
	/** Process trace results and determine best interactable from hits */
	AActor* ProcessAsyncTraceResults(const TArray<FHitResult>& Hits, APawn* Pawn);
	
	/** Cleanup stale traces that didn't complete */
	void CleanupStaleTraces();

	// ========================================================================
	// ASYNC AI PROCESSING INTERNALS
	// ========================================================================
	
	/** Data structure for AI processing on background thread */
	struct FAIPriorityData
	{
		TWeakObjectPtr<APawn> Pawn;
		FVector PawnLocation;
		TArray<TPair<TWeakObjectPtr<AActor>, float>> ItemsWithDistances; // Item + DistanceSquared
		
		FAIPriorityData() : PawnLocation(FVector::ZeroVector) {}
	};
	
	/** Callback when AI priority calculations complete */
	void OnAIPriorityCalculationsComplete(TArray<FAIPriorityData> ProcessedData);
	
	/** Helper: Apply load/unload to items based on new priority order */
	void ApplyAIItemLoadState(APawn* Pawn, const TArray<TPair<TWeakObjectPtr<AActor>, float>>& SortedItems);

	// ========================================================================
	// SERVER-SIDE VALIDATION (For Listen Server with Clients)
	// ========================================================================
	
	/** Pending validation requests from clients */
	struct FPendingValidation
	{
		TWeakObjectPtr<APawn> ClientPawn;
		TWeakObjectPtr<AActor> TargetActor;
		FVector ClientViewLocation;
		FRotator ClientViewRotation;
		float RequestTime;
		
		FPendingValidation() : RequestTime(0.f) {}
	};
	
	TArray<FPendingValidation> PendingValidations;
	
	/** Batch validation handles */
	struct FBatchValidationTrace
	{
		FTraceHandle Handle;
		TWeakObjectPtr<APawn> Pawn;
		TWeakObjectPtr<AActor> TargetActor;
	};
	
	TArray<FBatchValidationTrace> ActiveValidations;
	
	/** Launch batch validation traces (server-side) */
	void LaunchBatchValidation();
	
	/** Callback when validation trace completes */
	void OnValidationTraceComplete(const FTraceHandle& Handle, FTraceDatum& Data, 
		TWeakObjectPtr<APawn> Pawn, TWeakObjectPtr<AActor> TargetActor);
	
	/** Validate if target is within interaction range and line of sight */
	bool ValidateInteraction(const TArray<FHitResult>& Hits, APawn* Pawn, AActor* TargetActor);

	// ========================================================================
	// AI STAGGERED UPDATES
	// ========================================================================
	
	/** AI pawns registered for staggered trace updates */
	TArray<TWeakObjectPtr<APawn>> RegisteredAIPawns;
	
	/** Current index for staggered AI updates */
	int32 CurrentAIUpdateIndex;

	/** Update AI traces in staggered manner */
	void TickStaggeredAITraces(float DeltaTime);
	
	/** Get pawn view point for trace */
	void GetPawnViewPoint(APawn* Pawn, FVector& OutLocation, FVector& OutForward) const;

	// ========================================================================
	// AI-Async Timer UPDATES
	// ========================================================================
	
	/** Timer handle for AI priority updates */
	FTimerHandle AIUpdateTimerHandle;
	
	/** Start the AI update timer */
	UFUNCTION(BlueprintCallable, Category = "Interaction | AI | Control")
	void StartAIUpdateTimer();
	
	/** Stop the AI update timer */
	UFUNCTION(BlueprintCallable, Category = "Interaction | AI | Control")
	void StopAIUpdateTimer();

	UFUNCTION(BlueprintCallable, Category = "Interaction | AI | Control")
	void RestartAIUpdateTimer();

	UFUNCTION(BlueprintCallable, Category = "Interaction | AI | Control")
	void SetAIUpdateInterval(float NewInterval);

	UFUNCTION(BlueprintCallable, Category = "Interaction | AI")
	void PauseAIUpdates();

	UFUNCTION(BlueprintCallable, Category = "Interaction | AI")
	void ResumeAIUpdates();

	/** Track registered player pawns for automatic updates */
	TSet<TWeakObjectPtr<APawn>> RegisteredPlayerPawns;
    
	/** Time accumulator for player focus updates */
	float PlayerFocusUpdateTimeAccumulator = 0.0f;
    
	/** Automatically update focus for all registered players */
	void TickPlayerFocusUpdates(float DeltaTime);


	

};