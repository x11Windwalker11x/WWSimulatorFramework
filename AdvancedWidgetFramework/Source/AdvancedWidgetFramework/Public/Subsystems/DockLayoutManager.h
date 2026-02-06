// DockLayoutManager.h
// Dockable/Composable Layout Engine - dock zone management and layout persistence
// Layer: L2 (AdvancedWidgetFramework)

#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/AdvancedWidgetFramework/DockableLayoutData.h"
#include "Delegates/AdvancedWidgetFramework/WW_WidgetDelegates.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "DockLayoutManager.generated.h"

class UWidgetManagerBase;
class UDockZoneComponent;

/**
 * Internal tracking for a docked widget.
 */
USTRUCT()
struct FDockedWidgetEntry
{
	GENERATED_BODY()

	/** The docked widget */
	UPROPERTY()
	TWeakObjectPtr<UUserWidget> Widget = nullptr;

	/** Dockable widget ID */
	FName DockableID;

	/** Widget type tag */
	FGameplayTag WidgetTypeTag;

	/** Zone this widget is docked in */
	FGameplayTag ZoneTag;

	/** Position within the zone */
	EDockPosition Position = EDockPosition::Center;

	/** Size ratio within zone */
	float SizeRatio = 1.0f;

	/** Tab index (-1 = not tabbed) */
	int32 TabIndex = -1;

	/** Floating position (only if Position == Float) */
	FVector2D FloatPosition = FVector2D::ZeroVector;

	/** Floating size (only if Position == Float) */
	FVector2D FloatSize = FVector2D(400.0f, 300.0f);

	/** Dock configuration from the widget */
	FDockableWidgetConfig Config;
};

/**
 * Internal tracking for a registered dock zone.
 */
USTRUCT()
struct FDockZoneEntry
{
	GENERATED_BODY()

	/** Zone configuration */
	FDockZoneConfig Config;

	/** Component that owns this zone (if any) */
	TWeakObjectPtr<UDockZoneComponent> OwningComponent;

	/** Widgets currently docked in this zone */
	TArray<FName> DockedWidgetIDs;

	/** Current split config (if zone is split) */
	FDockSplitConfig SplitConfig;

	/** Active tab index for tabbed zones */
	int32 ActiveTabIndex = 0;
};

/**
 * UDockLayoutManager
 *
 * Manages the dockable/composable layout engine.
 *
 * Features:
 * - Dock zone registration and management
 * - Widget dock/undock with conflict resolution
 * - Split-view and tabbed container logic
 * - Layout save/load (serializable FDockLayout snapshots)
 * - Player-rearrangeable dock zones
 * - Registers into WidgetManagerBase via delegate
 * - Delete AWF -> no docking, widgets work normally (graceful degradation)
 *
 * Architecture:
 * - Subsystem manages logical layout (which widget in which zone)
 * - UDockZoneComponent marks physical dock targets in world/UI
 * - No L2->L2 dependencies
 */
