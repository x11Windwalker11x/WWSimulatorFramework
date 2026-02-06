// WidgetStateManager.h
// Widget State Machine Manager - manages widget transitions, interrupts, conflict resolution
// Layer: L2 (AdvancedWidgetFramework) - Registers into WidgetManagerBase (L0.5) via delegates

#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/AdvancedWidgetFramework/WidgetStateData.h"
#include "Delegates/AdvancedWidgetFramework/WW_WidgetDelegates.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "WidgetStateManager.generated.h"

class UWidgetManagerBase;
class UUserWidget;

/**
 * Runtime state tracking for a single widget's state machine.
 * Managed by UWidgetStateManager, not directly edited.
 */
USTRUCT(BlueprintType)
struct ADVANCEDWIDGETFRAMEWORK_API FWidgetStateMachineEntry
{
	GENERATED_BODY()

	/** The widget being tracked */
	UPROPERTY(BlueprintReadOnly, Category = "Widget State")
	TWeakObjectPtr<UUserWidget> Widget = nullptr;

	/** Current state tag (UI.Widget.State.*) */
	UPROPERTY(BlueprintReadOnly, Category = "Widget State")
	FGameplayTag CurrentState;

	/** Configuration for this widget */
	UPROPERTY(BlueprintReadOnly, Category = "Widget State")
	FWidgetStateConfig Config;

	/** Elapsed time in current state (for transitions) */
	UPROPERTY(BlueprintReadOnly, Category = "Widget State")
	float StateElapsed = 0.0f;

	/** Time spent visible (for auto-close timeout) */
	UPROPERTY(BlueprintReadOnly, Category = "Widget State")
	float VisibleElapsed = 0.0f;

	bool IsValid() const
	{
		return Widget.IsValid() && CurrentState.IsValid();
	}
};

/**
 * UWidgetStateManager
 *
 * Manages non-trivial widget transitions:
 *   Closed -> AnimatingIn -> Visible -> Paused -> AnimatingOut -> Closed
 *
 * Features:
 * - State machine per tracked widget
 * - Priority-based conflict resolution within categories
 * - Interrupt handling (Cancel / Queue / Pause)
 * - Transition animation timing
 * - Auto-close timeout
 * - Registers into UWidgetManagerBase via FWidgetTransitionInterceptDelegate
 * - Delete AWF -> graceful degradation to instant show/hide
 *
 * Architecture:
 * - L2 plugin (AWF) registers into L0.5 (MSB WidgetManagerBase)
 * - No L2->L2 dependencies
 * - Communication UP via delegates (Rule #7)
 * - Cached subsystem refs (Rule #41)
 */
