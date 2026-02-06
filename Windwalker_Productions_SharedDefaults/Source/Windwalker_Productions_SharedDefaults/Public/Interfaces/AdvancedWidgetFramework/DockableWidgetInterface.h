// DockableWidgetInterface.h
// Interface for widgets that participate in the dockable layout engine
// Layer: L0 (SharedDefaults) - Public contract

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Lib/Data/AdvancedWidgetFramework/DockableLayoutData.h"
#include "DockableWidgetInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UDockableWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IDockableWidgetInterface
 *
 * Opt-in interface for widgets that can be docked into dock zones.
 * Implement on any UUserWidget to enable docking via UDockLayoutManager.
 *
 * Examples: inventory panels, skill trees, map panels, chat windows.
 *
 * Rules:
 * - Mandatory getter: GetDockableAsObject() (Rule #29)
 * - Interface calls < 0.02ms (Rule #31)
 * - No Server_/Internal_/OnRep_ functions (Rule #28)
 */
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API IDockableWidgetInterface
{
	GENERATED_BODY()

public:
	// ============================================================================
	// MANDATORY GETTER (Rule #29)
	// ============================================================================

	/** Get the underlying UObject for type-safe access without casting */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dock Widget")
	UObject* GetDockableAsObject() const;

	// ============================================================================
	// DOCK IDENTITY
	// ============================================================================

	/**
	 * Get unique dockable ID for this widget.
	 * Must be deterministic and unique across the layout.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dock Widget")
	FName GetDockableID() const;

	/**
	 * Get the widget type tag for zone filtering.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dock Widget")
	FGameplayTag GetDockWidgetTypeTag() const;

	// ============================================================================
	// DOCK CONFIGURATION
	// ============================================================================

	/**
	 * Get docking configuration for this widget.
	 * Determines preferred position, zone, resize rules.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dock Widget")
	FDockableWidgetConfig GetDockConfig() const;

	// ============================================================================
	// DOCK LIFECYCLE
	// ============================================================================

	/**
	 * Called when this widget is docked into a zone.
	 * @param ZoneTag - The zone the widget was docked into
	 * @param Position - The position within the zone
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dock Widget")
	void OnDocked(const FGameplayTag& ZoneTag, EDockPosition Position);

	/**
	 * Called when this widget is undocked from a zone.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dock Widget")
	void OnUndocked();

	/**
	 * Called when the widget's dock position or size changes.
	 * @param NewPosition - The new position
	 * @param NewSizeRatio - The new size ratio within the zone
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dock Widget")
	void OnDockLayoutChanged(EDockPosition NewPosition, float NewSizeRatio);
};
