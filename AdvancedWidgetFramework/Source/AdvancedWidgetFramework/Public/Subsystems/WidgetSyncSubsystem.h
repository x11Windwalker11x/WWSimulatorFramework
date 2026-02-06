// WidgetSyncSubsystem.h
// Multiplayer Widget Synchronization - replicated widget state management
// Layer: L2 (AdvancedWidgetFramework)

#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/AdvancedWidgetFramework/WidgetSyncData.h"
#include "Delegates/AdvancedWidgetFramework/WW_WidgetDelegates.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "WidgetSyncSubsystem.generated.h"

class UWidgetManagerBase;
class UWidgetSyncComponent;

/**
 * Internal tracking for a synced widget.
 */
USTRUCT()
struct FSyncedWidgetEntry
{
	GENERATED_BODY()

	/** The synced widget */
	UPROPERTY()
	TWeakObjectPtr<UUserWidget> Widget = nullptr;

	/** Sync configuration */
	FWidgetSyncConfig Config;

	/** Widget sync ID */
	FName WidgetSyncID;

	/** Widget type tag */
	FGameplayTag WidgetTypeTag;

	/** Time since last sync capture */
	float TimeSinceLastSync = 0.0f;

	/** Last sent sequence number */
	uint32 LastSequenceNumber = 0;

	/** Last captured state (for delta comparison) */
	TArray<FWidgetSyncProperty> LastCapturedState;
};

/**
 * UWidgetSyncSubsystem
 *
 * Manages multiplayer widget state synchronization.
 *
 * Features:
 * - Server-authoritative widget ownership
 * - Delta-only replication (only changed properties sent)
 * - Spectator binding (observe another player's widget state)
 * - Bandwidth optimization via priority and interval throttling
 * - Registers into WidgetManagerBase via delegate
 * - Delete AWF -> no sync, widgets work locally (graceful degradation)
 *
 * Architecture:
 * - Subsystem tracks registrations and captures deltas
 * - UWidgetSyncComponent (replicated) handles RPC transport
 * - Server validates all state changes before broadcasting
 * - No L2->L2 dependencies
 */
UCLASS()
class ADVANCEDWIDGETFRAMEWORK_API UWidgetSyncSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

	friend class UWidgetSyncComponent;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(ULocalPlayer* LocalPlayer) const override { return true; }

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return SyncedWidgets.Num() > 0; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual TStatId GetStatId() const override;

	// ============================================================================
	// WIDGET SYNC REGISTRATION
	// ============================================================================

	/**
	 * Register a widget for multiplayer synchronization.
	 * Widget must implement IReplicatedWidgetInterface.
	 * @param Widget - Widget to sync (must implement IReplicatedWidgetInterface)
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Sync")
	void RegisterForSync(UUserWidget* Widget);

	/**
	 * Unregister a widget from synchronization.
	 * @param Widget - Widget to stop syncing
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Sync")
	void UnregisterFromSync(UUserWidget* Widget);

	/** Check if a widget is registered for sync */
	UFUNCTION(BlueprintPure, Category = "Widget Sync")
	bool IsRegisteredForSync(UUserWidget* Widget) const;

	// ============================================================================
	// SPECTATOR BINDING
	// ============================================================================

	/**
	 * Bind as spectator to observe another player's widget.
	 * @param TargetPlayerID - Player to observe
	 * @param WidgetSyncID - Which widget to observe
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Sync|Spectator")
	void BindSpectator(int32 TargetPlayerID, FName WidgetSyncID);

	/**
	 * Unbind from spectating.
	 * @param WidgetSyncID - Which widget to stop observing
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Sync|Spectator")
	void UnbindSpectator(FName WidgetSyncID);

	/** Get all active spectator bindings for this player */
	UFUNCTION(BlueprintPure, Category = "Widget Sync|Spectator")
	TArray<FSpectatorBinding> GetActiveSpectatorBindings() const { return SpectatorBindings; }

	// ============================================================================
	// STATE APPLICATION (called by network transport)
	// ============================================================================

	/**
	 * Apply a received sync payload to the appropriate local widget.
	 * Called by UWidgetSyncComponent when server data arrives.
	 * @param Payload - The sync data to apply
	 */
	void ApplyReceivedPayload(const FWidgetSyncPayload& Payload);

	// ============================================================================
	// QUERIES
	// ============================================================================

	/** Get sync ID for a registered widget */
	UFUNCTION(BlueprintPure, Category = "Widget Sync")
	FName GetWidgetSyncID(UUserWidget* Widget) const;

	/** Get the sync component (for external access if needed) */
	UWidgetSyncComponent* GetSyncComponent() const { return SyncComponent.Get(); }

	// ============================================================================
	// DELEGATES
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Widget Sync|Events")
	FOnSpectatorBound OnSpectatorBound;

