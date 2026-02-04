// NumpadWidget.h
// Numpad UI widget for vault/safe code entry minigames

#pragma once

#include "CoreMinimal.h"
#include "UI/MiniGameHUD_Base.h"
#include "NumpadWidget.generated.h"

class UButton;
class UTextBlock;
class UBorder;

/**
 * UNumpadWidget
 * Visual numpad UI for sequence-based minigames (vault codes, safe combinations)
 * Connects to USequenceHandler via ProcessActionInput
 */
UCLASS()
class SIMULATORFRAMEWORK_API UNumpadWidget : public UMiniGameHUD_Base
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    // ============================================================================
    // MINIGAME HUD OVERRIDES
    // ============================================================================

    virtual void OnMiniGameStarted_Implementation() override;
    virtual void OnMiniGameEnded_Implementation(bool bSuccess) override;
    virtual void OnProgressUpdated_Implementation(float Progress, int32 CurrentStep, int32 TotalSteps) override;

    // ============================================================================
    // BOUND WIDGETS - NUMPAD BUTTONS
    // ============================================================================

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_0;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_1;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_2;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_3;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_4;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_5;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_6;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_7;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_8;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_9;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Clear;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Enter;

    // ============================================================================
    // BOUND WIDGETS - DISPLAY
    // ============================================================================

    /** Shows entered code as dots or digits */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_CodeDisplay;

    /** Shows error count */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_ErrorCount;

    /** Visual feedback border */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UBorder> Border_Feedback;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    /** Show actual digits vs dots for code display */
    UPROPERTY(EditDefaultsOnly, Category = "Numpad|Display")
    bool bShowDigits = false;

    /** Color for correct input feedback */
    UPROPERTY(EditDefaultsOnly, Category = "Numpad|Colors")
    FLinearColor CorrectColor = FLinearColor::Green;

    /** Color for incorrect input feedback */
    UPROPERTY(EditDefaultsOnly, Category = "Numpad|Colors")
    FLinearColor IncorrectColor = FLinearColor::Red;

    /** Color for neutral/default state */
    UPROPERTY(EditDefaultsOnly, Category = "Numpad|Colors")
    FLinearColor NeutralColor = FLinearColor::White;

    /** Duration of feedback flash (seconds) */
    UPROPERTY(EditDefaultsOnly, Category = "Numpad|Display")
    float FeedbackDuration = 0.3f;

    // ============================================================================
    // BLUEPRINT EVENTS
    // ============================================================================

    /** Called when a digit is pressed - override for custom effects */
    UFUNCTION(BlueprintNativeEvent, Category = "Numpad")
    void OnDigitPressed(int32 Digit);

    /** Called when clear is pressed */
    UFUNCTION(BlueprintNativeEvent, Category = "Numpad")
    void OnClearPressed();

    /** Called when enter is pressed */
    UFUNCTION(BlueprintNativeEvent, Category = "Numpad")
    void OnEnterPressed();

private:
    // ============================================================================
    // BUTTON CALLBACKS
    // ============================================================================

    UFUNCTION()
    void OnButton0Clicked();
    UFUNCTION()
    void OnButton1Clicked();
    UFUNCTION()
    void OnButton2Clicked();
    UFUNCTION()
    void OnButton3Clicked();
    UFUNCTION()
    void OnButton4Clicked();
    UFUNCTION()
    void OnButton5Clicked();
    UFUNCTION()
    void OnButton6Clicked();
    UFUNCTION()
    void OnButton7Clicked();
    UFUNCTION()
    void OnButton8Clicked();
    UFUNCTION()
    void OnButton9Clicked();
    UFUNCTION()
    void OnButtonClearClicked();
    UFUNCTION()
    void OnButtonEnterClicked();

    /** Process a numpad digit input */
    void ProcessDigitInput(int32 Digit);

    /** Update the code display text */
    void UpdateCodeDisplay();

    /** Show visual feedback for input result */
    void ShowFeedback(bool bCorrect);

    /** Clear the feedback state */
    void ClearFeedback();

    /** Handle for feedback timer */
    FTimerHandle FeedbackTimerHandle;
};
