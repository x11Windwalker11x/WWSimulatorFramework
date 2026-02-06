// TemperatureGauge.h
// Temperature control UI for cooking/smithing minigames

#pragma once

#include "CoreMinimal.h"
#include "UI/MiniGameHUD_Base.h"
#include "TemperatureGauge.generated.h"

class UProgressBar;
class UImage;
class UTextBlock;
class UBorder;
class UTemperatureHandler;

/**
 * UTemperatureGauge
 * Visual thermometer/gauge for temperature-based minigames.
 * Displays current temperature, optimal zone, danger zones, and hold progress.
 * Connects to UTemperatureHandler for state.
 */
UCLASS()
class SIMULATORFRAMEWORK_API UTemperatureGauge : public UMiniGameHUD_Base
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
	// BOUND WIDGETS - GAUGE DISPLAY
	// ============================================================================

	/** Main temperature bar (0-1 normalized to DangerMin..DangerMax range) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> Bar_Temperature;

	/** Progress bar for time in optimal zone */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> Bar_HoldProgress;

	/** Heat input indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> Bar_HeatInput;

	// ============================================================================
	// BOUND WIDGETS - TEXT DISPLAY
	// ============================================================================

	/** Current temperature value */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Temperature;

	/** Zone status (e.g., "Optimal", "Too Hot!", "Too Cold!") */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ZoneStatus;

	/** Hold progress text (e.g., "2.1s / 3.0s") */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_HoldProgress;

	/** Visual feedback border */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_Feedback;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Color for optimal temperature zone */
	UPROPERTY(EditDefaultsOnly, Category = "Temperature|Colors")
	FLinearColor OptimalColor = FLinearColor::Green;

	/** Color for normal (between danger and optimal) */
	UPROPERTY(EditDefaultsOnly, Category = "Temperature|Colors")
	FLinearColor NormalColor = FLinearColor::White;

	/** Color for danger zone (too hot) */
	UPROPERTY(EditDefaultsOnly, Category = "Temperature|Colors")
	FLinearColor DangerHotColor = FLinearColor::Red;

	/** Color for danger zone (too cold) */
	UPROPERTY(EditDefaultsOnly, Category = "Temperature|Colors")
	FLinearColor DangerColdColor = FLinearColor(0.3f, 0.5f, 1.0f);

	/** Color when item is ruined */
	UPROPERTY(EditDefaultsOnly, Category = "Temperature|Colors")
	FLinearColor RuinedColor = FLinearColor(0.3f, 0.0f, 0.0f);

	/** Duration of feedback flash */
	UPROPERTY(EditDefaultsOnly, Category = "Temperature|Feedback")
	float FeedbackDuration = 0.3f;

	// ============================================================================
	// BLUEPRINT EVENTS
	// ============================================================================

	/** Called when entering optimal zone */
	UFUNCTION(BlueprintNativeEvent, Category = "Temperature")
	void OnEnteredOptimalZone();

	/** Called when leaving optimal zone */
	UFUNCTION(BlueprintNativeEvent, Category = "Temperature")
	void OnLeftOptimalZone();

	/** Called when item is ruined */
	UFUNCTION(BlueprintNativeEvent, Category = "Temperature")
	void OnItemRuined(const FString& Reason);

private:
	/** Cached temperature handler reference */
	TWeakObjectPtr<UTemperatureHandler> CachedTemperatureHandler;

	/** Previous zone state for edge detection */
	bool bWasInOptimalZone = false;

	/** Timer handle for feedback */
	FTimerHandle FeedbackTimerHandle;

	/** Normalize temperature to 0-1 range based on danger bounds */
	float NormalizeTemperature(float Temp) const;

	/** Get zone color for current temperature */
	FLinearColor GetZoneColor() const;

	/** Update all visual elements */
	void UpdateGaugeDisplay();

	/** Show visual feedback */
	void ShowFeedback(const FLinearColor& Color);

	/** Clear feedback state */
	void ClearFeedback();

	// === DELEGATE HANDLERS ===

	UFUNCTION()
	void HandleZoneEntered(bool bOptimalZone);

	UFUNCTION()
	void HandleItemRuined(const FString& Reason);
};