private:
	// ============================================================================
	// SYNC CAPTURE
	// ============================================================================

	/** Capture deltas for a single widget and send to server */
	void CaptureAndSendDelta(FSyncedWidgetEntry& Entry);

	/** Build a sync payload from captured deltas */
	FWidgetSyncPayload BuildPayload(FSyncedWidgetEntry& Entry, const TArray<FWidgetSyncProperty>& Delta);

	// ============================================================================
	// INTERCEPT HANDLER
	// ============================================================================

	/** Called by WidgetManagerBase when widget state changes */
	void HandleWidgetStateChanged(UUserWidget* Widget, FGameplayTag NewState);

	// ============================================================================
	// NETWORK TRANSPORT
	// ============================================================================

	/** Ensure the sync component exists on the PlayerController */
	void EnsureSyncComponent();

	/** Send a payload to the server via the sync component */
	void SendToServer(const FWidgetSyncPayload& Payload);

	// ============================================================================
	// INTERNAL
	// ============================================================================

	/** Find synced entry for a widget */
	FSyncedWidgetEntry* FindEntry(UUserWidget* Widget);
	const FSyncedWidgetEntry* FindEntry(UUserWidget* Widget) const;

	/** Find synced entry by sync ID */
	FSyncedWidgetEntry* FindEntryBySyncID(FName SyncID);

	/** Find local widget by sync ID (for applying received data) */
	UUserWidget* FindWidgetBySyncID(FName SyncID);

	/** Cached reference to WidgetManagerBase (Rule #41) */
	TWeakObjectPtr<UWidgetManagerBase> CachedWidgetManager;

	/** Replicated component for RPC transport */
	TWeakObjectPtr<UWidgetSyncComponent> SyncComponent;

	/** All synced widgets */
	UPROPERTY()
	TArray<FSyncedWidgetEntry> SyncedWidgets;

	/** Active spectator bindings */
	TArray<FSpectatorBinding> SpectatorBindings;

	/** Local player ID (cached) */
	int32 LocalPlayerID = -1;
};

// ============================================================================
// NETWORK TRANSPORT COMPONENT
// ============================================================================

/**
 * UWidgetSyncComponent
 *
 * Lightweight replicated component for RPC transport.
 * Attached to PlayerController by UWidgetSyncSubsystem.
 * Handles Server RPCs (client -> server) and Client RPCs (server -> client).
 *
 * Rule #13: Always add networking to new components.
 */
UCLASS(ClassGroup = (Custom))
class ADVANCEDWIDGETFRAMEWORK_API UWidgetSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWidgetSyncComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================================
	// SERVER RPCs
	// ============================================================================

	/** Client sends delta to server for validation and broadcast */
	UFUNCTION(Server, Reliable, Category = "Widget Sync")
	void Server_SendSyncPayload(const FWidgetSyncPayload& Payload);

	/** Client requests spectator binding */
	UFUNCTION(Server, Reliable, Category = "Widget Sync")
	void Server_RequestSpectatorBind(int32 TargetPlayerID, FName WidgetSyncID);

	/** Client requests spectator unbind */
	UFUNCTION(Server, Reliable, Category = "Widget Sync")
	void Server_RequestSpectatorUnbind(FName WidgetSyncID);

	// ============================================================================
	// CLIENT RPCs
	// ============================================================================

	/** Server broadcasts validated sync payload to relevant clients */
	UFUNCTION(Client, Reliable, Category = "Widget Sync")
	void Client_ReceiveSyncPayload(const FWidgetSyncPayload& Payload);

	/** Server confirms spectator binding */
	UFUNCTION(Client, Reliable, Category = "Widget Sync")
	void Client_SpectatorBindConfirmed(int32 TargetPlayerID, FName WidgetSyncID);

	/** Owning subsystem (set by UWidgetSyncSubsystem) */
	TWeakObjectPtr<UWidgetSyncSubsystem> OwningSubsystem;
};