UCLASS()
class ADVANCEDWIDGETFRAMEWORK_API UDockLayoutManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(ULocalPlayer* LocalPlayer) const override { return true; }

	// ============================================================================
	// DOCK ZONE MANAGEMENT
	// ============================================================================

	/**
	 * Register a dock zone.
	 * @param Config - Zone configuration
	 * @param OwningComponent - Optional component that owns this zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout")
	void RegisterDockZone(const FDockZoneConfig& Config);

	/** Register a dock zone with an owning component */
	void RegisterDockZoneWithComponent(const FDockZoneConfig& Config, UDockZoneComponent* Component);

	/**
	 * Unregister a dock zone. Undocks all widgets in the zone first.
	 * @param ZoneTag - The zone to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout")
	void UnregisterDockZone(FGameplayTag ZoneTag);

	/** Check if a dock zone is registered */
	UFUNCTION(BlueprintPure, Category = "Dock Layout")
	bool IsDockZoneRegistered(FGameplayTag ZoneTag) const;

	// ============================================================================
	// DOCK / UNDOCK
	// ============================================================================

	/**
	 * Dock a widget into a zone.
	 * Widget must implement IDockableWidgetInterface.
	 * @param Widget - The widget to dock
	 * @param ZoneTag - Target dock zone
	 * @param Position - Position within the zone (default = use widget preference)
	 * @return True if docking succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout")
	bool DockWidget(UUserWidget* Widget, FGameplayTag ZoneTag, EDockPosition Position = EDockPosition::Center);

	/**
	 * Undock a widget from its current zone.
	 * @param Widget - The widget to undock
	 * @return True if undocking succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout")
	bool UndockWidget(UUserWidget* Widget);

	/**
	 * Move a docked widget to a new zone.
	 * @param Widget - The widget to move
	 * @param NewZoneTag - The target zone
	 * @param NewPosition - New position within the zone
	 * @return True if move succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout")
	bool MoveWidget(UUserWidget* Widget, FGameplayTag NewZoneTag, EDockPosition NewPosition = EDockPosition::Center);

	/**
	 * Float a widget (undock and place at screen position).
	 * @param Widget - The widget to float
	 * @param ScreenPosition - Where to place the floating widget
	 * @param Size - Floating widget size
	 * @return True if float succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout")
	bool FloatWidget(UUserWidget* Widget, FVector2D ScreenPosition, FVector2D Size = FVector2D(400.0f, 300.0f));

	// ============================================================================
	// SPLIT AND TAB
	// ============================================================================

	/**
	 * Split a dock zone and dock a widget in the new sub-zone.
	 * @param ZoneTag - Zone to split
	 * @param Widget - Widget to dock in the new split
	 * @param Orientation - Split orientation
	 * @param SplitRatio - Where to split (0.5 = equal)
	 * @return True if split succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout|Split")
	bool SplitZone(FGameplayTag ZoneTag, UUserWidget* Widget, EDockSplitOrientation Orientation = EDockSplitOrientation::Horizontal, float SplitRatio = 0.5f);

	/**
	 * Set the active tab in a tabbed dock zone.
	 * @param ZoneTag - The tabbed zone
	 * @param TabIndex - Which tab to activate
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout|Tab")
	void SetActiveTab(FGameplayTag ZoneTag, int32 TabIndex);

	/** Get the active tab index for a zone */
	UFUNCTION(BlueprintPure, Category = "Dock Layout|Tab")
	int32 GetActiveTab(FGameplayTag ZoneTag) const;

	/** Get all widgets in a tabbed zone */
	UFUNCTION(BlueprintPure, Category = "Dock Layout|Tab")
	TArray<UUserWidget*> GetTabWidgets(FGameplayTag ZoneTag) const;

	// ============================================================================
	// LAYOUT PERSISTENCE
	// ============================================================================

	/**
	 * Capture current layout as a serializable snapshot.
	 * @param LayoutName - Name for the layout preset
	 * @return The captured layout
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout|Persistence")
	FDockLayout CaptureLayout(FName LayoutName) const;

	/**
	 * Apply a layout snapshot (restore all widget positions).
	 * @param Layout - The layout to apply
	 * @return True if layout was applied successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout|Persistence")
	bool ApplyLayout(const FDockLayout& Layout);

	/**
	 * Reset all docking to default positions (from widget configs).
	 */
	UFUNCTION(BlueprintCallable, Category = "Dock Layout|Persistence")
	void ResetToDefaultLayout();

	// ============================================================================
	// QUERIES
	// ============================================================================

	/** Check if a widget is currently docked */
	UFUNCTION(BlueprintPure, Category = "Dock Layout")
	bool IsWidgetDocked(UUserWidget* Widget) const;

	/** Get the zone a widget is docked in */
	UFUNCTION(BlueprintPure, Category = "Dock Layout")
	FGameplayTag GetWidgetDockZone(UUserWidget* Widget) const;

	/** Get all widgets in a zone */
	UFUNCTION(BlueprintPure, Category = "Dock Layout")
	TArray<UUserWidget*> GetWidgetsInZone(FGameplayTag ZoneTag) const;

	/** Get all registered zone tags */
	UFUNCTION(BlueprintPure, Category = "Dock Layout")
	TArray<FGameplayTag> GetRegisteredZones() const;

	/** Get zone config */
	UFUNCTION(BlueprintPure, Category = "Dock Layout")
	FDockZoneConfig GetZoneConfig(FGameplayTag ZoneTag) const;

	// ============================================================================
	// DELEGATES
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Dock Layout|Events")
	FOnWidgetDocked OnWidgetDocked;

	UPROPERTY(BlueprintAssignable, Category = "Dock Layout|Events")
	FOnWidgetUndocked OnWidgetUndocked;

	UPROPERTY(BlueprintAssignable, Category = "Dock Layout|Events")
	FOnDockLayoutChanged OnLayoutChanged;

private:
	// ============================================================================
	// INTERCEPT HANDLER
	// ============================================================================

	/** Called by WidgetManagerBase when a dock request is made */
	bool HandleDockIntercept(UUserWidget* Widget, FGameplayTag TargetZoneTag);

	// ============================================================================
	// CONFLICT RESOLUTION
	// ============================================================================

	/**
	 * Resolve a dock conflict when a zone is full.
	 * @param Zone - The zone entry
	 * @param IncomingWidget - The widget trying to dock
	 * @return True if conflict was resolved (incoming can dock)
	 */
	bool ResolveConflict(FDockZoneEntry& Zone, UUserWidget* IncomingWidget);

	// ============================================================================
	// INTERNAL
	// ============================================================================

	/** Find docked entry for a widget */
	FDockedWidgetEntry* FindDockedEntry(UUserWidget* Widget);
	const FDockedWidgetEntry* FindDockedEntry(UUserWidget* Widget) const;

	/** Find docked entry by ID */
	FDockedWidgetEntry* FindDockedEntryByID(FName DockableID);

	/** Find zone entry */
	FDockZoneEntry* FindZone(FGameplayTag ZoneTag);
	const FDockZoneEntry* FindZone(FGameplayTag ZoneTag) const;

	/** Check if a widget is accepted by a zone's tag filter */
	bool IsWidgetAcceptedByZone(const FDockZoneEntry& Zone, FGameplayTag WidgetTypeTag) const;

	/** Broadcast layout change to all listeners */
	void NotifyLayoutChanged();

	/** Cached reference to WidgetManagerBase (Rule #41) */
	TWeakObjectPtr<UWidgetManagerBase> CachedWidgetManager;

	/** All registered dock zones */
	UPROPERTY()
	TArray<FDockZoneEntry> DockZones;

	/** All docked widgets */
	UPROPERTY()
	TArray<FDockedWidgetEntry> DockedWidgets;
};
