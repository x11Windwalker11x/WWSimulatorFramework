// ReplicatedWidgetInterface.h
// Interface for widgets that participate in multiplayer synchronization
// Layer: L0 (SharedDefaults) - Public contract

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Lib/Data/AdvancedWidgetFramework/WidgetSyncData.h"
#include "ReplicatedWidgetInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UReplicatedWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IReplicatedWidgetInterface
 *
 * Opt-in interface for widgets that need multiplayer state synchronization.
 * Implement on any UUserWidget to enable replication via UWidgetSyncSubsystem.
 *
 * Examples: co-op crafting stations, auction house UIs, shared scoreboards.
 *
 * Rules:
 * - Mandatory getter: GetSyncableAsObject() (Rule #29)
 * - Interface calls < 0.02ms (Rule #31)
 * - No Server_/Internal_/OnRep_ functions (Rule #28)
 */
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API IReplicatedWidgetInterface
{
	GENERATED_BODY()

public:
	// ============================================================================
	// MANDATORY GETTER (Rule #29)
	// ============================================================================

	/** Get the underlying UObject for type-safe access without casting */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Widget Sync")
	UObject* GetSyncableAsObject() const;

	// ============================================================================
	// SYNC IDENTITY
	// ============================================================================

	/**
	 * Get unique sync ID for this widget instance.
	 * Must be deterministic: same widget on different clients must return same ID.
	 * Format: "WidgetClass.OwnerName.Context" (e.g., "CraftingUI.Player1.Station3")
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Widget Sync")
	FName GetWidgetSyncID() const;

	/**
	 * Get the widget type tag for validation and routing.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Widget Sync")
	FGameplayTag GetWidgetTypeTag() const;

	// ============================================================================
	// DELTA CAPTURE
	// ============================================================================

	/**
	 * Capture current state as sync properties.
	 * Called by UWidgetSyncSubsystem when building a delta.
	 * Only include properties that have changed since last capture.
	 * @return Array of changed properties
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Widget Sync")
	TArray<FWidgetSyncProperty> CaptureStateDelta();

	/**
	 * Apply received sync properties from server/other client.
	 * Called when replicated data arrives.
	 * @param Properties - The delta properties to apply
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Widget Sync")
	void ApplyStateDelta(const TArray<FWidgetSyncProperty>& Properties);

	// ============================================================================
	// SYNC CONFIGURATION
	// ============================================================================

	/**
	 * Get sync configuration for this widget.
	 * Determines sync interval, priority, spectating rules.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Widget Sync")
	FWidgetSyncConfig GetSyncConfig() const;
};
