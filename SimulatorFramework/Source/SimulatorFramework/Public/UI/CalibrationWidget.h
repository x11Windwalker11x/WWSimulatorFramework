// CalibrationWidget.h
// Precision meter widget for calibration minigames

#pragma once

#include "CoreMinimal.h"
#include "UI/MiniGameHUD_Base.h"
#include "CalibrationWidget.generated.h"

class UProgressBar;
class UImage;
class UTextBlock;
class UBorder;
class UCalibrationHandler;

/**
 * UCalibrationWidget
 * Visual precision meter for calibration-based minigames.
 * Displays current value vs target, tolerance zone, hold progress, and lock state.
 * Connects to UCalibrationHandler for state.
 */
UCLASS()
class SIMULATORFRAMEWORK_API UCalibrationWidget : public UMiniGameHUD_Base
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
	// BOUND WIDGETS - METER DISPLAY
	// ============================================================================

	/** Moving indicator for current value */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_ValueIndicator;

	/** Target position indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_TargetIndicator;

	/** Tolerance zone highlight around target */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_ToleranceZone;

	/** Hold progress bar (fill while locked on target) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> Bar_HoldProgress;

	// ============================================================================
	// BOUND WIDGETS - TEXT DISPLAY
	// ============================================================================

	/** Lock status text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_LockStatus;

	/** Hold progress text (e.g., "1.5s / 2.0s") */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_HoldProgress;

	/** Distance from target text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Distance;

	/** Visual feedback border */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_Feedback;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Meter width for positioning (pixels) */
	UPROPERTY(EditDefaultsOnly, Category = "Calibration|Layout")
	float MeterWidth = 400.0f;

	/** Color when locked on target */
	UPROPERTY(EditDefaultsOnly, Category = "Calibration|Colors")
	FLinearColor LockedColor = FLinearColor::Green;

	/** Color when not locked */
	UPROPERTY(EditDefaultsOnly, Category = "Calibration|Colors")
	FLinearColor UnlockedColor = FLinearColor::White;

	/** Color for target indicator */
	UPROPERTY(EditDefaultsOnly, Category = "Calibration|Colors")
	FLinearColor TargetColor = FLinearColor(1.0f, 0.84f, 0.0f);

	/** Color when lock is lost */
	UPROPERTY(EditDefaultsOnly, Category = "Calibration|Colors")
	FLinearColor LostColor = FLinearColor::Red;

	/** Color for successful completion */
	UPROPERTY(EditDefaultsOnly, Category = "Calibration|Colors")
	FLinearColor SuccessColor = FLinearColor(0.0f, 1.0f, 0.5f);

	/** Duration of feedback flash */
	UPROPERTY(EditDefaultsOnly, Category = "Calibration|Feedback")
	float FeedbackDuration = 0.3f;

	// ============================================================================
	// BLUEPRINT EVENTS
	// ============================================================================

	/** Called when lock is gained */
	UFUNCTION(BlueprintNativeEvent, Category = "Calibration")
	void OnLockGained();

	/** Called when lock is lost */
	UFUNCTION(BlueprintNativeEvent, Category = "Calibration")
	void OnLockLost();

private:
	/** Cached calibration handler reference */
	TWeakObjectPtr<UCalibrationHandler> CachedCalibrationHandler;

	/** Previous lock state for edge detection */
	bool bWasLockedOn = false;

	/** Timer handle for feedback */
	FTimerHandle FeedbackTimerHandle;

	/** Update value indicator position */
	void UpdateValuePosition(float Value);

	/** Update target indicator position */
	void UpdateTargetPosition(float Target);

	/** Update tolerance zone display */
	void UpdateToleranceZone(float Target, float Tolerance);

	/** Update hold progress display */
	void UpdateHoldDisplay(float TimeHeld, float Required);

	/** Show visual feedback */
	void ShowFeedback(const FLinearColor& Color);

	/** Clear feedback state */
	void ClearFeedback();

	// === DELEGATE HANDLERS ===

	UFUNCTION()
	void HandleCalibrationLocked();

	UFUNCTION()
	void HandleCalibrationLost();
};