UCLASS()
class ADVANCEDWIDGETFRAMEWORK_API UWidgetStateManager : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(ULocalPlayer* LocalPlayer) const override { return true; }

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return ManagedWidgets.Num() > 0; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual TStatId GetStatId() const override;

	// ============================================================================
	// WIDGET STATE MACHINE REGISTRATION
	// ============================================================================

	/**
	 * Register a widget for state machine management.
	 * Must be called before ShowWidget for transitions to apply.
	 * @param Widget - The widget to manage
	 * @param Config - State machine configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget State Manager")
	void RegisterWidgetStateMachine(UUserWidget* Widget, const FWidgetStateConfig& Config);

	/**
	 * Unregister a widget from state machine management.
	 * @param Widget - The widget to stop managing
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget State Manager")
	void UnregisterWidgetStateMachine(UUserWidget* Widget);

	/**
	 * Check if a widget is registered for state machine management.
	 * @param Widget - The widget to check
	 */
	UFUNCTION(BlueprintPure, Category = "Widget State Manager")
	bool IsWidgetManaged(UUserWidget* Widget) const;

	// ============================================================================
	// STATE QUERIES
	// ============================================================================

	/** Get current state tag for a managed widget */
	UFUNCTION(BlueprintPure, Category = "Widget State Manager")
	FGameplayTag GetWidgetState(UUserWidget* Widget) const;

	/** Get all widgets currently in a specific state */
	UFUNCTION(BlueprintPure, Category = "Widget State Manager")
	TArray<UUserWidget*> GetWidgetsInState(FGameplayTag StateTag) const;

	/** Get all visible widgets in a category (Visible or AnimatingIn) */
	UFUNCTION(BlueprintPure, Category = "Widget State Manager")
	TArray<UUserWidget*> GetVisibleWidgetsInCategory(FGameplayTag CategoryTag) const;

	/** Get the state machine entry for a widget (for debugging) */
	UFUNCTION(BlueprintPure, Category = "Widget State Manager")
	FWidgetStateMachineEntry GetWidgetStateMachineEntry(UUserWidget* Widget) const;

	// ============================================================================
	// MANUAL TRANSITIONS
	// ============================================================================

	/** Force a widget to a specific state (bypasses normal transition logic) */
	UFUNCTION(BlueprintCallable, Category = "Widget State Manager")
	void ForceWidgetState(UUserWidget* Widget, FGameplayTag NewState);

	/** Request to show a managed widget (triggers state machine) */
	UFUNCTION(BlueprintCallable, Category = "Widget State Manager")
	void RequestShow(UUserWidget* Widget);

	/** Request to hide a managed widget (triggers state machine) */
	UFUNCTION(BlueprintCallable, Category = "Widget State Manager")
	void RequestHide(UUserWidget* Widget);

	/** Resume a paused widget */
	UFUNCTION(BlueprintCallable, Category = "Widget State Manager")
	void ResumeWidget(UUserWidget* Widget);

	// ============================================================================
	// DELEGATES
	// ============================================================================

	/** Fired when any managed widget changes state */
	UPROPERTY(BlueprintAssignable, Category = "Widget State Manager|Events")
	FOnWidgetStateChanged OnWidgetStateChanged;

	/** Fired when a transition animation completes */
	UPROPERTY(BlueprintAssignable, Category = "Widget State Manager|Events")
	FOnWidgetTransitionComplete OnWidgetTransitionComplete;

private:
	// ============================================================================
	// INTERCEPT HANDLER (registered into WidgetManagerBase)
	// ============================================================================

	/**
	 * Called by WidgetManagerBase when ShowWidget/HideWidget is invoked.
	 * Returns true if intercepted (we handle transition), false for default.
	 */
	bool HandleTransitionIntercept(UUserWidget* Widget, bool bShow);

	// ============================================================================
	// STATE TRANSITIONS
	// ============================================================================

	/** Transition a widget to a new state */
	void TransitionTo(FWidgetStateMachineEntry& Entry, const FGameplayTag& NewState);

	/** Begin animate-in transition */
	void BeginAnimateIn(FWidgetStateMachineEntry& Entry);

	/** Complete animate-in, move to Visible */
	void CompleteAnimateIn(FWidgetStateMachineEntry& Entry);

	/** Begin animate-out transition */
	void BeginAnimateOut(FWidgetStateMachineEntry& Entry);

	/** Complete animate-out, move to Closed */
	void CompleteAnimateOut(FWidgetStateMachineEntry& Entry);

	/** Pause a widget (higher priority interrupted it) */
	void PauseWidget(FWidgetStateMachineEntry& Entry);

	// ============================================================================
	// CONFLICT RESOLUTION
	// ============================================================================

	/** Check for conflicts when a widget wants to become visible */
	void ResolveConflicts(FWidgetStateMachineEntry& IncomingEntry);

	/** Process the queued widgets - resume next in line after a widget closes */
	void ProcessQueue(FGameplayTag CategoryTag);

	// ============================================================================
	// INTERNAL
	// ============================================================================

	/** Find state machine entry for a widget (mutable) */
	FWidgetStateMachineEntry* FindEntry(UUserWidget* Widget);

	/** Find state machine entry for a widget (const) */
	const FWidgetStateMachineEntry* FindEntry(UUserWidget* Widget) const;

	/** Cached reference to WidgetManagerBase (Rule #41) */
	TWeakObjectPtr<UWidgetManagerBase> CachedWidgetManager;

	/** All managed widget state machines */
	UPROPERTY()
	TArray<FWidgetStateMachineEntry> ManagedWidgets;

	/** Queue of widgets waiting to be shown (per category) */
	TMap<FGameplayTag, TArray<TWeakObjectPtr<UUserWidget>>> WidgetQueue;

	/** Widgets pending destruction after animate-out */
	TArray<TWeakObjectPtr<UUserWidget>> PendingDestroy;
};
