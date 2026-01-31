// InteractionSubsystem.cpp
#include "InteractionSubsystem.h"

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Interface/InteractionSystem/InteractableInterface.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagContainer.h"
#include "InteractablePriorityStruct.h"
#include "InterchangeResult.h"
#include "Components/InteractorComponent.h"
#include "Components/OutlineComponent.h"

#include "Interface/InteractionSystem/InteractorInterface.h"#include "Interface/InteractionSystem/InteractableInterface.h"
#include "Interfaces/ModularInteractionSystem/InteractableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "LevelInstance/LevelInstanceTypes.h"

UInteractionSubsystem* UInteractionSubsystem::Get(const UWorld* World)
{
    return World ? World->GetGameInstance()->GetSubsystem<UInteractionSubsystem>() : nullptr;
}

void UInteractionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// ========================================================================
	// SPATIAL HASH GRID INITIALIZATION - NEW
	// ========================================================================
	if (bUseSpatialHashing)
	{
		SpatialGrid = MakeUnique<FSpatialHashGrid>(SpatialGridCellSize);
		UE_LOG(LogInteractableSubsystem, Log, TEXT("🗺️ Spatial Hash Grid initialized (Cell Size: %.1f, Query Radius: %d cells)"), 
			SpatialGridCellSize, SpatialQueryCellRadius);
		
		// Register console commands for spatial hash debugging
		FSpatialHashGrid::RegisterConsoleCommands(this);
	}
	
	// Initialize async trace system
	CurrentAIUpdateIndex = 0;
	AITracesPerFrame = 5; // Default: 100 AI ÷ 5 = 20 frames = 0.33s cycle @ 60 FPS
	
	// ✅ START AI UPDATE TIMER AUTOMATICALLY
	StartAIUpdateTimer();
	
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🚀 InteractionSubsystem initialized with ASYNC TRACES enabled"));
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🤖 AI Priority Update Timer started (Interval: %.2fs)"), AIUpdateInterval);

	//Player Register and their traces
	CurrentAIUpdateIndex = 0;
	AITracesPerFrame = 5;
	PlayerFocusUpdateTimeAccumulator = 0.0f; // NEW
    
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🚀 InteractionSubsystem initialized with AUTO PLAYER FOCUS"));

	// Initialize batch trace variables
	CompletedBatchTraces = 0;
	TotalBatchTraces = 0;
	PendingBatchTraces.Empty();

	// Detect and cache play mode
	CachedPlayMode = DetectPlayMode();

	
	UE_LOG(LogInteractableSubsystem, Log, TEXT("InteractionSubsystem initialized with batched trace support - PlayMode: %d"), (int32)CachedPlayMode);


}

void UInteractionSubsystem::Deinitialize()
{
	// ✅ STOP AI UPDATE TIMER
	StopAIUpdateTimer();

	// Cleanup all active traces
	ActivePlayerTraces.Empty();
	ActiveValidations.Empty();
	PendingValidations.Empty();
	RegisteredAIPawns.Empty();
	FocusedActors.Empty();
	
	// ========================================================================
	// SPATIAL HASH GRID CLEANUP - NEW
	// ========================================================================
	if (SpatialGrid)
	{
		// Unregister console commands first
		FSpatialHashGrid::UnregisterConsoleCommands();
		
		SpatialGrid->Clear();
		SpatialGrid.Reset();
		UE_LOG(LogInteractableSubsystem, Log, TEXT("🗺️ Spatial Hash Grid cleaned up"));
	}
	
	Super::Deinitialize();
	
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🤖 AI Priority Update Timer stopped"));
	UE_LOG(LogInteractableSubsystem, Log, TEXT("InteractionSubsystem deinitialized"));
}
void UInteractionSubsystem::Tick(float DeltaTime)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;  // Safe exit nullptr case
	}
	
	// Cleanup stale traces
	CleanupStaleTraces();
    
	// Server-side batch validation
	UWorld* World = GetGameInstance()->GetWorld();
	if (World && World->GetNetMode() != NM_Client && PendingValidations.Num() > 0)
	{
		LaunchBatchValidation();
	}
    
	// AI staggered updates
	TickStaggeredAITraces(DeltaTime);
    
	// NEW: Automatic player focus updates
	if (bAutoUpdatePlayerFocus)
	{
		TickPlayerFocusUpdates(DeltaTime);
	}
}

TStatId UInteractionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UInteractionSubsystem, STATGROUP_Tickables);
}

void UInteractionSubsystem::TickPlayerFocusUpdates(float DeltaTime)
{
	if (!bAutoUpdatePlayerFocus)
		return;
    
	PlayerFocusUpdateTimeAccumulator += DeltaTime;
    
	if (PlayerFocusUpdateTimeAccumulator >= PlayerFocusUpdateInterval)
	{
		PlayerFocusUpdateTimeAccumulator = 0.0f;
        
		// NEW: Update ALL players in one batched call ✅
		UpdateAllPlayersFocus_Batched();
	}
}

void UInteractionSubsystem::RegisterPlayerPawn(APawn* PlayerPawn)
{
	if (!PlayerPawn)
		return;
    
	TWeakObjectPtr<APawn> WeakPawn(PlayerPawn);
	if (!RegisteredPlayerPawns.Contains(WeakPawn))
	{
		RegisteredPlayerPawns.Add(WeakPawn);
		UE_LOG(LogInteractableSubsystem, Log, TEXT("🎮 Registered player: %s (Total: %d)"), 
			*PlayerPawn->GetName(), RegisteredPlayerPawns.Num());
	}
}

void UInteractionSubsystem::UnregisterPlayerPawn(APawn* PlayerPawn)
{
	if (!PlayerPawn)
		return;
    
	TWeakObjectPtr<APawn> WeakPawn(PlayerPawn);
	if (RegisteredPlayerPawns.Remove(WeakPawn) > 0)
	{
		UE_LOG(LogInteractableSubsystem, Log, TEXT("🎮 Unregistered player: %s (Remaining: %d)"), 
			*PlayerPawn->GetName(), RegisteredPlayerPawns.Num());
	}
}

void UInteractionSubsystem::SetAllPlayersUpdateRate(float NewRate)
{
	
	PlayerFocusUpdateInterval = FMath::Clamp(NewRate, 0.05f, 0.5f);
	
}

void UInteractionSubsystem::RegisterInteractable(AActor* Actor)
{
	if (Actor)
		Interactables.Add(Actor);
	
	// ========================================================================
	// SPATIAL HASH GRID REGISTRATION - NEW
	// ========================================================================
	if (bUseSpatialHashing && SpatialGrid && Actor)
	{
		SpatialGrid->RegisterActor(Actor);
		UE_LOG(LogInteractableSubsystem, Log, TEXT("🗺️ Registered %s in spatial grid"), *Actor->GetName());
	}
	
	if (Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		if (IInteractableInterface::Execute_GetbInteractionNotificationsAllowManualHandling(Actor))
		{
			IInteractableInterface::Execute_SetInteractionNotifications(Actor);	
		}
		
	}
}

void UInteractionSubsystem::UnregisterInteractable(AActor* Actor)
{
    if (Actor)
        Interactables.Remove(Actor);
    
    // ========================================================================
    // SPATIAL HASH GRID UNREGISTRATION - NEW
    // ========================================================================
    if (bUseSpatialHashing && SpatialGrid && Actor)
    {
        SpatialGrid->UnregisterActor(Actor);
        UE_LOG(LogInteractableSubsystem, VeryVerbose, TEXT("🗺️ Unregistered %s from spatial grid"), *Actor->GetName());
    }
}

void UInteractionSubsystem::RegisterNearbyItembyPawn(AActor* Actor, APawn* Pawn)
{
	if (!Actor || !Pawn) return;
	
	TArray<FNearbyItemInfo>& ItemList = NearbyItemsbyPawn.FindOrAdd(Pawn);
	
	for (const FNearbyItemInfo& ExistingInfo : ItemList)
	{
		if (ExistingInfo.Item == Actor)
		{
			return; // Already registered
		}
	}

	FNearbyItemInfo Info;
	Info.Item = Actor;
	Info.DistanceSquared = FVector::DistSquared(Pawn->GetActorLocation(), Actor->GetActorLocation());
	Info.bIsLoaded = false;

	ItemList.Add(Info);
	//Sort items by distance from closest to farthest
	ItemList.Sort([](const FNearbyItemInfo& A, const FNearbyItemInfo& B) { return A.DistanceSquared < B.DistanceSquared;});

	//Load only top N
	for (int32 i = 0; i < FMath::Min(ItemList.Num(), MaxSimultaneouslyLoadedItems); i++)
	{
		if (ItemList[i].Item.IsValid())
		{
			IInteractableInterface::Execute_LoadAssets(ItemList[i].Item.Get());
			ItemList[i].bIsLoaded = true;
			
		}
	}
	
}

void UInteractionSubsystem::UpdateNearbyItemPriorities(float UpdateInterval)
{
	for (auto& Pair : NearbyItemsbyPawn)
	{
		APawn* Pawn = Pair.Key.Get();
		if (!Pawn) continue;

		TArray<FNearbyItemInfo>& Items = Pair.Value;

		//Recalculate distances
		for (FNearbyItemInfo& Info : Items)
		{
			if (Info.Item.IsValid())
			{
				Info.DistanceSquared = FVector::DistSquared(
					Pawn->GetActorLocation(), 
					Info.Item->GetActorLocation()
				);
			}
		}


		//Re-sort
        Items.Sort([](const FNearbyItemInfo& A, const FNearbyItemInfo& B) 
		{return A.DistanceSquared < B.DistanceSquared;});

		//Unload Items that fall out of top N
		for (int32 i = MaxSimultaneouslyLoadedItems; i < Items.Num(); i++)
		{
			if (Items[i].bIsLoaded && Items[i].Item.IsValid())
			{
				IInteractableInterface::Execute_UnloadAssets(Items[i].Item.Get());
				Items[i].bIsLoaded = false;
			}
		}
		//Load items entered top N
		for (int32 i = 0; i < FMath::Min(Items.Num(), MaxSimultaneouslyLoadedItems); ++i)
		{
			if (!Items[i].bIsLoaded && Items[i].Item.IsValid())
			{
				IInteractableInterface::Execute_LoadAssets(Items[i].Item.Get());
				Items[i].bIsLoaded = true;
			}
		}
	}
}

// ================================================================
// ASYNC AI Interaction - NEW IMPLEMENTATION
// ================================================================

//Calls UpdateNearbyItemPriorities func in separate thread.

