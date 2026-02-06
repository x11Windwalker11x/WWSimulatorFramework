// DockableLayoutData.h
// Data structs for Dockable/Composable Layout Engine
// Layer: L0 (SharedDefaults) - Pure data, no logic, no UMG dependency

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DockableLayoutData.generated.h"

/**
 * Dock position within a dock zone.
 */
UENUM(BlueprintType)
enum class EDockPosition : uint8
{
	Left		UMETA(DisplayName = "Left"),
	Right		UMETA(DisplayName = "Right"),
	Top			UMETA(DisplayName = "Top"),
	Bottom		UMETA(DisplayName = "Bottom"),
	Center		UMETA(DisplayName = "Center"),
	Float		UMETA(DisplayName = "Floating")
};

/**
 * Split orientation for split-view containers.
 */
UENUM(BlueprintType)
enum class EDockSplitOrientation : uint8
{
	Horizontal	UMETA(DisplayName = "Horizontal"),
	Vertical	UMETA(DisplayName = "Vertical")
};

/**
 * How to resolve conflicts when two widgets target the same dock slot.
 */
UENUM(BlueprintType)
enum class EDockConflictPolicy : uint8
{
	/** Reject the incoming widget (first-come-first-served) */
	Reject		UMETA(DisplayName = "Reject"),
	/** Swap: undock existing, dock incoming */
	Replace		UMETA(DisplayName = "Replace"),
	/** Create a tabbed container with both widgets */
	Tab			UMETA(DisplayName = "Tab"),
	/** Split the zone and dock both side by side */
	Split		UMETA(DisplayName = "Split")
};

// ============================================================================
// DOCK ZONE CONFIGURATION
// ============================================================================

/**
 * Configuration for a dock zone.
 * Defines what the zone accepts, how many widgets, and conflict rules.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FDockZoneConfig
{
	GENERATED_BODY()

	/** Unique tag identifying this dock zone (e.g., UI.Dock.Zone.LeftPanel) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Zone")
	FGameplayTag ZoneTag;

	/** Maximum number of widgets this zone can hold (0 = unlimited) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Zone", meta = (ClampMin = "0"))
	int32 MaxOccupants = 1;

	/** Default dock position for widgets entering this zone */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Zone")
	EDockPosition DefaultPosition = EDockPosition::Center;

	/** Default split orientation when splitting occurs */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Zone")
	EDockSplitOrientation DefaultSplitOrientation = EDockSplitOrientation::Horizontal;

	/** How to handle conflicts (two widgets in the same slot) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Zone")
	EDockConflictPolicy ConflictPolicy = EDockConflictPolicy::Tab;

	/** Filter: only widgets with matching tags can dock here (empty = accept all) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Zone")
	FGameplayTagContainer AcceptedWidgetTags;

	/** Zone size ratio relative to parent (0.0 - 1.0, 0 = auto) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Zone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SizeRatio = 0.0f;

	bool IsValid() const
	{
		return ZoneTag.IsValid();
	}
};

// ============================================================================
// DOCKABLE WIDGET CONFIGURATION
// ============================================================================

/**
 * Configuration for a dockable widget.
 * Describes how a widget prefers to be docked.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FDockableWidgetConfig
{
	GENERATED_BODY()

	/** Unique identifier for this dockable widget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Widget")
	FName DockableID;

	/** Widget type tag for filtering */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Widget")
	FGameplayTag WidgetTypeTag;

	/** Preferred dock position */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Widget")
	EDockPosition PreferredPosition = EDockPosition::Center;

	/** Preferred dock zone (empty = any zone) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Widget")
	FGameplayTag PreferredZoneTag;

	/** Can this widget float (be dragged out of dock zones)? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Widget")
	bool bCanFloat = true;

	/** Can this widget be resized within its dock zone? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Widget")
	bool bCanResize = true;

	/** Minimum size (0 = no minimum) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dock Widget")
	FVector2D MinSize = FVector2D::ZeroVector;

	bool IsValid() const
	{
		return !DockableID.IsNone();
	}
};

// ============================================================================
// LAYOUT SERIALIZATION
// ============================================================================

/**
 * A single entry in a dock layout - one widget's position.
 * Serializable for save/load.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FDockLayoutEntry
{
	GENERATED_BODY()

	/** The dockable widget's unique ID */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Layout")
	FName DockableID;

	/** The dock zone this widget is in */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Layout")
	FGameplayTag ZoneTag;

	/** Position within the zone */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Layout")
	EDockPosition Position = EDockPosition::Center;

	/** Size ratio within zone (for splits) */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Layout")
	float SizeRatio = 1.0f;

	/** Tab index if in a tabbed container (-1 = not tabbed) */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Layout")
	int32 TabIndex = -1;

	/** Floating position (only used when Position == Float) */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Layout")
	FVector2D FloatPosition = FVector2D::ZeroVector;

	/** Floating size (only used when Position == Float) */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Layout")
	FVector2D FloatSize = FVector2D(400.0f, 300.0f);

	bool IsValid() const
	{
		return !DockableID.IsNone() && ZoneTag.IsValid();
	}
};

/**
 * A complete dock layout - all widgets and their positions.
 * Serializable snapshot for save/load/preset.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FDockLayout
{
	GENERATED_BODY()

	/** Layout name (for presets) */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Layout")
	FName LayoutName;

	/** All widget positions in this layout */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Layout")
	TArray<FDockLayoutEntry> Entries;

	bool IsValid() const
	{
		return !LayoutName.IsNone() && Entries.Num() > 0;
	}
};

/**
 * Split-view configuration for a dock zone.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FDockSplitConfig
{
	GENERATED_BODY()

	/** Split orientation */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Split")
	EDockSplitOrientation Orientation = EDockSplitOrientation::Horizontal;

	/** Split ratio (0.0 - 1.0, where 0.5 = equal split) */
	UPROPERTY(BlueprintReadWrite, Category = "Dock Split", meta = (ClampMin = "0.1", ClampMax = "0.9"))
	float SplitRatio = 0.5f;

	bool IsValid() const
	{
		return SplitRatio > 0.0f && SplitRatio < 1.0f;
	}
};
