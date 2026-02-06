// WidgetPoolManager.cpp
// Widget Pooling System implementation

#include "Subsystems/WidgetPoolManager.h"
#include "Subsystems/AdvancedWidgetFramework/WidgetManagerBase.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

DECLARE_STATS_GROUP(TEXT("WidgetPoolManager"), STATGROUP_WidgetPoolManager, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("WidgetPoolManager::Tick"), STAT_WidgetPoolManagerTick, STATGROUP_WidgetPoolManager);

TStatId UWidgetPoolManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWidgetPoolManager, STATGROUP_WidgetPoolManager);
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UWidgetPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Cache WidgetManagerBase ref (Rule #41)
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer)
	{
		CachedWidgetManager = LocalPlayer->GetSubsystem<UWidgetManagerBase>();
	}
}

void UWidgetPoolManager::Deinitialize()
{
	// Destroy all pooled widgets
	for (FWidgetPool& Pool : Pools)
	{
		for (FPooledWidgetEntry& Entry : Pool.Entries)
		{
			if (Entry.Widget)
			{
				Entry.Widget->RemoveFromParent();
				Entry.Widget->ConditionalBeginDestroy();
				Entry.Widget = nullptr;
			}
		}
		Pool.Entries.Empty();
	}

	Pools.Empty();
	TotalActiveCount = 0;

	Super::Deinitialize();
}

// ============================================================================
// TICK - Auto-release and cleanup
// ============================================================================

void UWidgetPoolManager::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_WidgetPoolManagerTick);

	double CurrentTime = FPlatformTime::Seconds();

	for (FWidgetPool& Pool : Pools)
	{
		if (Pool.Config.AutoReleaseTimeout <= 0.0f) continue;

		for (FPooledWidgetEntry& Entry : Pool.Entries)
		{
			if (!Entry.bActive) continue;
			if (!Entry.Widget) continue;

			// Check auto-release timeout
			double Elapsed = CurrentTime - Entry.AcquireTime;
			if (Elapsed >= Pool.Config.AutoReleaseTimeout)
			{
				DeactivateEntry(Entry, Pool);
			}
		}
	}
}

// ============================================================================
// POOL REGISTRATION
// ============================================================================

void UWidgetPoolManager::RegisterPool(TSubclassOf<UUserWidget> WidgetClass, const FWidgetPoolConfig& Config)
{
	if (!WidgetClass || !Config.IsValid()) return;

	// Check if already registered
	if (FindPool(WidgetClass)) return;

	FWidgetPool NewPool;
	NewPool.WidgetClass = WidgetClass;
	NewPool.Config = Config;

	// Prewarm
	if (Config.PrewarmCount > 0)
	{
		int32 Count = FMath::Min(Config.PrewarmCount, Config.MaxPoolSize);
		NewPool.Entries.Reserve(Count);

		for (int32 i = 0; i < Count; ++i)
		{
			FPooledWidgetEntry Entry;
			Entry.Widget = CreatePooledInstance(NewPool);
			if (Entry.Widget)
			{
				Entry.bActive = false;
				Entry.Widget->SetVisibility(ESlateVisibility::Collapsed);
				NewPool.Entries.Add(Entry);
				NewPool.Stats.TotalInstances++;
				NewPool.Stats.PooledCount++;
			}
		}
	}

	Pools.Add(MoveTemp(NewPool));
}

void UWidgetPoolManager::UnregisterPool(TSubclassOf<UUserWidget> WidgetClass)
{
	for (int32 i = Pools.Num() - 1; i >= 0; --i)
	{
		if (Pools[i].WidgetClass == WidgetClass)
		{
			// Destroy all instances
			for (FPooledWidgetEntry& Entry : Pools[i].Entries)
			{
				if (Entry.bActive)
				{
					TotalActiveCount--;
				}
				if (Entry.Widget)
				{
					Entry.Widget->RemoveFromParent();
					Entry.Widget->ConditionalBeginDestroy();
				}
			}
			Pools.RemoveAtSwap(i);
			break;
		}
	}
}

bool UWidgetPoolManager::HasPool(TSubclassOf<UUserWidget> WidgetClass) const
{
	return FindPool(WidgetClass) != nullptr;
}

// ============================================================================
// ACQUIRE / RELEASE
// ============================================================================