void UInteractionSubsystem::UpdateNearbyItemPrioritiesAsync()
{
	// ⚡ NEW ASYNC VERSION - Does NOT block game thread!
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🔄 AI Update tick (AI count: %d)"), NearbyItemsbyPawn.Num());

	if (!bUseAsyncAIProcessing)
	{
		// Fallback to sync if disabled
		UpdateNearbyItemPriorities(0.5f);
		return;
	}
		// ========================================================================
	// STEP 1: Collect all AI data on GAME THREAD (fast - just copying data)
	// ========================================================================
	
	TArray<FAIPriorityData> AIDataBatch;
	AIDataBatch.Reserve(NearbyItemsbyPawn.Num());
	
	for (auto& Pair : NearbyItemsbyPawn)
	{
		APawn* Pawn = Pair.Key.Get();
		if (!Pawn)
			continue;
		
		// Copy pawn data and item list
		FAIPriorityData Data;
		Data.Pawn = Pawn;
		Data.PawnLocation = Pawn->GetActorLocation();
		
		// Copy items with their current data
		TArray<FNearbyItemInfo>& Items = Pair.Value;
		Data.ItemsWithDistances.Reserve(Items.Num());
		
		for (const FNearbyItemInfo& Info : Items)
		{
			if (Info.Item.IsValid())
			{
				Data.ItemsWithDistances.Add(TPair<TWeakObjectPtr<AActor>, float>(
					Info.Item,
					Info.DistanceSquared
				));
			}
		}
		
		AIDataBatch.Add(Data);
	}
	
	// If no AI to process, return early
	if (AIDataBatch.Num() == 0)
	{
		return;
	}
	
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🤖 Starting async AI priority calculations for %d pawns"), AIDataBatch.Num());
	
	// ========================================================================
	// STEP 2: Launch BACKGROUND THREAD for heavy calculations
	// ========================================================================
	
	Async(EAsyncExecution::ThreadPool, [AIDataBatch, this]() mutable
	{
		// 🧵 NOW ON BACKGROUND THREAD - Heavy math here!
		
		// Process each AI pawn
		for (FAIPriorityData& Data : AIDataBatch)
		{
			if (!Data.Pawn.IsValid())
				continue;
			
			// ========================================
			// HEAVY CALCULATIONS (off main thread!)
			// ========================================
			
			// Recalculate all distances
			for (auto& ItemPair : Data.ItemsWithDistances)
			{
				if (ItemPair.Key.IsValid())
				{
					// Calculate distance squared
					AActor* Item = ItemPair.Key.Get();
					ItemPair.Value = FVector::DistSquared(
						Data.PawnLocation,
						Item->GetActorLocation()
					);
				}
			}
			
			// Sort by distance (closest first)
			Data.ItemsWithDistances.Sort([](const TPair<TWeakObjectPtr<AActor>, float>& A, 
											 const TPair<TWeakObjectPtr<AActor>, float>& B)
			{
				return A.Value < B.Value; // Ascending order (closest = lowest distance)
			});
		}
		
		// ========================================================================
		// STEP 3: Switch back to GAME THREAD to apply results
		// ========================================================================
		
		AsyncTask(ENamedThreads::GameThread, [this, AIDataBatch]()
		{
			// 🎮 BACK ON GAME THREAD - Safe to modify UObjects!
			
			UE_LOG(LogInteractableSubsystem, VeryVerbose, TEXT("✅ AI priority calculations complete, applying results"));
			
			// Apply calculated priorities to each AI
			OnAIPriorityCalculationsComplete(AIDataBatch);
		});
	});
	
	// Function returns IMMEDIATELY - no blocking! ✅
}


// Runs on GAME THREAD after background calculations finish
void UInteractionSubsystem::OnAIPriorityCalculationsComplete(TArray<FAIPriorityData> ProcessedData)
{
	
	for (const FAIPriorityData& Data : ProcessedData)
	{
		APawn* Pawn = Data.Pawn.Get();
		if (!Pawn)
			continue;
		
		// Apply the new sorted priority order
		ApplyAIItemLoadState(Pawn, Data.ItemsWithDistances);
	}
}

void UInteractionSubsystem::ApplyAIItemLoadState(
	APawn* Pawn, 
	const TArray<TPair<TWeakObjectPtr<AActor>, float>>& SortedItems)
{
	if (!Pawn)
		return;
	
	// Get or create the item list for this pawn
	TArray<FNearbyItemInfo>* ItemListPtr = NearbyItemsbyPawn.Find(Pawn);
	if (!ItemListPtr)
		return;
	
	TArray<FNearbyItemInfo>& ItemList = *ItemListPtr;
	
	// Update the item list with new distances
	for (int32 i = 0; i < ItemList.Num(); ++i)
	{
		if (!ItemList[i].Item.IsValid())
			continue;
		
		// Find matching item in sorted results
		for (const auto& SortedPair : SortedItems)
		{
			if (SortedPair.Key == ItemList[i].Item)
			{
				ItemList[i].DistanceSquared = SortedPair.Value;
				break;
			}
		}
	}
	
	// Re-sort based on updated distances
	ItemList.Sort([](const FNearbyItemInfo& A, const FNearbyItemInfo& B) 
	{
		return A.DistanceSquared < B.DistanceSquared;
	});
	
	// ========================================================================
	// UNLOAD items that fell out of top N
	// ========================================================================
	
	for (int32 i = MaxSimultaneouslyLoadedItems; i < ItemList.Num(); ++i)
	{
		if (ItemList[i].bIsLoaded && ItemList[i].Item.IsValid())
		{
			IInteractableInterface::Execute_UnloadAssets(ItemList[i].Item.Get());
			ItemList[i].bIsLoaded = false;
			
			UE_LOG(LogInteractableSubsystem, VeryVerbose, TEXT("🔽 Unloaded: %s (fell below top %d)"), 
				   *ItemList[i].Item->GetName(), 
				   MaxSimultaneouslyLoadedItems);
		}
	}
	
	// ========================================================================
	// LOAD items that entered top N
	// ========================================================================
	
	for (int32 i = 0; i < FMath::Min(ItemList.Num(), MaxSimultaneouslyLoadedItems); ++i)
	{
		if (!ItemList[i].bIsLoaded && ItemList[i].Item.IsValid())
		{
			IInteractableInterface::Execute_LoadAssets(ItemList[i].Item.Get());
			ItemList[i].bIsLoaded = true;
			
			UE_LOG(LogInteractableSubsystem, VeryVerbose, TEXT("🔼 Loaded: %s (entered top %d)"), 
				   *ItemList[i].Item->GetName(), 
				   MaxSimultaneouslyLoadedItems);
		}
	}
}



// ================================================================
// ASYNC TRACE SYSTEM - NEW IMPLEMENTATION
// ================================================================

void UInteractionSubsystem::RequestAsyncFocusUpdate(APawn* Pawn)
{
	if (!Pawn || !bUseAsyncTraces)
	{
		// Fallback to sync if async disabled
		if (Pawn && !bUseAsyncTraces)
		{
			UpdatePlayerFocus(Pawn, PlayerTraceCapsuleRadius, PlayerTraceCapsuleHalfHeight, GlobalFullInteractionDistanceUI);
		}
		return;
	}
	
	// Don't start new trace if one is already running for this pawn
	for (const FActivePlayerTrace& ActiveTrace : ActivePlayerTraces)
	{
		if (ActiveTrace.Pawn == Pawn)
		{
			return; // Trace already in progress
		}
	}
	
	LaunchAsyncPlayerFocusTrace(Pawn, PlayerTraceCapsuleRadius, PlayerTraceCapsuleHalfHeight, GlobalFullInteractionDistanceUI);
}

void UInteractionSubsystem::UpdatePlayerFocusAsync(APawn* Pawn, float SweepRadius, float SweepHalfHeight, float SweepDistance)
{
	if (!Pawn)
		return;
	
	if (!bUseAsyncTraces)
	{
		// Fallback to sync version
		UpdatePlayerFocus(Pawn, SweepRadius, SweepHalfHeight, SweepDistance);
		return;
	}
	
	// Don't start new trace if one is already running for this pawn
	for (const FActivePlayerTrace& ActiveTrace : ActivePlayerTraces)
	{
		if (ActiveTrace.Pawn == Pawn)
		{
			return; // Trace already in progress
		}
	}
	
	LaunchAsyncPlayerFocusTrace(Pawn, SweepRadius, SweepHalfHeight, SweepDistance);
}

void UInteractionSubsystem::LaunchAsyncPlayerFocusTrace(APawn* Pawn, float SweepRadius, float SweepHalfHeight, float SweepDistance)
{
	if (!Pawn)
		return;
	
	UWorld* World = Pawn->GetWorld();
	if (!World)
		return;
	
	// Get pawn view location and direction
	FVector Start, Forward;
	GetPawnViewPoint(Pawn, Start, Forward);
	FVector End = Start + (Forward * SweepDistance);
	
	// Setup collision parameters
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AsyncPlayerFocusTrace), false);
	Params.AddIgnoredActor(Pawn);
	
	// Setup trace shape (capsule)
	FCollisionShape Shape = FCollisionShape::MakeCapsule(SweepRadius, SweepHalfHeight);
	FQuat Rotation = Forward.ToOrientationQuat();
	
	// Setup async callback
	FTraceDelegate Callback;
	Callback.BindUObject(this, &UInteractionSubsystem::OnPlayerFocusTraceComplete, TWeakObjectPtr<APawn>(Pawn));
	
	// Launch async trace (runs on physics thread!)
	FTraceHandle Handle = World->AsyncSweepByChannel(
		EAsyncTraceType::Multi,
		Start,
		End,
		Rotation,
		ECC_Visibility,
		Shape,
		Params,
		FCollisionResponseParams::DefaultResponseParam,
		&Callback
	);
	
	// Track active trace
	FActivePlayerTrace NewTrace(Pawn, Handle, SweepRadius, SweepHalfHeight, SweepDistance);
	NewTrace.StartTime = World->GetTimeSeconds();
	ActivePlayerTraces.Add(NewTrace);
	
	UE_LOG(LogInteractableSubsystem, VeryVerbose, TEXT("⚡ Launched async focus trace for %s"), *Pawn->GetName());
}

