#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "WW_WidgetDelegates.generated.h"

class UUserWidget;

// NOTE: FOnWidgetShown(UUserWidget*) and FOnWidgetHidden(UUserWidget*) are declared
// in WidgetManagerBase.h (L0.5) as the canonical versions. Do NOT duplicate here.

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWidgetRegistered,
	FGameplayTag, WidgetTag,
	UUserWidget*, Widget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnWidgetUnRegistered,
	FGameplayTag, WidgetTag);

// ============================================================================
// WIDGET STATE MACHINE DELEGATES (V2.13.5)
// ============================================================================

/**
 * Intercept delegate for widget show/hide transitions.
 * Single-cast: only one state manager should intercept at a time.
 * Return true = intercepted (state manager handles transition).
 * Return false / unbound = default immediate show/hide.
 */
DECLARE_DELEGATE_RetVal_TwoParams(bool, FWidgetTransitionInterceptDelegate,
	UUserWidget*, /* Widget */
	bool /* bShow - true=show, false=hide */);

/** Fired when a widget's state machine state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnWidgetStateChanged,
	UUserWidget*, Widget,
	FGameplayTag, OldState,
	FGameplayTag, NewState);

/** Fired when a widget transition animation completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWidgetTransitionComplete,
	UUserWidget*, Widget,
	FGameplayTag, NewState);

// ============================================================================
// WIDGET SYNC DELEGATES (V2.13.5)
// ============================================================================

/**
 * Intercept delegate for widget state sync.
 * If bound, UWidgetSyncSubsystem captures state changes for replication.
 * Single-cast: only one sync subsystem should intercept at a time.
 */
DECLARE_DELEGATE_TwoParams(FWidgetSyncInterceptDelegate,
	UUserWidget*, /* Widget */
	FGameplayTag /* NewState */);

/** Fired when a spectator binding is established */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSpectatorBound,
	int32, SpectatorPlayerID,
	int32, TargetPlayerID);

// ============================================================================
// DOCKABLE LAYOUT DELEGATES (V2.13.5)
// ============================================================================

/**
 * Intercept delegate for widget dock/undock requests.
 * If bound, UDockLayoutManager handles dock zone placement.
 * Single-cast: only one dock manager should intercept at a time.
 * Return true = intercepted (dock manager handles placement).
 * Return false / unbound = default behavior (no docking).
 */
DECLARE_DELEGATE_RetVal_TwoParams(bool, FWidgetDockInterceptDelegate,
	UUserWidget*, /* Widget */
	FGameplayTag /* TargetZoneTag */);

/** Fired when a widget is docked into a zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnWidgetDocked,
	UUserWidget*, Widget,
	FGameplayTag, ZoneTag,
	FName, DockableID);

/** Fired when a widget is undocked from a zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWidgetUndocked,
	UUserWidget*, Widget,
	FName, DockableID);

/** Fired when the dock layout changes (any dock/undock/rearrange) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDockLayoutChanged);