UUserWidget* UWidgetPoolManager::AcquireWidget(TSubclassOf<UUserWidget> WidgetClass, int32 Priority, FVector2D ScreenPosition)
{
	FWidgetPool* Pool = FindPool(WidgetClass);
	if (!Pool) return nullptr;

	// Try to find an inactive entry first (recycle)
	FPooledWidgetEntry* Entry = FindInactiveEntry(*Pool);

	if (!Entry)
	{
		// No inactive entries - try to create new if under limit
		if (Pool->Entries.Num() < Pool->Config.MaxPoolSize)
		{
			FPooledWidgetEntry NewEntry;
			NewEntry.Widget = CreatePooledInstance(*Pool);
			if (NewEntry.Widget)
			{
				Pool->Entries.Add(NewEntry);
				Pool->Stats.TotalInstances++;
				Entry = &Pool->Entries.Last();
			}
		}
	}

	if (!Entry)
	{
		// Pool is full - try eviction
		Entry = EvictWidget(*Pool);
	}

	if (!Entry || !Entry->Widget)
	{
		return nullptr; // Pool full and no eviction possible
	}

	// Apply spatial clustering
	FVector2D FinalPosition = ScreenPosition;
	if (Pool->Config.ClusterDistanceThreshold > 0.0f)
	{
		FinalPosition = ApplySpatialClustering(*Pool, ScreenPosition);
	}

	ActivateEntry(*Entry, *Pool, Priority, FinalPosition);

	Pool->Stats.TotalAcquisitions++;

	return Entry->Widget;
}

void UWidgetPoolManager::ReleaseWidget(UUserWidget* Widget)
{
	if (!Widget) return;

	FWidgetPool* Pool = FindPoolForWidget(Widget);
	if (!Pool) return;

	FPooledWidgetEntry* Entry = FindEntry(*Pool, Widget);
	if (!Entry || !Entry->bActive) return;

	DeactivateEntry(*Entry, *Pool);
}

void UWidgetPoolManager::ReleaseAllInPool(TSubclassOf<UUserWidget> WidgetClass)
{
	FWidgetPool* Pool = FindPool(WidgetClass);
	if (!Pool) return;

	for (FPooledWidgetEntry& Entry : Pool->Entries)
	{
		if (Entry.bActive)
		{
			DeactivateEntry(Entry, *Pool);
		}
	}
}

// ============================================================================
// QUERIES
// ============================================================================

FWidgetPoolStats UWidgetPoolManager::GetPoolStats(TSubclassOf<UUserWidget> WidgetClass) const
{
	if (const FWidgetPool* Pool = FindPool(WidgetClass))
	{
		return Pool->Stats;
	}
	return FWidgetPoolStats();
}

TArray<UUserWidget*> UWidgetPoolManager::GetActiveWidgets(TSubclassOf<UUserWidget> WidgetClass) const
{
	TArray<UUserWidget*> Result;
	if (const FWidgetPool* Pool = FindPool(WidgetClass))
	{
		for (const FPooledWidgetEntry& Entry : Pool->Entries)
		{
			if (Entry.bActive && Entry.Widget)
			{
				Result.Add(Entry.Widget);
			}
		}
	}
	return Result;
}

// ============================================================================
// INTERNAL - Pool lookup
// ============================================================================

FWidgetPool* UWidgetPoolManager::FindPool(TSubclassOf<UUserWidget> WidgetClass)
{
	for (FWidgetPool& Pool : Pools)
	{
		if (Pool.WidgetClass == WidgetClass)
		{
			return &Pool;
		}
	}
	return nullptr;
}

const FWidgetPool* UWidgetPoolManager::FindPool(TSubclassOf<UUserWidget> WidgetClass) const
{
	for (const FWidgetPool& Pool : Pools)
	{
		if (Pool.WidgetClass == WidgetClass)
		{
			return &Pool;
		}
	}
	return nullptr;
}

FWidgetPool* UWidgetPoolManager::FindPoolForWidget(UUserWidget* Widget)
{
	if (!Widget) return nullptr;

	for (FWidgetPool& Pool : Pools)
	{
		for (const FPooledWidgetEntry& Entry : Pool.Entries)
		{
			if (Entry.Widget == Widget)
			{
				return &Pool;
			}
		}
	}
	return nullptr;
}

FPooledWidgetEntry* UWidgetPoolManager::FindEntry(FWidgetPool& Pool, UUserWidget* Widget)
{
	for (FPooledWidgetEntry& Entry : Pool.Entries)
	{
		if (Entry.Widget == Widget)
		{
			return &Entry;
		}
	}
	return nullptr;
}

// ============================================================================
// INTERNAL - Instance management
// ============================================================================

UUserWidget* UWidgetPoolManager::CreatePooledInstance(FWidgetPool& Pool)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return nullptr;

	APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, Pool.WidgetClass);
	if (Widget)
	{
		// Add to viewport but hidden - pooled widgets stay in the tree
		Widget->AddToViewport(Pool.Config.ZOrder);
		Widget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return Widget;
}