void UInteractionSubsystem::OnPlayerFocusTraceComplete(
	const FTraceHandle& Handle,
	FTraceDatum& Data,
	TWeakObjectPtr<APawn> Pawn)
{
	// Back on game thread now
	
	if (!Pawn.IsValid())
	{
		// Pawn was destroyed, remove from active traces
		ActivePlayerTraces.RemoveAll([&Handle](const FActivePlayerTrace& Trace) {
			return Trace.Handle == Handle;
		});
		return;
	}
	
	// Remove from active traces
	ActivePlayerTraces.RemoveAll([&Handle](const FActivePlayerTrace& Trace) {
		return Trace.Handle == Handle;
	});
	
	// Process results and find best interactable
	AActor* BestInteractable = ProcessAsyncTraceResults(Data.OutHits, Pawn.Get());
	
	// Update focused actor
	UpdateFocus(Pawn.Get(), BestInteractable);
	
	UE_LOG(LogInteractableSubsystem, VeryVerbose, TEXT("✅ Async trace completed for %s, found: %s"), 
		*Pawn->GetName(), 
		BestInteractable ? *BestInteractable->GetName() : TEXT("nullptr"));
}

AActor* UInteractionSubsystem::ProcessAsyncTraceResults(const TArray<FHitResult>& Hits, APawn* Pawn)
{
	if (!Pawn || Hits.Num() == 0)
		return nullptr;
	
	AActor* BestActor = nullptr;
	float BestDistance = GlobalFullInteractionDistanceUI + 1.f;
	int32 BestPriority = TNumericLimits<int32>::Min();
	
	FVector PawnLocation, PawnForward;
	GetPawnViewPoint(Pawn, PawnLocation, PawnForward);
	
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
			continue;
		
		// Check if actor is interactable
		if (!HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
			continue;
		
		if (!IInteractableInterface::Execute_GetInteractionEnabled(HitActor))
			continue;
		
		if (!IInteractableInterface::Execute_IsCurrentlyInteractable(HitActor))
			continue;
		
		// Calculate angle from view center
		FVector ToActor = (HitActor->GetActorLocation() - PawnLocation).GetSafeNormal();
		float DotProduct = FVector::DotProduct(PawnForward, ToActor);
		float Angle = FMath::Acos(DotProduct);
		float AngleDegrees = FMath::RadiansToDegrees(Angle);
		
		// Check if within interaction angle
		if (AngleDegrees > MaxInteractionAngle)
			continue;
		
		// Get priority
		int32 Priority = GetGameplayTagPriority(HitActor, true);
		
		// Check distance
		float Distance = Hit.Distance;
		
		// Prioritize by: 1) Higher priority, 2) Closer distance
		if (Priority > BestPriority || (Priority == BestPriority && Distance < BestDistance))
		{
			BestPriority = Priority;
			BestDistance = Distance;
			BestActor = HitActor;
		}
	}
	
	return BestActor;
}

void UInteractionSubsystem::CleanupStaleTraces()
{
	if (ActivePlayerTraces.Num() == 0)
		return;
	
	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
		return;
	
	float CurrentTime = World->GetTimeSeconds();
	
	// Cleanup player traces that took too long
	int32 RemovedCount = ActivePlayerTraces.RemoveAll([this, CurrentTime](const FActivePlayerTrace& Trace) {
		if (CurrentTime - Trace.StartTime > TraceTimeoutDuration)
		{
			UE_LOG(LogInteractableSubsystem, Warning, TEXT("⚠️ Cleaning up stale player trace for %s"),
				Trace.Pawn.IsValid() ? *Trace.Pawn->GetName() : TEXT("InvalidPawn"));
			return true; // Remove
		}
		return false; // Keep
	});
	
	if (RemovedCount > 0)
	{
		UE_LOG(LogInteractableSubsystem, Warning, TEXT("Cleaned up %d stale traces"), RemovedCount);
	}
}

// ================================================================
// SERVER-SIDE VALIDATION FOR MULTIPLAYER
// ================================================================

void UInteractionSubsystem::RequestServerValidation(APawn* ClientPawn, AActor* TargetActor)
{
	if (!ClientPawn || !TargetActor)
		return;
	
	UWorld* World = GetGameInstance()->GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogInteractableSubsystem, Warning, TEXT("RequestServerValidation called on client - ignoring"));
		return;
	}
	
	// Add to pending validations
	FPendingValidation Validation;
	Validation.ClientPawn = ClientPawn;
	Validation.TargetActor = TargetActor;
	
	// Get view point (need temp variable for forward vector)
	FVector ClientForward;
	GetPawnViewPoint(ClientPawn, Validation.ClientViewLocation, ClientForward);
	Validation.ClientViewRotation = ClientForward.Rotation();
	Validation.RequestTime = World->GetTimeSeconds();
	
	PendingValidations.Add(Validation);
	
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🔐 Server validation requested: %s -> %s"), 
		*ClientPawn->GetName(), *TargetActor->GetName());
}

void UInteractionSubsystem::LaunchBatchValidation()
{
	UWorld* World = GetGameInstance()->GetWorld();
	if (!World || PendingValidations.Num() == 0)
		return;
	
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🚀 Launching %d validation traces"), PendingValidations.Num());
	
	// Launch async trace for each pending validation
	for (const FPendingValidation& Validation : PendingValidations)
	{
		if (!Validation.ClientPawn.IsValid() || !Validation.TargetActor.IsValid())
			continue;
		
		APawn* Pawn = Validation.ClientPawn.Get();
		AActor* Target = Validation.TargetActor.Get();
		
		FVector Start = Validation.ClientViewLocation;
		FVector Forward = Validation.ClientViewRotation.Vector();
		FVector End = Start + (Forward * GlobalFullInteractionDistanceUI);
		
		// Setup collision parameters
		FCollisionQueryParams Params(SCENE_QUERY_STAT(ServerValidationTrace), false);
		Params.AddIgnoredActor(Pawn);
		
		// Setup trace shape
		FCollisionShape Shape = FCollisionShape::MakeCapsule(PlayerTraceCapsuleRadius, PlayerTraceCapsuleHalfHeight);
		FQuat Rotation = Forward.ToOrientationQuat();
		
		// Setup callback
		FTraceDelegate Callback;
		Callback.BindUObject(this, &UInteractionSubsystem::OnValidationTraceComplete,
			TWeakObjectPtr<APawn>(Pawn), TWeakObjectPtr<AActor>(Target));
		
		// Launch async trace
		FTraceHandle Handle = World->AsyncSweepByChannel(
			EAsyncTraceType::Multi,
			Start,
			End,
			Rotation,
			ECC_Visibility,
			Shape,
			Params,
			FCollisionResponseParams::DefaultResponseParam,
			&Callback
		);
		
		// Track validation
		FBatchValidationTrace ValidationTrace;
		ValidationTrace.Handle = Handle;
		ValidationTrace.Pawn = Pawn;
		ValidationTrace.TargetActor = Target;
		ActiveValidations.Add(ValidationTrace);
	}
	
	// Clear pending validations (they're now active)
	PendingValidations.Empty();
}

void UInteractionSubsystem::OnValidationTraceComplete(
	const FTraceHandle& Handle,
	FTraceDatum& Data,
	TWeakObjectPtr<APawn> Pawn,
	TWeakObjectPtr<AActor> TargetActor)
{
	// Back on game thread
	
	// Remove from active validations
	ActiveValidations.RemoveAll([&Handle](const FBatchValidationTrace& Trace) {
		return Trace.Handle == Handle;
	});
	
	if (!Pawn.IsValid() || !TargetActor.IsValid())
	{
		UE_LOG(LogInteractableSubsystem, Warning, TEXT("❌ Validation trace completed but pawn or target invalid"));
		return;
	}
	
	// Validate interaction
	bool bIsValid = ValidateInteraction(Data.OutHits, Pawn.Get(), TargetActor.Get());
	
	if (bIsValid)
	{
		// Interaction is valid, execute on server
		UE_LOG(LogInteractableSubsystem, Log, TEXT("✅ Server validation PASSED: %s -> %s"), 
			*Pawn->GetName(), *TargetActor->GetName());
		
		if (TargetActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
			if (PC)
			{
				IInteractableInterface::Execute_OnInteract(TargetActor.Get(), PC);
			}
		}
	}
	else
	{
		// Interaction failed validation
		UE_LOG(LogInteractableSubsystem, Warning, TEXT("❌ Server validation FAILED: %s -> %s (out of range or no LOS)"),
			*Pawn->GetName(), *TargetActor->GetName());
		
		// TODO: Notify client of failed interaction if desired
	}
}

bool UInteractionSubsystem::ValidateInteraction(const TArray<FHitResult>& Hits, APawn* Pawn, AActor* TargetActor)
{
	if (!Pawn || !TargetActor)
		return false;
	
	// Check if target was hit in the trace
	for (const FHitResult& Hit : Hits)
	{
		if (Hit.GetActor() == TargetActor)
		{
			// Verify distance is within range
			if (Hit.Distance <= GlobalFullInteractionDistanceUI)
			{
				// Verify angle is within acceptable range
				FVector PawnLocation, PawnForward;
				GetPawnViewPoint(Pawn, PawnLocation, PawnForward);
				
				FVector ToTarget = (TargetActor->GetActorLocation() - PawnLocation).GetSafeNormal();
				float DotProduct = FVector::DotProduct(PawnForward, ToTarget);
				float Angle = FMath::Acos(DotProduct);
				float AngleDegrees = FMath::RadiansToDegrees(Angle);
				
				if (AngleDegrees <= MaxInteractionAngle)
				{
					return true; // Valid interaction!
				}
			}
		}
	}
	
	return false; // Target not found in trace or out of range/angle
}

// ================================================================
// AI STAGGERED UPDATES
// ================================================================

void UInteractionSubsystem::RegisterAIPawn(APawn* AIPawn)
{
	if (!AIPawn)
		return;
	
	TWeakObjectPtr<APawn> PawnPtr(AIPawn);
	if (!RegisteredAIPawns.Contains(PawnPtr))
	{
		RegisteredAIPawns.Add(PawnPtr);
		UE_LOG(LogInteractableSubsystem, Log, TEXT("🤖 Registered AI pawn for staggered updates: %s"), *AIPawn->GetName());
	}
	// Start timer if this is first AI
	if (RegisteredAIPawns.Num() == 1)
	{
		StartAIUpdateTimer();
	}

}

void UInteractionSubsystem::UnregisterAIPawn(APawn* AIPawn)
{
	if (!AIPawn)
		return;
	
	TWeakObjectPtr<APawn> PawnPtr(AIPawn);
	RegisteredAIPawns.Remove(PawnPtr);
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🤖 Unregistered AI pawn: %s"), *AIPawn->GetName());
}

