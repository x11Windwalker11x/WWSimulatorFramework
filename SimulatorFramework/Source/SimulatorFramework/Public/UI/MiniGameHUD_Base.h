// MiniGameHUD_Base.h
#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "Lib/Data/SimulatorFramework/MiniGameData.h"
#include "GameplayTagContainer.h"
#include "MiniGameHUD_Base.generated.h"

class UMiniGameComponent;
class UMiniGameHandlerBase;

/**
 * Abstract base class for mini-game HUD widgets.
 * Provides lifecycle binding to MiniGameComponent and MiniGameHandlerBase.
 * Subclasses implement handler-specific UI (6 types: Manipulation, Lockpick, Sequence, Timing, Calibration, Temperature).
 *
 * Features:
 * - Auto-show on mini-game start, auto-hide on end/cancel
 * - Delegate binding to component and handler events
 * - Progress/state exposure to Blueprint for UI binding
 * - Cached subsystem refs per Golden Rule #41
 */
UCLASS(Abstract, Blueprintable)
class SIMULATORFRAMEWORK_API UMiniGameHUD_Base : public UManagedWidget_Master
{
	GENERATED_BODY()

public:
	UMiniGameHUD_Base(const FObjectInitializer& ObjectInitializer);

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Bind this HUD to a MiniGameComponent on the owning player's pawn.
	 * Called once after widget creation, typically by PlayerController or HUD class.
	 * @param InMiniGameComponent - The component to bind to
	 */
	UFUNCTION(BlueprintCallable, Category = "MiniGame|HUD")
	void BindToMiniGameComponent(UMiniGameComponent* InMiniGameComponent);

	/**
	 * Unbind from the current MiniGameComponent.
	 * Called automatically on destruct or manually for cleanup.
	 */
	UFUNCTION(BlueprintCallable, Category = "MiniGame|HUD")
	void UnbindFromMiniGameComponent();

	// ============================================================================
	// STATE QUERIES (BlueprintPure for UI binding)
	// ============================================================================

	/** Check if a mini-game is currently active */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD|State")
	bool IsMiniGameActive() const;

	/** Get current mini-game ID tag */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD|State")
	FGameplayTag GetCurrentMiniGameID() const;

	/** Get current mini-game type tag */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD|State")
	FGameplayTag GetCurrentMiniGameType() const;

	/** Get current progress (0.0 - 1.0) */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD|State")
	float GetCurrentProgress() const;

	/** Get elapsed time in seconds */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD|State")
	float GetElapsedTime() const;

	/** Get time remaining (if timeout configured, else 0) */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD|State")
	float GetTimeRemaining() const;

	/** Check if mini-game has timeout */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD|State")
	bool HasTimeout() const;

	/** Get current config data (for advanced UI) */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD|State")
	const FMiniGameData& GetCurrentConfig() const { return CachedConfig; }

	/** Get the active handler (for type-specific queries) */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD|State")
	UMiniGameHandlerBase* GetActiveHandler() const;

protected:
	// ============================================================================
	// LIFECYCLE OVERRIDES
	// ============================================================================

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ============================================================================
	// BLUEPRINT IMPLEMENTABLE EVENTS (Override in child classes/Blueprints)
	// ============================================================================

	/** Called when a mini-game starts - override to setup type-specific UI */
	UFUNCTION(BlueprintNativeEvent, Category = "MiniGame|HUD|Events")
	void OnMiniGameStarted(const FGameplayTag& MiniGameID, UMiniGameHandlerBase* Handler);

	/** Called when mini-game ends (success or failure) */
	UFUNCTION(BlueprintNativeEvent, Category = "MiniGame|HUD|Events")
	void OnMiniGameEnded(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus);

	/** Called when mini-game is cancelled */
	UFUNCTION(BlueprintNativeEvent, Category = "MiniGame|HUD|Events")
	void OnMiniGameCancelled(const FGameplayTag& MiniGameID, const FString& Reason);

	/** Called when handler reports progress on an objective */
	UFUNCTION(BlueprintNativeEvent, Category = "MiniGame|HUD|Events")
	void OnProgressUpdated(const FGameplayTag& ObjectiveTag, float Progress);

	/** Called every tick while mini-game is active - for animations/timers */
	UFUNCTION(BlueprintImplementableEvent, Category = "MiniGame|HUD|Events")
	void OnMiniGameTick(float DeltaTime);

