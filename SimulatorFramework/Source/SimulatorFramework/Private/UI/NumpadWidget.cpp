// NumpadWidget.cpp
// Numpad UI widget for vault/safe code entry minigames

#include "UI/NumpadWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/MiniGameComponent.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "TimerManager.h"

void UNumpadWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button callbacks
    if (Button_0) Button_0->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton0Clicked);
    if (Button_1) Button_1->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton1Clicked);
    if (Button_2) Button_2->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton2Clicked);
    if (Button_3) Button_3->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton3Clicked);
    if (Button_4) Button_4->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton4Clicked);
    if (Button_5) Button_5->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton5Clicked);
    if (Button_6) Button_6->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton6Clicked);
    if (Button_7) Button_7->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton7Clicked);
    if (Button_8) Button_8->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton8Clicked);
    if (Button_9) Button_9->OnClicked.AddDynamic(this, &UNumpadWidget::OnButton9Clicked);
    if (Button_Clear) Button_Clear->OnClicked.AddDynamic(this, &UNumpadWidget::OnButtonClearClicked);
    if (Button_Enter) Button_Enter->OnClicked.AddDynamic(this, &UNumpadWidget::OnButtonEnterClicked);
}

// ============================================================================
// MINIGAME HUD OVERRIDES
// ============================================================================

void UNumpadWidget::OnMiniGameStarted_Implementation()
{
    Super::OnMiniGameStarted_Implementation();

    // Reset display
    UpdateCodeDisplay();

    if (Text_ErrorCount)
    {
        Text_ErrorCount->SetText(FText::FromString(TEXT("Errors: 0")));
    }

    if (Border_Feedback)
    {
        Border_Feedback->SetBrushColor(NeutralColor);
    }
}

void UNumpadWidget::OnMiniGameEnded_Implementation(bool bSuccess)
{
    Super::OnMiniGameEnded_Implementation(bSuccess);

    // Final feedback
    if (Border_Feedback)
    {
        Border_Feedback->SetBrushColor(bSuccess ? CorrectColor : IncorrectColor);
    }

    // Clear any pending timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(FeedbackTimerHandle);
    }
}

void UNumpadWidget::OnProgressUpdated_Implementation(float Progress, int32 CurrentStep, int32 TotalSteps)
{
    Super::OnProgressUpdated_Implementation(Progress, CurrentStep, TotalSteps);

    UpdateCodeDisplay();

    // Update error count
    if (Text_ErrorCount)
    {
        int32 Errors = GetErrorCount();
        Text_ErrorCount->SetText(FText::FromString(FString::Printf(TEXT("Errors: %d"), Errors)));
    }

    // Show feedback based on last input
    ShowFeedback(WasLastInputCorrect());
}

// ============================================================================
// BUTTON CALLBACKS
// ============================================================================

void UNumpadWidget::OnButton0Clicked() { ProcessDigitInput(0); }
void UNumpadWidget::OnButton1Clicked() { ProcessDigitInput(1); }
void UNumpadWidget::OnButton2Clicked() { ProcessDigitInput(2); }
void UNumpadWidget::OnButton3Clicked() { ProcessDigitInput(3); }
void UNumpadWidget::OnButton4Clicked() { ProcessDigitInput(4); }
void UNumpadWidget::OnButton5Clicked() { ProcessDigitInput(5); }
void UNumpadWidget::OnButton6Clicked() { ProcessDigitInput(6); }
void UNumpadWidget::OnButton7Clicked() { ProcessDigitInput(7); }
void UNumpadWidget::OnButton8Clicked() { ProcessDigitInput(8); }
void UNumpadWidget::OnButton9Clicked() { ProcessDigitInput(9); }

void UNumpadWidget::OnButtonClearClicked()
{
    if (CachedMiniGameComponent.IsValid())
    {
        CachedMiniGameComponent->ProcessActionInput(FWWTagLibrary::Input_Numpad_Clear(), true);
    }
    OnClearPressed();
}

void UNumpadWidget::OnButtonEnterClicked()
{
    if (CachedMiniGameComponent.IsValid())
    {
        CachedMiniGameComponent->ProcessActionInput(FWWTagLibrary::Input_Numpad_Enter(), true);
    }
    OnEnterPressed();
}

// ============================================================================
// INPUT PROCESSING
// ============================================================================

void UNumpadWidget::ProcessDigitInput(int32 Digit)
{
    if (!CachedMiniGameComponent.IsValid())
    {
        return;
    }

    // Get appropriate tag for digit
    FGameplayTag DigitTag;
    switch (Digit)
    {
        case 0: DigitTag = FWWTagLibrary::Input_Numpad_0(); break;
        case 1: DigitTag = FWWTagLibrary::Input_Numpad_1(); break;
        case 2: DigitTag = FWWTagLibrary::Input_Numpad_2(); break;
        case 3: DigitTag = FWWTagLibrary::Input_Numpad_3(); break;
        case 4: DigitTag = FWWTagLibrary::Input_Numpad_4(); break;
        case 5: DigitTag = FWWTagLibrary::Input_Numpad_5(); break;
        case 6: DigitTag = FWWTagLibrary::Input_Numpad_6(); break;
        case 7: DigitTag = FWWTagLibrary::Input_Numpad_7(); break;
        case 8: DigitTag = FWWTagLibrary::Input_Numpad_8(); break;
        case 9: DigitTag = FWWTagLibrary::Input_Numpad_9(); break;
        default: return;
    }

    CachedMiniGameComponent->ProcessActionInput(DigitTag, true);
    OnDigitPressed(Digit);
}

// ============================================================================
// DISPLAY UPDATES
// ============================================================================

void UNumpadWidget::UpdateCodeDisplay()
{
    if (!Text_CodeDisplay)
    {
        return;
    }

    TArray<FGameplayTag> CurrentSequence = GetCurrentSequence();
    FString DisplayStr;

    if (bShowDigits)
    {
        // Show actual digits
        for (const FGameplayTag& Tag : CurrentSequence)
        {
            FString TagStr = Tag.GetTagName().ToString();
            // Extract last character (the digit)
            if (TagStr.Len() > 0)
            {
                DisplayStr += TagStr.Right(1);
            }
        }
    }
    else
    {
        // Show dots/asterisks for security
        for (int32 i = 0; i < CurrentSequence.Num(); ++i)
        {
            DisplayStr += TEXT("*");
        }
    }

    Text_CodeDisplay->SetText(FText::FromString(DisplayStr));
}

void UNumpadWidget::ShowFeedback(bool bCorrect)
{
    if (!Border_Feedback || !GetWorld())
    {
        return;
    }

    Border_Feedback->SetBrushColor(bCorrect ? CorrectColor : IncorrectColor);

    // Clear after duration
    GetWorld()->GetTimerManager().ClearTimer(FeedbackTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(
        FeedbackTimerHandle,
        this,
        &UNumpadWidget::ClearFeedback,
        FeedbackDuration,
        false
    );
}

void UNumpadWidget::ClearFeedback()
{
    if (Border_Feedback)
    {
        Border_Feedback->SetBrushColor(NeutralColor);
    }
}

// ============================================================================
// BLUEPRINT EVENTS
// ============================================================================

void UNumpadWidget::OnDigitPressed_Implementation(int32 Digit)
{
    // Default: no-op, override in Blueprint for custom effects (sounds, animations)
}

void UNumpadWidget::OnClearPressed_Implementation()
{
    // Default: no-op
}

void UNumpadWidget::OnEnterPressed_Implementation()
{
    // Default: no-op
}