void UInteractionSubsystem::TickStaggeredAITraces(float DeltaTime)
{
	if (RegisteredAIPawns.Num() == 0 || !bUseAsyncTraces)
		return;
	
	// Update N AI pawns per frame in round-robin fashion
	int32 TracesThisFrame = FMath::Min(AITracesPerFrame, RegisteredAIPawns.Num());
	
	for (int32 i = 0; i < TracesThisFrame; ++i)
	{
		// Wrap around to start if we reach the end
		if (CurrentAIUpdateIndex >= RegisteredAIPawns.Num())
		{
			CurrentAIUpdateIndex = 0;
		}
		
		// Clean up invalid pawns
		if (!RegisteredAIPawns.IsValidIndex(CurrentAIUpdateIndex) || !RegisteredAIPawns[CurrentAIUpdateIndex].IsValid())
		{
			RegisteredAIPawns.RemoveAtSwap(CurrentAIUpdateIndex);
			continue;
		}
		
		// Update this AI pawn
		APawn* AIPawn = RegisteredAIPawns[CurrentAIUpdateIndex].Get();
		RequestAsyncFocusUpdate(AIPawn);
		
		// Move to next AI
		CurrentAIUpdateIndex++;
	}
}

void UInteractionSubsystem::GetPawnViewPoint(APawn* Pawn, FVector& OutLocation, FVector& OutForward) const
{
	if (!Pawn)
		return;
	
	// Try to get player controller view point
	if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
	{
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(OutLocation, ViewRotation);
		OutForward = ViewRotation.Vector();
	}
	else
	{
		// Fallback for AI pawns
		OutLocation = Pawn->GetPawnViewLocation();
		OutForward = Pawn->GetActorForwardVector();
	}
}

// ================================================================
// LEGACY SYNC TRACE FUNCTIONS (KEPT FOR COMPATIBILITY)
// ================================================================

/*How the sorting works:
 *Gets the minimum interaction range
 *Sort based on priority using gameplay tags
 *Does Optional LOS check and do the first two steps
 */

AActor* UInteractionSubsystem::GetBestInteractableForPlayer(APawn* Pawn) const
{
    if (!Pawn)
        return nullptr;

    // ========================================================================
    // CHECK IF THIS IS AI - Redirect to AI function
    // ========================================================================
    
    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC) // No PlayerController = AI
    {
        // Redirect to dedicated AI function
        return GetBestInteractableForAI(Pawn);
    }
	
	// ✅ NEW: Network check - only run for locally controlled players
	if (!PC->IsLocalController())
	{
		UE_LOG(LogInteraction, VeryVerbose, 
			TEXT("GetBestInteractableForPlayer: Skipping non-local controller %s"), 
			*Pawn->GetName());
		return nullptr;
	}

    // ========================================================================
    // PLAYER-ONLY CODE BELOW
    // ========================================================================

    TArray<AActor*> Candidates;

    // ========================================================================
    // SPATIAL HASH OPTIMIZATION - ACTIVATED FOR TESTING
    // ========================================================================
    // Using spatial hash instead of InteractorComponent sphere for performance testing
    Candidates = GetNearbyInteractables(Pawn->GetActorLocation(), GlobalFullInteractionDistanceUI);
    // This is 10-40x faster in dense areas (200+ items)
    // ========================================================================

    // Get candidates from InteractorComponent's interaction sphere (DISABLED FOR TESTING)
    /*
    if (Pawn->GetClass()->ImplementsInterface(UInteractorInterface::StaticClass()))
    {
        UActorComponent* InteractorComponentAsActorComponent = 
            IInteractorInterface::Execute_GetInteractorComponentAsActorComponent(Pawn, UInteractorComponent::StaticClass());
        if (InteractorComponentAsActorComponent)
        {
            if (UInteractorComponent* InteractorComponent = Cast<UInteractorComponent>(InteractorComponentAsActorComponent))
            {
                Candidates = InteractorComponent->GetInteractionUIOverlappingActors();
            }
        }
    }
    */

    if (Candidates.Num() == 0)
    {
        UE_LOG(LogInventoryInteractableSystem, Log, TEXT("No candidates in interaction sphere"));
        return nullptr;
    }

    // ✅ Filter: Must be valid interactable
    Candidates = Candidates.FilterByPredicate([](AActor* Actor)
    {
        if (!Actor || !Actor->Implements<UInteractableInterface>())
            return false;
            
        if (!IInteractableInterface::Execute_GetInteractionEnabled(Actor))
            return false;
            
        if (!IInteractableInterface::Execute_IsCurrentlyInteractable(Actor))
            return false;
            
        return true;
    });

    if (Candidates.Num() == 0)
    {
        UE_LOG(LogInventoryInteractableSystem, Log, TEXT("No valid interactables in interaction sphere"));
        return nullptr;
    }

    // ========================================================================
    // Setup Capsule Trace Parameters
    // ========================================================================

    FVector PawnView = Pawn->GetPawnViewLocation();
    FVector Forward = Pawn->GetControlRotation().Vector();
    float MaxInteractionDistance = InteractionDefaultsConstants::InteractionDistanceUI;
    FVector CapsuleEnd = PawnView + Forward * MaxInteractionDistance;

    // Capsule trace parameters
    float CapsuleRadius = InteractionDefaultsConstants::DefaultDebugDrawRadius;
    float CapsuleHalfHeight = MaxInteractionDistance;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Pawn);
    // CRITICAL: Set to trace complex collision to catch all objects
    Params.bTraceComplex = false;  // Use simple collision for performance
    Params.bReturnPhysicalMaterial = false;

    TArray<FHitResult> HitResults;
    
    // IMPORTANT: Use ObjectType trace instead of Channel trace to get ALL objects
    // This ensures we detect interactable objects even if they don't block Visibility
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    
    bool bHit = Pawn->GetWorld()->SweepMultiByObjectType(
        HitResults,
        PawnView,
        CapsuleEnd,
        Pawn->GetControlRotation().Quaternion(),
        ObjectParams,
        FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
        Params
    );

    // Draw debug capsule trace
	UDebugSubsystem::DrawDebugCapsuleForLayer(
		Pawn, 
		FWWTagLibrary::Debug_Interaction(), 
		PawnView,
		CapsuleEnd,
		CapsuleHalfHeight,
		CapsuleRadius,
		Pawn->GetControlRotation(),
		bHit,
		HitResults
	);

    UE_LOG(LogInteraction, Log, 
           TEXT("Capsule trace: %d hits from camera"), 
           HitResults.Num());

    // Rest of GetBestInteractableForPlayer implementation...
    // [TRUNCATED FOR BREVITY - Keep your existing implementation]
    
    // MISSING IMPLEMENTATION FOR GetBestInteractableForPlayer
// Add this after line 949 in InteractionSubsystem.cpp
// Replace the "return nullptr;" at line 954 with this code:

    // ========================================================================
    // Process Hit Results - Find actors that were both hit by trace AND in candidates list
    // ========================================================================
    
    // Find valid candidates that were hit by the trace
    // CRITICAL: Check ALL hits, not just first - trace may hit environment first!
    TArray<AActor*> HitCandidates;
    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        
        // 🔍 DEBUG: Log what we actually hit
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("🔍 Trace hit: Actor=%s, Component=%s, ComponentOwner=%s"), 
            HitActor ? *HitActor->GetName() : TEXT("NULL"),
            Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("NULL"),
            (Hit.GetComponent() && Hit.GetComponent()->GetOwner()) ? *Hit.GetComponent()->GetOwner()->GetName() : TEXT("NULL"));
        
        // Check if the hit actor is directly in candidates
        if (HitActor && Candidates.Contains(HitActor))
        {
            if (!HitCandidates.Contains(HitActor))  // Avoid duplicates
            {
                HitCandidates.Add(HitActor);
                
                UE_LOG(LogInventoryInteractableSystem, Log, 
                    TEXT("✅ Valid hit candidate (direct): %s (Distance: %.2f)"), 
                    *HitActor->GetName(), 
                    Hit.Distance);
            }
        }
        // Also check if the component's owner is in candidates
        else if (Hit.GetComponent() && Hit.GetComponent()->GetOwner())
        {
            AActor* ComponentOwner = Hit.GetComponent()->GetOwner();
            if (Candidates.Contains(ComponentOwner) && !HitCandidates.Contains(ComponentOwner))
            {
                HitCandidates.Add(ComponentOwner);
                
                UE_LOG(LogInventoryInteractableSystem, Log, 
                    TEXT("✅ Valid hit candidate (via owner): %s (Distance: %.2f)"), 
                    *ComponentOwner->GetName(), 
                    Hit.Distance);
            }
            else
            {
                UE_LOG(LogInventoryInteractableSystem, Verbose, 
                    TEXT("⏭️ Skipping non-candidate: %s"), 
                    HitActor ? *HitActor->GetName() : TEXT("NULL"));
            }
        }
        else
        {
            UE_LOG(LogInventoryInteractableSystem, Verbose, 
                TEXT("⏭️ Skipping non-candidate: %s"), 
                HitActor ? *HitActor->GetName() : TEXT("NULL"));
        }
    }

    if (HitCandidates.Num() == 0)
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("❌ Interaction attempt failed: No valid interactable candidate found."));
        return nullptr;
    }

    // ========================================================================
    // STEP 1: Find HIGHEST Priority among hit candidates
    // ========================================================================
    
    int32 HighestPriority = TNumericLimits<int32>::Min();
    TArray<AActor*> HighestPriorityCandidates;
    
    for (AActor* Actor : HitCandidates)
    {
        int32 Priority = GetGameplayTagPriority(Actor);
        if (Priority > HighestPriority)
        {
            HighestPriority = Priority;
            HighestPriorityCandidates.Empty();
            HighestPriorityCandidates.Add(Actor);
        }
        else if (Priority == HighestPriority)
        {
            HighestPriorityCandidates.Add(Actor);
        }
    }

    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("Found %d candidates with highest priority %d"), 
        HighestPriorityCandidates.Num(), HighestPriority);

    // ========================================================================
    // STEP 2: From Highest Priority, Pick CLOSEST to center of view
    // ========================================================================
    
    AActor* BestActor = nullptr;
    float BestAngle = MaxInteractionAngle; // Use configured max angle
    FVector ViewForward = Forward;

    for (AActor* Actor : HighestPriorityCandidates)
    {
        FVector ToActor = (Actor->GetActorLocation() - PawnView).GetSafeNormal();
        float DotProduct = FVector::DotProduct(ViewForward, ToActor);
        float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        if (AngleDegrees < BestAngle)
        {
            BestAngle = AngleDegrees;
            BestActor = Actor;
        }
    }

    if (BestActor)
    {
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("✅ Best interactable: %s (Priority: %d, Angle: %.2f°)"), 
            *BestActor->GetName(), 
            HighestPriority, 
            BestAngle);
    }
    else
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("❌ No actor within max interaction angle (%.2f°)"), 
            MaxInteractionAngle);
    }

    return BestActor;
}