	// ============================================================================
	// VIRTUAL METHODS FOR C++ SUBCLASSES
	// ============================================================================

	/** Called to check if this HUD should handle a specific mini-game type */
	virtual bool ShouldHandleMiniGameType(const FGameplayTag& TypeTag) const;

	/** Called after binding to component - cache any additional refs */
	virtual void OnBoundToComponent();

	/** Called before unbinding from component - cleanup refs */
	virtual void OnUnboundFromComponent();

	// ============================================================================
	// VISIBILITY CONTROL
	// ============================================================================

	/** Show the HUD widget */
	UFUNCTION(BlueprintCallable, Category = "MiniGame|HUD")
	void ShowHUD();

	/** Hide the HUD widget */
	UFUNCTION(BlueprintCallable, Category = "MiniGame|HUD")
	void HideHUD();

	/** Check if HUD is currently visible */
	UFUNCTION(BlueprintPure, Category = "MiniGame|HUD")
	bool IsHUDVisible() const { return bIsHUDVisible; }

	// ============================================================================
	// CACHED STATE (Performance - Golden Rule #41)
	// ============================================================================

	/** Cached reference to bound MiniGameComponent */
	UPROPERTY(BlueprintReadOnly, Category = "MiniGame|HUD|Cache")
	TWeakObjectPtr<UMiniGameComponent> CachedMiniGameComponent;

	/** Cached reference to active handler (updated on start/end) */
	UPROPERTY(BlueprintReadOnly, Category = "MiniGame|HUD|Cache")
	TWeakObjectPtr<UMiniGameHandlerBase> CachedHandler;

	/** Cached mini-game config for quick access */
	UPROPERTY(BlueprintReadOnly, Category = "MiniGame|HUD|Cache")
	FMiniGameData CachedConfig;

	/** Cached mini-game ID */
	UPROPERTY(BlueprintReadOnly, Category = "MiniGame|HUD|Cache")
	FGameplayTag CachedMiniGameID;

	/** Cached mini-game type */
	UPROPERTY(BlueprintReadOnly, Category = "MiniGame|HUD|Cache")
	FGameplayTag CachedMiniGameType;

	/** Is HUD currently visible */
	UPROPERTY(BlueprintReadOnly, Category = "MiniGame|HUD|State")
	bool bIsHUDVisible = false;

	/** Is mini-game currently active (cached for performance) */
	UPROPERTY(BlueprintReadOnly, Category = "MiniGame|HUD|State")
	bool bIsMiniGameActive = false;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Mini-game types this HUD handles (empty = all types) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MiniGame|HUD|Config")
	FGameplayTagContainer HandledMiniGameTypes;

	/** Auto-hide on mini-game end (default: true) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MiniGame|HUD|Config")
	bool bAutoHideOnEnd = true;

	/** Delay before auto-hide (for end animations) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MiniGame|HUD|Config", meta = (ClampMin = "0.0", EditCondition = "bAutoHideOnEnd"))
	float AutoHideDelay = 0.5f;

private:
	// ============================================================================
	// DELEGATE HANDLERS
	// ============================================================================

	/** Handler for MiniGameComponent::OnMiniGameStarted */
	UFUNCTION()
	void HandleMiniGameStarted(const FGameplayTag& MiniGameID, UMiniGameHandlerBase* Handler);

	/** Handler for MiniGameComponent::OnMiniGameEnded */
	UFUNCTION()
	void HandleMiniGameEnded(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus);

	/** Handler for MiniGameComponent::OnMiniGameCancelled */
	UFUNCTION()
	void HandleMiniGameCancelled(const FGameplayTag& MiniGameID, const FString& Reason);

	/** Handler for MiniGameHandlerBase::OnProgress */
	UFUNCTION()
	void HandleProgressUpdated(const FGameplayTag& ObjectiveTag, float Progress);

	// ============================================================================
	// INTERNAL
	// ============================================================================

	/** Bind to handler delegates */
	void BindToHandler(UMiniGameHandlerBase* Handler);

	/** Unbind from handler delegates */
	void UnbindFromHandler();

	/** Timer handle for auto-hide delay */
	FTimerHandle AutoHideTimerHandle;

	/** Perform delayed hide */
	void PerformDelayedHide();
};
