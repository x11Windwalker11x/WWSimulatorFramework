// WidgetPoolData.h
// Data structs for Widget Pooling System
// Layer: L0 (SharedDefaults) - Pure data, no logic, no UMG dependency

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WidgetPoolData.generated.h"

/**
 * How a pooled widget is evicted when the pool is full.
 * Operator enum per convention.
 */
UENUM(BlueprintType)
enum class EWidgetEvictionPolicy : uint8
{
	/** Evict the oldest active widget */
	Oldest		UMETA(DisplayName = "Oldest"),

	/** Evict the lowest-priority active widget */
	LowestPriority	UMETA(DisplayName = "Lowest Priority"),

	/** No eviction - refuse new widget if pool full */
	None		UMETA(DisplayName = "None")
};

/**
 * Configuration for a widget pool.
 * One config per widget class that participates in pooling.
 * Pure data struct - no logic (Rule #12).
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWidgetPoolConfig
{
	GENERATED_BODY()

	/** Widget class to pool (set at registration, not in DataTable) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Pool")
	FGameplayTag PoolTag;

	/** Maximum number of instances in the pool (active + inactive) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Pool", meta = (ClampMin = "1"))
	int32 MaxPoolSize = 10;

	/** Number of instances to pre-create on registration (0 = lazy) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Pool", meta = (ClampMin = "0"))
	int32 PrewarmCount = 0;

	/** Auto-release timeout in seconds (0 = manual release only) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Pool", meta = (ClampMin = "0.0"))
	float AutoReleaseTimeout = 3.0f;

	/** Eviction policy when pool is full */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Pool")
	EWidgetEvictionPolicy EvictionPolicy = EWidgetEvictionPolicy::Oldest;

	/** Minimum screen distance between widgets before clustering (pixels, 0 = no clustering) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Pool|Spatial", meta = (ClampMin = "0.0"))
	float ClusterDistanceThreshold = 0.0f;

	/** Z-order for pooled widgets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Pool")
	int32 ZOrder = 10;

	bool IsValid() const
	{
		return MaxPoolSize > 0;
	}
};

/**
 * Delegate payload for pool events (L0 so any layer can listen).
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWidgetPoolStats
{
	GENERATED_BODY()

	/** Total instances created */
	UPROPERTY(BlueprintReadOnly, Category = "Widget Pool|Stats")
	int32 TotalInstances = 0;

	/** Currently active (visible) instances */
	UPROPERTY(BlueprintReadOnly, Category = "Widget Pool|Stats")
	int32 ActiveCount = 0;

	/** Currently pooled (hidden, ready for reuse) instances */
	UPROPERTY(BlueprintReadOnly, Category = "Widget Pool|Stats")
	int32 PooledCount = 0;

	/** Total acquisitions since registration */
	UPROPERTY(BlueprintReadOnly, Category = "Widget Pool|Stats")
	int32 TotalAcquisitions = 0;

	/** Total evictions since registration */
	UPROPERTY(BlueprintReadOnly, Category = "Widget Pool|Stats")
	int32 TotalEvictions = 0;

	bool IsValid() const
	{
		return TotalInstances >= 0;
	}
};