// ============================================================================
// SUMMARY OF WHAT THIS DOES:
// ============================================================================
// 1. Takes hit results from capsule trace
// 2. Filters to only actors that are BOTH:
//    - In the trace hits
//    - In the candidates list (from InteractionUI sphere)
// 3. Finds actors with highest gameplay tag priority
// 4. Among those, picks the one closest to center of view
// 5. Returns the best actor (or nullptr if none qualify)

AActor* UInteractionSubsystem::GetBestInteractableForAI(APawn* AIPawn) const
{
    if (!AIPawn)
        return nullptr;

    TArray<AActor*> Candidates;

    // ========================================================================
    // SPATIAL HASH OPTIMIZATION - NEW (Optional Alternative)
    // ========================================================================
    // If you want to use spatial hash instead of InteractorComponent sphere:
    // Candidates = GetNearbyInteractables(AIPawn->GetActorLocation(), GlobalLoadingDistance_Items);
    // This is 10-40x faster in dense areas with 100+ AI and 200+ items
    // 
    // Current approach uses InteractorComponent sphere which is already optimized via overlap events
    // Spatial hash provides additional optimization layer for extreme density scenarios
    // ========================================================================

    // Get candidates from InteractorComponent's interaction sphere
    if (AIPawn->GetClass()->ImplementsInterface(UInteractorInterface::StaticClass()))
    {
        UActorComponent* InteractorComponentAsActorComponent = 
            IInteractorInterface::Execute_GetInteractorComponentAsActorComponent(AIPawn, UInteractorComponent::StaticClass());
        if (InteractorComponentAsActorComponent)
        {
            if (UInteractorComponent* InteractorComponent = Cast<UInteractorComponent>(InteractorComponentAsActorComponent))
            {
                // AI uses full interaction sphere (not just UI sphere)
                Candidates = InteractorComponent->GetOverlappingInteractables();
            }
        }
    }

    if (Candidates.Num() == 0)
    {
        UE_LOG(LogInventoryInteractableSystem, Verbose, TEXT("AI: No candidates in interaction sphere"));
        return nullptr;
    }

    // ✅ Filter: Must be valid interactable
    Candidates = Candidates.FilterByPredicate([](AActor* Actor)
    {
        if (!Actor || !Actor->Implements<UInteractableInterface>())
            return false;
            
        if (!IInteractableInterface::Execute_GetInteractionEnabled(Actor))
            return false;
            
        if (!IInteractableInterface::Execute_IsCurrentlyInteractable(Actor))
            return false;
            
        return true;
    });

    if (Candidates.Num() == 0)
    {
        UE_LOG(LogInventoryInteractableSystem, Verbose, TEXT("AI: No valid interactables"));
        return nullptr;
    }

    // ========================================================================
    // STEP 1: Find HIGHEST Priority
    // ========================================================================
    
    int32 HighestPriority = TNumericLimits<int32>::Min();
    TArray<AActor*> HighestPriorityCandidates;
    
    for (AActor* Actor : Candidates)
    {
        int32 Priority = this->GetGameplayTagPriority(Actor);
        if (Priority > HighestPriority)
        {
            HighestPriority = Priority;
            HighestPriorityCandidates.Empty();
            HighestPriorityCandidates.Add(Actor);
        }
        else if (Priority == HighestPriority)
        {
            HighestPriorityCandidates.Add(Actor);
        }
    }

    UE_LOG(LogInventoryInteractableSystem, Verbose, TEXT("AI: Found %d candidates with priority %d"), 
           HighestPriorityCandidates.Num(), HighestPriority);

    // ========================================================================
    // STEP 2: From Highest Priority, Pick CLOSEST (with optional LOS check)
    // ========================================================================
    
    AActor* BestActor = nullptr;
    float MinDistSq = FLT_MAX;
    FVector AIPawnLocation = AIPawn->GetActorLocation();

    for (AActor* Actor : HighestPriorityCandidates)
    {
        // ✅ OPTIONAL: Line-of-sight check (configurable)
        if (bEnableAILineOfSight)
        {
            FHitResult Hit;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(AIPawn);
            
            bool bBlocked = AIPawn->GetWorld()->LineTraceSingleByChannel(
                Hit, 
                AIPawnLocation, 
                Actor->GetActorLocation(), 
                ECC_Visibility, 
                Params
            );
            
            // Skip if blocked by something other than the target
            if (bBlocked && Hit.GetActor() != Actor)
            {
                UE_LOG(LogInventoryInteractableSystem, Verbose, 
                       TEXT("AI: %s rejected - blocked by %s"), 
                       *Actor->GetName(), 
                       Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("Unknown"));
                continue;
            }
        }
        
        // Calculate distance
        float DistSq = FVector::DistSquared(AIPawnLocation, Actor->GetActorLocation());
        if (DistSq < MinDistSq)
        {
            MinDistSq = DistSq;
            BestActor = Actor;
        }
    }

    if (BestActor)
    {
        UE_LOG(LogInventoryInteractableSystem, Verbose, 
               TEXT("AI: Best interactable: %s at %.2f units (LOS: %s)"), 
               *BestActor->GetName(), 
               FMath::Sqrt(MinDistSq),
               bEnableAILineOfSight ? TEXT("ON") : TEXT("OFF"));
    }
    else
    {
        UE_LOG(LogInventoryInteractableSystem, Verbose, 
               TEXT("AI: No suitable interactable found (LOS filtering may have rejected all)"));
    }

    return BestActor;
}

AActor* UInteractionSubsystem::GetBestInteractableForPlayerWithViewParams(
    APawn* Pawn,
    FVector ViewLocation,
    FRotator ViewRotation) const
{
    if (!Pawn)
        return nullptr;

    // ========================================================================
    // Get candidates from InteractorComponent
    // ========================================================================
    
    TArray<AActor*> Candidates;
    
    if (Pawn->GetClass()->ImplementsInterface(UInteractorInterface::StaticClass()))
    {
        UActorComponent* InteractorComponentAsActorComponent = 
            IInteractorInterface::Execute_GetInteractorComponentAsActorComponent(Pawn, UInteractorComponent::StaticClass());
        if (InteractorComponentAsActorComponent)
        {
            if (UInteractorComponent* InteractorComponent = Cast<UInteractorComponent>(InteractorComponentAsActorComponent))
            {
                Candidates = InteractorComponent->GetInteractionUIOverlappingActors();
            }
        }
    }

    if (Candidates.Num() == 0)
    {
        return nullptr;
    }

    // ========================================================================
    // Filter: Must be valid interactable
    // ========================================================================
    
    Candidates = Candidates.FilterByPredicate([](AActor* Actor)
    {
        if (!Actor || !Actor->Implements<UInteractableInterface>())
            return false;
            
        if (!IInteractableInterface::Execute_GetInteractionEnabled(Actor))
            return false;
            
        if (!IInteractableInterface::Execute_IsCurrentlyInteractable(Actor))
            return false;
            
        return true;
    });

    if (Candidates.Num() == 0)
    {
        return nullptr;
    }

    // ========================================================================
    // Setup Capsule Trace Parameters - USING PROVIDED VIEW PARAMS
    // ========================================================================

    FVector Forward = ViewRotation.Vector();
    float MaxInteractionDistance = InteractionDefaultsConstants::InteractionDistanceUI;
    FVector CapsuleEnd = ViewLocation + Forward * MaxInteractionDistance;

    float CapsuleRadius = InteractionDefaultsConstants::DefaultDebugDrawRadius;
    float CapsuleHalfHeight = MaxInteractionDistance;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Pawn);
    Params.bTraceComplex = false;
    Params.bReturnPhysicalMaterial = false;

    TArray<FHitResult> HitResults;
    
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    
    bool bHit = Pawn->GetWorld()->SweepMultiByObjectType(
        HitResults,
        ViewLocation,  // ✅ Using provided view location
        CapsuleEnd,
        ViewRotation.Quaternion(),  // ✅ Using provided view rotation
        ObjectParams,
        FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
        Params
    );

    UE_LOG(LogInteraction, Log, 
        TEXT("Capsule trace: %d hits from camera"), 
        HitResults.Num());

    // ========================================================================
    // Process Hit Results - Find actors in candidates list
    // ========================================================================
    
    TArray<TPair<AActor*, float>> ValidCandidates;

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        UPrimitiveComponent* HitComponent = Hit.GetComponent();
        
        // Check direct hit
        if (HitActor && Candidates.Contains(HitActor))
        {
            float Distance = FVector::Dist(ViewLocation, HitActor->GetActorLocation());
            ValidCandidates.Add(TPair<AActor*, float>(HitActor, Distance));
            
            UE_LOG(LogInventoryInteractableSystem, Log, 
                TEXT("✅ Valid hit candidate (direct): %s (Distance: %.2f)"),
                *HitActor->GetName(), Distance);
        }
        // Check component owner
        else if (HitComponent && HitComponent->GetOwner() && Candidates.Contains(HitComponent->GetOwner()))
        {
            AActor* ComponentOwner = HitComponent->GetOwner();
            float Distance = FVector::Dist(ViewLocation, ComponentOwner->GetActorLocation());
            ValidCandidates.Add(TPair<AActor*, float>(ComponentOwner, Distance));
            
            UE_LOG(LogInventoryInteractableSystem, Log, 
                TEXT("✅ Valid hit candidate (via component owner): %s (Distance: %.2f)"),
                *ComponentOwner->GetName(), Distance);
        }
    }

    if (ValidCandidates.Num() == 0)
    {
        return nullptr;
    }

    // ========================================================================
    // Sort by distance and return closest
    // ========================================================================
    
    ValidCandidates.Sort([](const TPair<AActor*, float>& A, const TPair<AActor*, float>& B)
    {
        return A.Value < B.Value;
    });

    AActor* BestActor = ValidCandidates[0].Key;
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("✅ Best interactable: %s"), 
        *BestActor->GetName());

    return BestActor;
}