FPooledWidgetEntry* UWidgetPoolManager::FindInactiveEntry(FWidgetPool& Pool)
{
	for (FPooledWidgetEntry& Entry : Pool.Entries)
	{
		if (!Entry.bActive && Entry.Widget)
		{
			return &Entry;
		}
	}
	return nullptr;
}

FPooledWidgetEntry* UWidgetPoolManager::EvictWidget(FWidgetPool& Pool)
{
	if (Pool.Config.EvictionPolicy == EWidgetEvictionPolicy::None)
	{
		return nullptr; // No eviction allowed
	}

	FPooledWidgetEntry* Candidate = nullptr;

	for (FPooledWidgetEntry& Entry : Pool.Entries)
	{
		if (!Entry.bActive || !Entry.Widget) continue;

		if (!Candidate)
		{
			Candidate = &Entry;
			continue;
		}

		switch (Pool.Config.EvictionPolicy)
		{
		case EWidgetEvictionPolicy::Oldest:
			if (Entry.AcquireTime < Candidate->AcquireTime)
			{
				Candidate = &Entry;
			}
			break;

		case EWidgetEvictionPolicy::LowestPriority:
			if (Entry.Priority < Candidate->Priority ||
				(Entry.Priority == Candidate->Priority && Entry.AcquireTime < Candidate->AcquireTime))
			{
				Candidate = &Entry;
			}
			break;

		default:
			break;
		}
	}

	if (Candidate)
	{
		// Deactivate the evicted widget
		DeactivateEntry(*Candidate, Pool);
		Pool.Stats.TotalEvictions++;
	}

	return Candidate;
}

void UWidgetPoolManager::ActivateEntry(FPooledWidgetEntry& Entry, FWidgetPool& Pool, int32 Priority, FVector2D ScreenPosition)
{
	Entry.bActive = true;
	Entry.Priority = Priority;
	Entry.AcquireTime = FPlatformTime::Seconds();
	Entry.ScreenPosition = ScreenPosition;

	if (Entry.Widget)
	{
		// Position the widget
		Entry.Widget->SetRenderTranslation(ScreenPosition);
		Entry.Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	Pool.Stats.ActiveCount++;
	Pool.Stats.PooledCount = FMath::Max(0, Pool.Stats.PooledCount - 1);
	TotalActiveCount++;

	OnWidgetAcquired.Broadcast(Entry.Widget, Pool.Config.PoolTag);
}

void UWidgetPoolManager::DeactivateEntry(FPooledWidgetEntry& Entry, FWidgetPool& Pool)
{
	Entry.bActive = false;
	Entry.Priority = 0;
	Entry.AcquireTime = 0.0;
	Entry.ScreenPosition = FVector2D::ZeroVector;

	if (Entry.Widget)
	{
		Entry.Widget->SetVisibility(ESlateVisibility::Collapsed);
	}

	Pool.Stats.ActiveCount = FMath::Max(0, Pool.Stats.ActiveCount - 1);
	Pool.Stats.PooledCount++;
	TotalActiveCount = FMath::Max(0, TotalActiveCount - 1);

	OnWidgetReleased.Broadcast(Entry.Widget, Pool.Config.PoolTag);
}

// ============================================================================
// INTERNAL - Spatial clustering
// ============================================================================

FVector2D UWidgetPoolManager::ApplySpatialClustering(FWidgetPool& Pool, FVector2D RequestedPosition)
{
	float Threshold = Pool.Config.ClusterDistanceThreshold;
	if (Threshold <= 0.0f) return RequestedPosition;

	FVector2D FinalPosition = RequestedPosition;
	bool bNeedsOffset = true;
	int32 MaxIterations = 8; // Prevent infinite loop

	while (bNeedsOffset && MaxIterations > 0)
	{
		bNeedsOffset = false;

		for (const FPooledWidgetEntry& Entry : Pool.Entries)
		{
			if (!Entry.bActive) continue;

			float Distance = FVector2D::Distance(FinalPosition, Entry.ScreenPosition);
			if (Distance < Threshold && Distance > 0.01f)
			{
				// Push away from existing widget
				FVector2D Direction = (FinalPosition - Entry.ScreenPosition).GetSafeNormal();
				if (Direction.IsNearlyZero())
				{
					// Default to pushing upward if exactly overlapping
					Direction = FVector2D(0.0f, -1.0f);
				}
				FinalPosition = Entry.ScreenPosition + Direction * Threshold;
				bNeedsOffset = true;
				break; // Re-check all after adjustment
			}
		}

		MaxIterations--;
	}

	return FinalPosition;
}
