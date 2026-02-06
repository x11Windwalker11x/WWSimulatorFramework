// WidgetPoolManager.h
// Widget Pooling System - pre-instantiation and recycling for high-frequency widgets
// Layer: L2 (AdvancedWidgetFramework)

#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/AdvancedWidgetFramework/WidgetPoolData.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "WidgetPoolManager.generated.h"

class UWidgetManagerBase;

/**
 * Internal tracking for a single pooled widget instance.
 */
USTRUCT()
struct FPooledWidgetEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UUserWidget> Widget = nullptr;

	/** Is this instance currently active (visible, in use) */
	bool bActive = false;

	/** Priority for eviction (higher = less likely to be evicted) */
	int32 Priority = 0;

	/** World time when acquired (for oldest-first eviction and auto-release) */
	double AcquireTime = 0.0;

	/** Screen position for spatial sorting */
	FVector2D ScreenPosition = FVector2D::ZeroVector;
};

/**
 * Internal tracking for an entire widget pool (one per registered class).
 */
USTRUCT()
struct FWidgetPool
{
	GENERATED_BODY()

	/** Widget class this pool manages */
	UPROPERTY()
	TSubclassOf<UUserWidget> WidgetClass = nullptr;

	/** Pool configuration */
	UPROPERTY()
	FWidgetPoolConfig Config;

	/** All instances (active + pooled) */
	UPROPERTY()
	TArray<FPooledWidgetEntry> Entries;

	/** Stats tracking */
	FWidgetPoolStats Stats;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPooledWidgetAcquired, UUserWidget*, Widget, FGameplayTag, PoolTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPooledWidgetReleased, UUserWidget*, Widget, FGameplayTag, PoolTag);

/**
 * UWidgetPoolManager
 *
 * High-performance widget recycling for frequently spawned/destroyed widgets:
 * damage numbers, floating markers, notifications, kill feed entries.
 *
 * Features:
 * - Pre-instantiation (prewarm) for zero-alloc acquire
 * - Acquire/Release API with automatic recycling
 * - Auto-release timeout
 * - Priority-based eviction when pool full
 * - Screen-space spatial clustering to prevent overlap
 * - Per-pool stats for debugging
 *
 * Architecture:
 * - L2 (AWF) subsystem, no L2→L2 dependencies
 * - Config structs in L0 (SharedDefaults)
 * - Ticks only when active widgets exist (performance)
 */
UCLASS()
class ADVANCEDWIDGETFRAMEWORK_API UWidgetPoolManager : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(ULocalPlayer* LocalPlayer) const override { return true; }

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return TotalActiveCount > 0; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual TStatId GetStatId() const override;

	// ============================================================================
	// POOL REGISTRATION
	// ============================================================================

	/**
	 * Register a widget class for pooling.
	 * Optionally prewarms the pool with PrewarmCount instances.
	 * @param WidgetClass - The widget class to pool
	 * @param Config - Pool configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Pool")
	void RegisterPool(TSubclassOf<UUserWidget> WidgetClass, const FWidgetPoolConfig& Config);

	/**
	 * Unregister a pool and destroy all instances.
	 * @param WidgetClass - The widget class to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Pool")
	void UnregisterPool(TSubclassOf<UUserWidget> WidgetClass);

	/** Check if a pool exists for a widget class */
	UFUNCTION(BlueprintPure, Category = "Widget Pool")
	bool HasPool(TSubclassOf<UUserWidget> WidgetClass) const;

	// ============================================================================
	// ACQUIRE / RELEASE
	// ============================================================================

	/**
	 * Acquire a widget from the pool (or create if pool not full).
	 * Returns nullptr if pool is full and eviction policy is None.
	 * @param WidgetClass - The widget class to acquire
	 * @param Priority - Priority for eviction (higher = kept longer)
	 * @param ScreenPosition - Screen position for spatial sorting
	 * @return The acquired widget, or nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Pool")
	UUserWidget* AcquireWidget(TSubclassOf<UUserWidget> WidgetClass, int32 Priority = 0, FVector2D ScreenPosition = FVector2D::ZeroVector);

	/**
	 * Release a widget back to the pool for reuse.
	 * @param Widget - The widget to release
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Pool")
	void ReleaseWidget(UUserWidget* Widget);

	/**
	 * Release all active widgets in a pool.
	 * @param WidgetClass - The widget class pool to release
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Pool")
	void ReleaseAllInPool(TSubclassOf<UUserWidget> WidgetClass);

	// ============================================================================
	// QUERIES
	// ============================================================================

	/** Get stats for a pool */
	UFUNCTION(BlueprintPure, Category = "Widget Pool|Stats")
	FWidgetPoolStats GetPoolStats(TSubclassOf<UUserWidget> WidgetClass) const;

	/** Get total active widgets across all pools */
	UFUNCTION(BlueprintPure, Category = "Widget Pool|Stats")
	int32 GetTotalActiveWidgets() const { return TotalActiveCount; }

	/** Get all active widgets in a pool */
	UFUNCTION(BlueprintPure, Category = "Widget Pool")
	TArray<UUserWidget*> GetActiveWidgets(TSubclassOf<UUserWidget> WidgetClass) const;

	// ============================================================================
	// DELEGATES
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Widget Pool|Events")
	FOnPooledWidgetAcquired OnWidgetAcquired;

	UPROPERTY(BlueprintAssignable, Category = "Widget Pool|Events")
	FOnPooledWidgetReleased OnWidgetReleased;

private:
	// ============================================================================
	// INTERNAL
	// ============================================================================

	/** Find pool for a widget class (mutable) */
	FWidgetPool* FindPool(TSubclassOf<UUserWidget> WidgetClass);

	/** Find pool for a widget class (const) */
	const FWidgetPool* FindPool(TSubclassOf<UUserWidget> WidgetClass) const;

	/** Find pool that contains a specific widget instance */
	FWidgetPool* FindPoolForWidget(UUserWidget* Widget);

	/** Find entry within a pool for a specific widget */
	FPooledWidgetEntry* FindEntry(FWidgetPool& Pool, UUserWidget* Widget);

	/** Create a new widget instance for the pool */
	UUserWidget* CreatePooledInstance(FWidgetPool& Pool);

	/** Find an inactive entry to reuse */
	FPooledWidgetEntry* FindInactiveEntry(FWidgetPool& Pool);

	/** Evict a widget based on pool policy */
	FPooledWidgetEntry* EvictWidget(FWidgetPool& Pool);

	/** Activate a pooled entry */
	void ActivateEntry(FPooledWidgetEntry& Entry, FWidgetPool& Pool, int32 Priority, FVector2D ScreenPosition);

	/** Deactivate an entry (return to pool) */
	void DeactivateEntry(FPooledWidgetEntry& Entry, FWidgetPool& Pool);

	/** Apply spatial clustering offset to a widget */
	FVector2D ApplySpatialClustering(FWidgetPool& Pool, FVector2D RequestedPosition);

	/** Cached reference to WidgetManagerBase (Rule #41) */
	TWeakObjectPtr<UWidgetManagerBase> CachedWidgetManager;

	/** All registered pools */
	UPROPERTY()
	TArray<FWidgetPool> Pools;

	/** Total active widget count across all pools (for IsTickable optimization) */
	int32 TotalActiveCount = 0;
};