bool UInteractionSubsystem::TryInteract(APlayerController* InstigatorController)
{
	UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("🎯 InteractionSubsystem::TryInteract called"));
    
	APawn* ControlledPawn = InstigatorController->GetPawn();
	if (!ControlledPawn || !ControlledPawn->GetClass()->ImplementsInterface(UInteractorInterface::StaticClass()))
		return false;

	// Check if pawn can interact
	if (!IInteractorInterface::Execute_GetbCanInteract(ControlledPawn))
	{
		UE_LOG(LogInventoryInteractableSystem, Log, TEXT("Pawn %s can't interact right now"), 
			   *ControlledPawn->GetName());
		return false;
	}

	// ✅ Get InteractorComponent and delegate to it
	UInteractorComponent* InteractorComp = ControlledPawn->FindComponentByClass<UInteractorComponent>();
	if (!InteractorComp)
	{
		UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ No InteractorComponent on pawn: %s"), 
			   *ControlledPawn->GetName());
		return false;
	}

	UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("TryInteract from InteractionSubsystem Triggered"));
    
	InteractorComp->TryInteract();
	UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("InteractionCompName:%s"), *InteractorComp->GetName());
	return true;
}

bool UInteractionSubsystem::TryInteract(APlayerController* InstigatorController, FVector ViewLocation, FRotator ViewRotation)
{
	UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("🎯 InteractionSubsystem::TryInteract called WITH VIEW PARAMS"));
	UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("   ViewLocation: %s, ViewRotation: %s"), 
		*ViewLocation.ToString(), *ViewRotation.ToString());
    
	APawn* ControlledPawn = InstigatorController->GetPawn();
	if (!ControlledPawn || !ControlledPawn->GetClass()->ImplementsInterface(UInteractorInterface::StaticClass()))
		return false;

	// Check if pawn can interact
	if (!IInteractorInterface::Execute_GetbCanInteract(ControlledPawn))
	{
		UE_LOG(LogInventoryInteractableSystem, Log, TEXT("Pawn %s can't interact right now"), 
			   *ControlledPawn->GetName());
		return false;
	}

	// ✅ Get InteractorComponent
	UInteractorComponent* InteractorComp = ControlledPawn->FindComponentByClass<UInteractorComponent>();
	if (!InteractorComp)
	{
		UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ No InteractorComponent on pawn: %s"), 
			   *ControlledPawn->GetName());
		return false;
	}

	// ========================================================================
	// Instead of delegating to InteractorComp->TryInteract() which would re-trace,
	// we directly get candidates and trace using the PROVIDED view parameters
	// ========================================================================

	TArray<AActor*> Candidates = InteractorComp->GetInteractionUIOverlappingActors();
	
	if (Candidates.Num() == 0)
	{
		UE_LOG(LogInventoryInteractableSystem, Log, TEXT("No candidates in interaction sphere"));
		return false;
	}

	// ✅ Filter: Must be valid interactable
	Candidates = Candidates.FilterByPredicate([](AActor* Actor)
	{
		if (!Actor || !Actor->Implements<UInteractableInterface>())
			return false;
			
		if (!IInteractableInterface::Execute_GetInteractionEnabled(Actor))
			return false;
			
		if (!IInteractableInterface::Execute_IsCurrentlyInteractable(Actor))
			return false;
			
		return true;
	});

	if (Candidates.Num() == 0)
	{
		UE_LOG(LogInventoryInteractableSystem, Log, TEXT("No valid interactables in interaction sphere"));
		return false;
	}

	// ========================================================================
	// Setup Capsule Trace Parameters - USING PROVIDED VIEW PARAMS!
	// ========================================================================

	FVector Forward = ViewRotation.Vector();
	float MaxInteractionDistance = InteractionDefaultsConstants::InteractionDistanceUI;
	FVector CapsuleEnd = ViewLocation + Forward * MaxInteractionDistance;

	float CapsuleRadius = InteractionDefaultsConstants::DefaultDebugDrawRadius;
	float CapsuleHalfHeight = MaxInteractionDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControlledPawn);
	Params.bTraceComplex = false;
	Params.bReturnPhysicalMaterial = false;

	TArray<FHitResult> HitResults;
	
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	
	bool bHit = ControlledPawn->GetWorld()->SweepMultiByObjectType(
		HitResults,
		ViewLocation,  // ✅ Using client's view location!
		CapsuleEnd,
		ViewRotation.Quaternion(),  // ✅ Using client's view rotation!
		ObjectParams,
		FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
		Params
	);

	UE_LOG(LogInteraction, Log, 
		TEXT("Capsule trace: %d hits from camera"), 
		HitResults.Num());

	// ========================================================================
	// Process Hit Results - Find actors that were both hit by trace AND in candidates list
	// ========================================================================
	
	TArray<TPair<AActor*, float>> ValidCandidates;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		UPrimitiveComponent* HitComponent = Hit.GetComponent();
		
		UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("🔍 Trace hit: Actor=%s, Component=%s, ComponentOwner=%s"),
			HitActor ? *HitActor->GetName() : TEXT("NULL"),
			HitComponent ? *HitComponent->GetName() : TEXT("NULL"),
			HitComponent ? (HitComponent->GetOwner() ? *HitComponent->GetOwner()->GetName() : TEXT("NULL")) : TEXT("NULL"));

		// Check if hit actor is in candidates
		if (HitActor && Candidates.Contains(HitActor))
		{
			float Distance = FVector::Dist(ViewLocation, HitActor->GetActorLocation());
			ValidCandidates.Add(TPair<AActor*, float>(HitActor, Distance));
			UE_LOG(LogInventoryInteractableSystem, Log, TEXT("✅ Valid hit candidate (direct): %s (Distance: %.2f)"),
				*HitActor->GetName(), Distance);
		}
		// Check if hit component's owner is in candidates
		else if (HitComponent && HitComponent->GetOwner() && Candidates.Contains(HitComponent->GetOwner()))
		{
			AActor* ComponentOwner = HitComponent->GetOwner();
			float Distance = FVector::Dist(ViewLocation, ComponentOwner->GetActorLocation());
			ValidCandidates.Add(TPair<AActor*, float>(ComponentOwner, Distance));
			UE_LOG(LogInventoryInteractableSystem, Log, TEXT("✅ Valid hit candidate (via component owner): %s (Distance: %.2f)"),
				*ComponentOwner->GetName(), Distance);
		}
	}

	if (ValidCandidates.Num() == 0)
	{
		UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("❌ Interaction attempt failed: No valid interactable candidate found."));
		return false;
	}

	// Sort by distance and get closest
	ValidCandidates.Sort([](const TPair<AActor*, float>& A, const TPair<AActor*, float>& B)
	{
		return A.Value < B.Value;
	});

	AActor* BestActor = ValidCandidates[0].Key;
	UE_LOG(LogInventoryInteractableSystem, Log, TEXT("✅ Best interactable: %s"), *BestActor->GetName());

	// Execute interaction via InteractorComponent
	InteractorComp->Server_Interact(BestActor);
	
	return true;
}

bool UInteractionSubsystem::PerformInteraction(AController* InstigatorController)
{
	if (!InstigatorController) return false;

	APawn* Pawn = InstigatorController->GetPawn();
	if (!Pawn) return false;

	AActor* Target = FocusedActors.Contains(Pawn) ? FocusedActors[Pawn].Get() : nullptr;
	if (!Target) return false;

	if (!Target->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		return false;

	IInteractableInterface::Execute_OnInteract(Target, InstigatorController);
	UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("Execute_OnInteract Executed for %s, %s"), 
		*Target->GetName(), *InstigatorController->GetName());
    
	return true;
}

void UInteractionSubsystem::UpdateFocus(APawn* Pawn, AActor* NewFocus)
{
    if (!Pawn) return;

    TWeakObjectPtr<AActor>& CurrentFocus = FocusedActors.FindOrAdd(Pawn);

    if (CurrentFocus.Get() == NewFocus) return;

    if (CurrentFocus.IsValid())
        OnInteractableUnfocused.Broadcast(CurrentFocus.Get());

    CurrentFocus = NewFocus;

    if (CurrentFocus.IsValid())
        OnInteractableFocused.Broadcast(CurrentFocus.Get());
}

void UInteractionSubsystem::UpdatePlayerFocus(APawn* Pawn, float SweepRadius, float SweepHalfHeight, float SweepDistance)
{
	if (!Pawn) return;

	FVector Start = Pawn->GetPawnViewLocation();
	FVector Forward = Pawn->GetControlRotation().Vector();
	FVector End = Start + Forward * SweepDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn);

	TArray<FHitResult> HitResults;

	bool bHit = Pawn->GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeCapsule(SweepRadius, SweepHalfHeight),
		Params
	);

	AActor* ClosestActor = nullptr;
	float MinDistanceSqr = TNumericLimits<float>::Max();

	if (bHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!HitActor) continue;
			if (!HitActor->Implements<UInteractableInterface>()) continue;
			if (!IInteractableInterface::Execute_IsCurrentlyInteractable(HitActor)) continue;

			float DistSqr = (HitActor->GetActorLocation() - Pawn->GetActorLocation()).SizeSquared();
			if (DistSqr < MinDistanceSqr)
			{
				MinDistanceSqr = DistSqr;
				ClosestActor = HitActor;
			}
		}
	}
	//Outline the closest Actor
	//TODO: implement a choice for the designer to use outlines
	if (ClosestActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		UActorComponent* OutlineComponent = IInteractableInterface::Execute_GetOutlineComponent(ClosestActor);
		if (OutlineComponent)
		{
			IInteractableInterface::Execute_SetOutlineActive(ClosestActor, true);
		}
	}
	UpdateFocus(Pawn, ClosestActor);
}

AActor* UInteractionSubsystem::GetFocusedActorForPawn(APawn* Pawn) const
{
	if (!Pawn) return nullptr;

	const TWeakObjectPtr<AActor>* Found = FocusedActors.Find(Pawn);
	return Found ? Found->Get() : nullptr;
}

int32 UInteractionSubsystem::GetGameplayTagPriority(AActor* Actor, bool bGetMax) const
{
	if (!Actor || !InteractableTagPriorityTable) return 0;

	int32 BestPriority = 0;

	// Get all gameplay tags from the actor
	FGameplayTagContainer Tags;
	if (Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		Tags = IInteractableInterface::Execute_GetGameplayTags(Actor);
	}
	
	//Iterate tags and find max priority from DataTable
	for (const FGameplayTag& Tag : Tags)
	{
		static const FString Context = TEXT("GetGameplayTagContainer");
		FGameplayTagPriorityRow* Row = InteractableTagPriorityTable->FindRow<FGameplayTagPriorityRow>(Tag.GetTagName(), Context);
		if (Row)
		{
			BestPriority = bGetMax ?
			BestPriority= FMath::Max(BestPriority, Row->Priority) :
			FMath::Min(BestPriority, Row->Priority);
			
		}
	}
	return BestPriority;
}

