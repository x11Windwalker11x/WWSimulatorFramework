// WidgetSyncData.h
// Data structs for Multiplayer Widget Synchronization
// Layer: L0 (SharedDefaults) - Pure data, no logic, no UMG dependency

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WidgetSyncData.generated.h"

/**
 * A single property delta for widget sync.
 * Key-value pair representing a changed property.
 * Serialized as compact binary for bandwidth optimization.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWidgetSyncProperty
{
	GENERATED_BODY()

	/** Property name (short key, not full path) */
	UPROPERTY(BlueprintReadWrite, Category = "Widget Sync")
	FName PropertyName;

	/** Serialized property value as string (compact format) */
	UPROPERTY(BlueprintReadWrite, Category = "Widget Sync")
	FString Value;

	bool IsValid() const
	{
		return !PropertyName.IsNone();
	}
};

/**
 * A snapshot of changed widget properties for replication.
 * Contains only deltas (changed properties since last sync), not full state.
 * Bandwidth optimization: only what changed gets replicated.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWidgetSyncPayload
{
	GENERATED_BODY()

	/** Unique widget sync ID (deterministic, same widget = same ID across clients) */
	UPROPERTY(BlueprintReadWrite, Category = "Widget Sync")
	FName WidgetSyncID;

	/** Widget class for validation */
	UPROPERTY(BlueprintReadWrite, Category = "Widget Sync")
	FGameplayTag WidgetTypeTag;

	/** Owning player (server-authoritative) */
	UPROPERTY(BlueprintReadWrite, Category = "Widget Sync")
	int32 OwningPlayerID = -1;

	/** Server timestamp for ordering */
	UPROPERTY(BlueprintReadWrite, Category = "Widget Sync")
	float ServerTimestamp = 0.0f;

	/** Changed properties (delta only) */
	UPROPERTY(BlueprintReadWrite, Category = "Widget Sync")
	TArray<FWidgetSyncProperty> Properties;

	/** Sequence number for delta ordering (prevents out-of-order application) */
	UPROPERTY(BlueprintReadWrite, Category = "Widget Sync")
	uint32 SequenceNumber = 0;

	bool IsValid() const
	{
		return !WidgetSyncID.IsNone() && Properties.Num() > 0;
	}
};

/**
 * Configuration for a synced widget registration.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWidgetSyncConfig
{
	GENERATED_BODY()

	/** How frequently to check for deltas (seconds, 0 = every frame) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Sync", meta = (ClampMin = "0.0"))
	float SyncInterval = 0.1f;

	/** Priority for bandwidth allocation (higher = synced first when constrained) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Sync", meta = (ClampMin = "0"))
	int32 SyncPriority = 0;

	/** Maximum properties per sync payload (0 = unlimited) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Sync", meta = (ClampMin = "0"))
	int32 MaxPropertiesPerSync = 0;

	/** Allow spectators to observe this widget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Sync")
	bool bAllowSpectating = true;

	bool IsValid() const
	{
		return SyncInterval >= 0.0f;
	}
};

/**
 * Spectator binding entry - one client observing another's widget.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FSpectatorBinding
{
	GENERATED_BODY()

	/** Player ID of the spectator (the observer) */
	UPROPERTY(BlueprintReadOnly, Category = "Widget Sync")
	int32 SpectatorPlayerID = -1;

	/** Player ID of the target (the one being observed) */
	UPROPERTY(BlueprintReadOnly, Category = "Widget Sync")
	int32 TargetPlayerID = -1;

	/** Widget sync ID being observed */
	UPROPERTY(BlueprintReadOnly, Category = "Widget Sync")
	FName WidgetSyncID;

	bool IsValid() const
	{
		return SpectatorPlayerID >= 0 && TargetPlayerID >= 0 && !WidgetSyncID.IsNone();
	}
};
