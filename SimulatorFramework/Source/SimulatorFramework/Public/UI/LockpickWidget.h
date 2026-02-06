// LockpickWidget.h
// Sweetspot-based lockpick UI for lock minigames

#pragma once

#include "CoreMinimal.h"
#include "UI/MiniGameHUD_Base.h"
#include "LockpickWidget.generated.h"

class UProgressBar;
class UImage;
class UTextBlock;
class UBorder;
class USweetspotHandler;

/**
 * ULockpickWidget
 * Visual lockpicking UI for sweetspot-based minigames (Skyrim/Fallout style).
 * Displays pick position, feedback intensity, pin progress, and attempts remaining.
 * Connects to USweetspotHandler for state.
 */
UCLASS()
class SIMULATORFRAMEWORK_API ULockpickWidget : public UMiniGameHUD_Base
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ============================================================================
	// MINIGAME HUD OVERRIDES
	// ============================================================================

	virtual void OnMiniGameStarted_Implementation(const FGameplayTag& MiniGameID, UMiniGameHandlerBase* Handler) override;
	virtual void OnMiniGameEnded_Implementation(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus) override;
	virtual void OnProgressUpdated_Implementation(const FGameplayTag& ObjectiveTag, float Progress) override;

	// ============================================================================
	// BOUND WIDGETS - PICK DISPLAY
	// ============================================================================

	/** Visual indicator showing pick position on the dial (0-1) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_PickIndicator;

	/** Sweetspot zone highlight */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_SweetspotZone;

	/** Feedback intensity bar (vibration/resistance) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> Bar_Feedback;

	/** Tension damage indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> Bar_TensionDamage;

	// ============================================================================
	// BOUND WIDGETS - STATUS DISPLAY
	// ============================================================================

	/** Pin progress text (e.g., "Pin 2/5") */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_PinProgress;

	/** Attempts remaining (e.g., "Picks: 3") */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Attempts;

	/** Status text (e.g., "Tensioning...", "Pin Unlocked!") */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Status;

	/** Visual feedback border */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_Feedback;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Dial width for pick position (pixels) */
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick|Layout")
	float DialWidth = 300.0f;

	/** Color when in sweetspot */
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick|Colors")
	FLinearColor InZoneColor = FLinearColor::Green;

	/** Color when outside sweetspot */
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick|Colors")
	FLinearColor OutZoneColor = FLinearColor::White;

	/** Color for tension warning */
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick|Colors")
	FLinearColor TensionColor = FLinearColor(1.0f, 0.5f, 0.0f);

	/** Color for pick broken feedback */
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick|Colors")
	FLinearColor BrokenColor = FLinearColor::Red;

	/** Color for pin unlocked feedback */
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick|Colors")
	FLinearColor UnlockedColor = FLinearColor(1.0f, 0.84f, 0.0f);

	/** Duration of feedback flash */
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick|Feedback")
	float FeedbackDuration = 0.4f;

	// ============================================================================
	// BLUEPRINT EVENTS
	// ============================================================================

	/** Called when a pin is unlocked - override for custom effects */
	UFUNCTION(BlueprintNativeEvent, Category = "Lockpick")
	void OnPinUnlocked(int32 PinIndex);

	/** Called when the pick breaks */
	UFUNCTION(BlueprintNativeEvent, Category = "Lockpick")
	void OnPickBroken(int32 AttemptsRemaining);

	/** Called when entering/leaving sweetspot zone */
	UFUNCTION(BlueprintNativeEvent, Category = "Lockpick")
	void OnSweetspotFeedback(float Intensity, bool bInZone);

private:
	/** Cached sweetspot handler reference */
	TWeakObjectPtr<USweetspotHandler> CachedSweetspotHandler;

	/** Timer handle for feedback */
	FTimerHandle FeedbackTimerHandle;

	/** Update pick indicator position */
	void UpdatePickPosition(float Position);

	/** Update feedback intensity bar */
	void UpdateFeedbackBar(float Intensity);

	/** Update pin progress display */
	void UpdatePinDisplay(int32 CurrentPin, int32 TotalPins);

	/** Update attempts display */
	void UpdateAttemptsDisplay(int32 Remaining);

	/** Show visual feedback for events */
	void ShowFeedback(const FLinearColor& Color);

	/** Clear feedback state */
	void ClearFeedback();

	// === DELEGATE HANDLERS ===

	UFUNCTION()
	void HandleSweetspotFeedback(float Intensity, bool bInZone);

	UFUNCTION()
	void HandlePinUnlocked(int32 PinIndex);

	UFUNCTION()
	void HandlePickBroken(int32 AttemptsRemaining);
};