void UInteractionSubsystem::SetGlobalInteractionNotifications(bool bEnabled)
{
	bEnableGlobalInteractionNotifications = bEnabled;
    
	// Update all registered interactables
	for (TWeakObjectPtr<AActor> WeakActor : Interactables)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			if (Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
			{
				IInteractableInterface::Execute_SetInteractionNotifications(Actor);
			}
		}
	}
    
	UE_LOG(LogInteractableSubsystem, Log, TEXT("Global interaction notifications %s"), 
		   bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

bool UInteractionSubsystem::IsInteractableItem(AActor* Actor)
{
	if (!Actor) return false;
    
	// Check if actor has InteractableComponent with Item tag
	if (UInteractableComponent* InteractableComp = Actor->FindComponentByClass<UInteractableComponent>())
	{
		FGameplayTag ItemTag = FGameplayTag::RequestGameplayTag(FName("Interactable.Item"), false);
		return InteractableComp->InteractableTypeTag.MatchesTag(ItemTag);
	}
    
	return false;
}

bool UInteractionSubsystem::IsInteractableActor(AActor* Actor)
{
	if (!Actor) return false;
    
	// Has InteractableComponent but NOT an Item
	if (UInteractableComponent* InteractableComp = Actor->FindComponentByClass<UInteractableComponent>())
	{
		FGameplayTag ItemTag = FGameplayTag::RequestGameplayTag(FName("Interactable.Item"), false);
		return !InteractableComp->InteractableTypeTag.MatchesTag(ItemTag);
	}
    
	return false;
}

void UInteractionSubsystem::StartAIUpdateTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogInteractableSubsystem, Error, TEXT("❌ Cannot start AI timer - No valid world!"));
		return;
	}
	
	// Stop existing timer if running
	if (AIUpdateTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(AIUpdateTimerHandle);
	}
	
	// Start new timer
	World->GetTimerManager().SetTimer(
		AIUpdateTimerHandle,
		[this]()
		{
			if (bUseAsyncAIProcessing)
			{
				// ✅ ASYNC VERSION (preferred)
				UpdateNearbyItemPrioritiesAsync();
			}
			else
			{
				// Fallback to sync version if async disabled
				UpdateNearbyItemPriorities(AIUpdateInterval);
				UE_LOG(LogInteractableSubsystem, Warning, TEXT("AIAsysnc disabled, fallingback to synced UpdateNearbyItemPriorities."));
			}
		},
		AIUpdateInterval,  // Configurable in editor (default 0.1s)
		true               // Loop
	);
	
	UE_LOG(LogInteractableSubsystem, Log, TEXT("✅ AI Priority Update Timer started (%.2fs interval, Async: %s)"), 
		   AIUpdateInterval,
		   bUseAsyncAIProcessing ? TEXT("ON") : TEXT("OFF"));
}

void UInteractionSubsystem::StopAIUpdateTimer()
{
	UWorld* World = GetWorld();
	if (!World)
		return;
	
	if (AIUpdateTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(AIUpdateTimerHandle);
		AIUpdateTimerHandle.Invalidate();
		
		UE_LOG(LogInteractableSubsystem, Log, TEXT("⏹️ AI Priority Update Timer stopped"));
	}
}

void UInteractionSubsystem::RestartAIUpdateTimer()
{
	StopAIUpdateTimer();
	StartAIUpdateTimer();
}


void UInteractionSubsystem::SetAIUpdateInterval(float NewInterval)
{
	AIUpdateInterval = FMath::Clamp(NewInterval, 0.05f, 1.0f);
	StopAIUpdateTimer();
	StartAIUpdateTimer();
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🔄 AI Update Interval: %.2fs"), AIUpdateInterval);
}

void UInteractionSubsystem::PauseAIUpdates()
{
	StopAIUpdateTimer();
	UE_LOG(LogInteractableSubsystem, Log, TEXT("⏸️ AI Updates paused"));
}

void UInteractionSubsystem::ResumeAIUpdates()
{
	StartAIUpdateTimer();
	UE_LOG(LogInteractableSubsystem, Log, TEXT("▶️ AI Updates resumed"));
}

// ========================================================================
// BATCHED TRACE IMPLEMENTATION
// ========================================================================

void UInteractionSubsystem::UpdateAllPlayersFocus_Batched()
{
	// Detect play mode and determine strategy
	EPlayMode PlayMode = DetectPlayMode();
    
	// Log for debugging (remove in shipping builds)
#if !UE_BUILD_SHIPPING
	static EPlayMode LastLoggedMode = EPlayMode::Unknown;
	if (PlayMode != LastLoggedMode)
	{
		FString ModeString;
		switch (PlayMode)
		{
		case EPlayMode::SinglePlayer: ModeString = "SinglePlayer"; break;
		case EPlayMode::SplitScreen: ModeString = "SplitScreen"; break;
		case EPlayMode::ListenServer: ModeString = "ListenServer"; break;
		case EPlayMode::DedicatedServer: ModeString = "DedicatedServer"; break;
		case EPlayMode::Client: ModeString = "Client"; break;
		default: ModeString = "Unknown"; break;
		}
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🎮 Play Mode: %s"), *ModeString);
		LastLoggedMode = PlayMode;
	}
#endif
    
	// Route to appropriate update method based on play mode
	switch (PlayMode)
	{
	case EPlayMode::SinglePlayer:
		UpdateSinglePlayer();
		break;
        
	case EPlayMode::SplitScreen:
		UpdateSplitScreen();
		break;
        
	case EPlayMode::ListenServer:
		UpdateListenServer();
		break;
        
	case EPlayMode::Client:
		UpdateClient();
		break;
        
	case EPlayMode::DedicatedServer:
		// Dedicated server doesn't update player focus
		// Only handles validation (already implemented)
		break;
        
	default:
		UE_LOG(LogInteractableSubsystem, Warning, TEXT("Unknown play mode - falling back to individual async traces"));
		UpdateFallback();
		break;
	}
}

void UInteractionSubsystem::LaunchBatchTraces()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	UE_LOG(LogInteractableSubsystem, Log, TEXT("🚀 Launching batch: %d traces"), PendingBatchTraces.Num());

	// Launch all traces with minimal delay between them
	for (FBatchTraceRequest& Request : PendingBatchTraces)
	{
		if (!Request.Pawn.IsValid())
			continue;
        
		// Setup collision params
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Request.Pawn.Get());
        
		// Create callback delegate
		FTraceDelegate Callback;
		Callback.BindUObject(this, &UInteractionSubsystem::OnBatchedTraceComplete, Request.RequestID);
        
		// Launch async trace
		FTraceHandle Handle = World->AsyncSweepByChannel(
			EAsyncTraceType::Multi,
			Request.Start,
			Request.End,
			Request.Rotation,
			ECC_Visibility,
			Request.Shape,
			QueryParams,
			FCollisionResponseParams::DefaultResponseParam,
			&Callback
		);
        
		// Store handle in request
		Request.Handle = Handle;
	}
	

	// All traces now running on physics thread in parallel! 🚀
}

void UInteractionSubsystem::OnBatchedTraceComplete(const FTraceHandle& Handle, FTraceDatum& Data, int32 RequestID)
{
	// Validate request ID
	if (!PendingBatchTraces.IsValidIndex(RequestID))
	{
		UE_LOG(LogInteractableSubsystem, Warning, TEXT("Invalid batch trace RequestID: %d"), RequestID);
		return;
	}
    
	FBatchTraceRequest& Request = PendingBatchTraces[RequestID];
    
	// Validate pawn still exists
	APawn* Pawn = Request.Pawn.Get();
	if (!Pawn || !Pawn->IsValidLowLevel())
	{
		CompletedBatchTraces++;
		if (CompletedBatchTraces >= TotalBatchTraces)
			OnBatchComplete();
		return;
	}
    
	// Process trace results using existing helper function
	AActor* BestInteractable = ProcessAsyncTraceResults(Data.OutHits, Pawn);
    
	// Update focus (this will trigger UI updates, outlines, etc.)
	UpdateFocus(Pawn, BestInteractable);
    
	// Track completion
	CompletedBatchTraces++;
    
	// Check if all traces in batch are complete
	if (CompletedBatchTraces >= TotalBatchTraces)
	{
		OnBatchComplete();
	}
	UE_LOG(LogInteractableSubsystem, Log, TEXT("Batch complete: %d/%d traces finished"), 
	CompletedBatchTraces, TotalBatchTraces);

}

void UInteractionSubsystem::OnBatchComplete()
{
	// Optional: Analytics, logging, or cleanup
	UE_LOG(LogInteractableSubsystem, Verbose, TEXT("Batch complete: %d/%d traces finished"), 
		CompletedBatchTraces, TotalBatchTraces);
    
	// Clear batch data to free memory
	PendingBatchTraces.Empty();
	CompletedBatchTraces = 0;
	TotalBatchTraces = 0;
    
	// Optional: You could trigger any post-batch events here
	// For example: notify UI system that all players have updated focus
}

// ========================================================================
// PLAY MODE DETECTION IMPLEMENTATION
// ========================================================================

EPlayMode UInteractionSubsystem::DetectPlayMode() const
{
    UWorld* World = GetWorld();
    if (!World)
        return EPlayMode::Unknown;
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
        return EPlayMode::Unknown;
    
    // Check network mode
    ENetMode NetMode = World->GetNetMode();
    
    // Get number of local players
    int32 NumLocalPlayers = GameInstance->GetNumLocalPlayers();
    
    // Dedicated Server - no local players
    if (NetMode == NM_DedicatedServer)
    {
        return EPlayMode::DedicatedServer;
    }
    
    // Client - connected to remote server
    if (NetMode == NM_Client)
    {
        return EPlayMode::Client;
    }
    
    // Listen Server - host + clients
    if (NetMode == NM_ListenServer)
    {
        return EPlayMode::ListenServer;
    }
    
    // Standalone (no network)
    if (NetMode == NM_Standalone)
    {
        if (NumLocalPlayers > 1)
        {
            return EPlayMode::SplitScreen; // 2-4 local players
        }
        else
        {
            return EPlayMode::SinglePlayer; // 1 local player
        }
    }
    
    return EPlayMode::Unknown;
}

bool UInteractionSubsystem::ShouldUseBatchedTraces() const
{
    // Must be enabled in settings
    if (!bUseAsyncTraces || !bUseBatchedTraces)
        return false;
    
    EPlayMode PlayMode = DetectPlayMode();
    
    switch (PlayMode)
    {
        case EPlayMode::SinglePlayer:
            // Don't batch - only 1 player, overhead not worth it
            return false;
        
        case EPlayMode::SplitScreen:
            // ALWAYS batch - multiple local players benefit greatly
            return true;
        
        case EPlayMode::ListenServer:
            // Batch LOCAL players only (host machine)
            // Remote clients handle their own traces
            return true;
        
        case EPlayMode::Client:
            // Client only handles 1 local player
            return false;
        
        case EPlayMode::DedicatedServer:
            // Server doesn't run player focus traces (only validation)
            return false;
        
        default:
            return false;
    }
}

void UInteractionSubsystem::GetLocalPlayerPawns(TArray<APawn*>& OutLocalPawns) const
{
    OutLocalPawns.Empty();
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
        return;
    
    // Iterate through all local players
    for (int32 i = 0; i < GameInstance->GetNumLocalPlayers(); i++)
    {
        ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(i);
        if (!LocalPlayer)
            continue;
        
        APlayerController* PC = LocalPlayer->GetPlayerController(World);
        if (!PC)
            continue;
        
        APawn* Pawn = PC->GetPawn();
        if (Pawn && RegisteredPlayerPawns.Contains(Pawn))
        {
            OutLocalPawns.Add(Pawn);
        }
    }
}

// ========================================================================
// MODE-SPECIFIC UPDATE IMPLEMENTATIONS
// ========================================================================

void UInteractionSubsystem::UpdateSinglePlayer()
{
    // Single player: Use individual async trace (no batching overhead)
    for (TWeakObjectPtr<APawn> WeakPawn : RegisteredPlayerPawns)
    {
        if (APawn* Pawn = WeakPawn.Get())
        {
            // Use async but not batched
            UpdatePlayerFocusAsync(Pawn, 30.f, 30.f, 500.f);
            break; // Only one player in single-player mode
        }
    }
}

void UInteractionSubsystem::UpdateSplitScreen()
{
    // Split-screen: Batch ALL local players (most efficient)
    if (!bUseAsyncTraces || !bUseBatchedTraces)
    {
        UpdateFallback();
        return;
    }
    
    // Get only local players
    TArray<APawn*> LocalPawns;
    GetLocalPlayerPawns(LocalPawns);
    
    if (LocalPawns.Num() == 0)
        return;
    
    // Clear previous batch data
    PendingBatchTraces.Empty();
    CompletedBatchTraces = 0;
    TotalBatchTraces = 0;
    
    // Collect trace data for all local players
    for (APawn* Pawn : LocalPawns)
    {
        if (!Pawn || !Pawn->IsValidLowLevel())
            continue;
        
        // Get pawn view point
        FVector Start;
        FVector Forward;
        GetPawnViewPoint(Pawn, Start, Forward);
        
        // Use default trace parameters
        float SweepRadius = 30.f;
        float SweepHalfHeight = 30.f;
        float SweepDistance = 500.f;
        
        FVector End = Start + (Forward * SweepDistance);
        
        // Create batch request
        FBatchTraceRequest Request;
        Request.Pawn = Pawn;
        Request.Start = Start;
        Request.End = End;
        Request.Rotation = FQuat::Identity;
        Request.Shape = FCollisionShape::MakeCapsule(SweepRadius, SweepHalfHeight);
        Request.RequestID = PendingBatchTraces.Num();
        Request.SweepRadius = SweepRadius;
        Request.SweepHalfHeight = SweepHalfHeight;
        Request.SweepDistance = SweepDistance;
        
        PendingBatchTraces.Add(Request);
    }
    
    // Launch the batch
    if (PendingBatchTraces.Num() > 0)
    {
        TotalBatchTraces = PendingBatchTraces.Num();
        LaunchBatchTraces();
        
	UE_LOG(LogInteractableSubsystem, Log, TEXT("[Split-Screen] Batched %d players"), TotalBatchTraces);
    }
}

void UInteractionSubsystem::UpdateListenServer()
{
    // Listen Server: Batch ONLY local players (host machine)
    // Remote clients handle their own traces separately
    
    if (!bUseAsyncTraces)
    {
        UpdateFallback();
        return;
    }
    
    // Get only local players on host machine
    TArray<APawn*> LocalPawns;
    GetLocalPlayerPawns(LocalPawns);
    
    // If we have multiple local players, batch them
	if (LocalPawns.Num() > 1 && bUseBatchedTraces)
    {
        // Clear previous batch data
        PendingBatchTraces.Empty();
        CompletedBatchTraces = 0;
        TotalBatchTraces = 0;
        
        // Collect trace data for local players only
        for (APawn* Pawn : LocalPawns)
        {
            if (!Pawn || !Pawn->IsValidLowLevel())
                continue;
            
            FVector Start;
            FVector Forward;
            GetPawnViewPoint(Pawn, Start, Forward);
            
            float SweepRadius = 30.f;
            float SweepHalfHeight = 30.f;
            float SweepDistance = 500.f;
            FVector End = Start + (Forward * SweepDistance);
            
            FBatchTraceRequest Request;
            Request.Pawn = Pawn;
            Request.Start = Start;
            Request.End = End;
            Request.Rotation = FQuat::Identity;
            Request.Shape = FCollisionShape::MakeCapsule(SweepRadius, SweepHalfHeight);
            Request.RequestID = PendingBatchTraces.Num();
            Request.SweepRadius = SweepRadius;
            Request.SweepHalfHeight = SweepHalfHeight;
            Request.SweepDistance = SweepDistance;
            
            PendingBatchTraces.Add(Request);
        }
        
        if (PendingBatchTraces.Num() > 0)
        {
            TotalBatchTraces = PendingBatchTraces.Num();
            LaunchBatchTraces();
            
            UE_LOG(LogInteractableSubsystem, Verbose, TEXT("[Listen Server] Batched %d local player traces (host)"), TotalBatchTraces);
        }
    }
    else if (LocalPawns.Num() == 1)
    {
        // Only 1 local player on host - use individual async
        UpdatePlayerFocusAsync(LocalPawns[0], 30.f, 30.f, 500.f);
    }
    
    // NOTE: Remote clients run their own UpdateClient() on their machines
    // Server doesn't update focus for remote clients - they do it themselves
}

void UInteractionSubsystem::UpdateClient()
{
    // Client: Only 1 local player, use individual async trace
    TArray<APawn*> LocalPawns;
    GetLocalPlayerPawns(LocalPawns);
    
    if (LocalPawns.Num() > 0)
    {
        // Client only has 1 local pawn
        UpdatePlayerFocusAsync(LocalPawns[0], 30.f, 30.f, 500.f);
    }
}

void UInteractionSubsystem::UpdateFallback()
{
    // Fallback: Individual async traces for all registered players
    for (TWeakObjectPtr<APawn> WeakPawn : RegisteredPlayerPawns)
    {
        if (APawn* Pawn = WeakPawn.Get())
        {
            UpdatePlayerFocusAsync(Pawn, 30.f, 30.f, 500.f);
        }
    }
}

// ========================================================================
// SPATIAL HASH GRID API IMPLEMENTATIONS - NEW
// ========================================================================

TArray<AActor*> UInteractionSubsystem::GetNearbyInteractables(const FVector& Location, float Radius) const
{
	// Use spatial hash if enabled, otherwise fallback to brute force
	if (bUseSpatialHashing && SpatialGrid)
	{
		// ⚡ FAST PATH: Use spatial hash (10-40x faster!)

		// Measure spatial hash performance
		double StartTime = FPlatformTime::Seconds();
		TArray<AActor*> Result = SpatialGrid->GetActorsInRadius(Location, Radius);
		double EndTime = FPlatformTime::Seconds();
		double QueryTimeMs = (EndTime - StartTime) * 1000.0;

		UE_LOG(LogTemp, Warning, TEXT("🔍 SPATIAL QUERY: %.6f ms | Found: %d actors | Radius: %.1f"), 
			QueryTimeMs, Result.Num(), Radius);

		return Result;
	}
	else
	{
		// ⚠️ SLOW PATH: Brute force check all interactables
		
		// Measure brute force performance
		double StartTime = FPlatformTime::Seconds();
		
		TArray<AActor*> NearbyActors;
		float RadiusSq = Radius * Radius;
		
		for (TWeakObjectPtr<AActor> WeakActor : Interactables)
		{
			if (AActor* Actor = WeakActor.Get())
			{
				float DistSq = FVector::DistSquared(Location, Actor->GetActorLocation());
				if (DistSq <= RadiusSq)
				{
					NearbyActors.Add(Actor);
				}
			}
		}
		
		double EndTime = FPlatformTime::Seconds();
		double QueryTimeMs = (EndTime - StartTime) * 1000.0;
		
		UE_LOG(LogTemp, Warning, TEXT("🐌 BRUTE FORCE QUERY: %.6f ms | Found: %d actors | Checked: %d total | Radius: %.1f"), 
			QueryTimeMs, NearbyActors.Num(), Interactables.Num(), Radius);
		
		return NearbyActors;
	}
}

void UInteractionSubsystem::NotifyActorMoved(AActor* Actor, const FVector& OldLocation)
{
	if (!bUseSpatialHashing || !SpatialGrid || !Actor)
		return;
	
	SpatialGrid->UpdateActorPosition(Actor, OldLocation);
	
	UE_LOG(LogInteractableSubsystem, VeryVerbose, 
		TEXT("🗺️ Updated actor position in spatial grid: %s"), *Actor->GetName());
}

void UInteractionSubsystem::DebugDrawSpatialGrid(const FVector& Center, int32 Radius)
{
	if (!SpatialGrid)
	{
		UE_LOG(LogInteractableSubsystem, Warning, TEXT("Spatial Grid not initialized! Enable bUseSpatialHashing"));
		return;
	}
	
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
		return;
	
	SpatialGrid->DebugDrawGrid(World, Center, Radius, 5.f);
	
	UE_LOG(LogInteractableSubsystem, Log, TEXT("🗺️ Drawing spatial grid at %s (Radius: %d cells)"), 
		*Center.ToString(), Radius);
}

void UInteractionSubsystem::PrintSpatialGridStats()
{
	if (!SpatialGrid)
	{
		UE_LOG(LogInteractableSubsystem, Warning, TEXT("Spatial Grid not initialized! Enable bUseSpatialHashing"));
		return;
	}
	
	SpatialGrid->PrintGridStats();
}