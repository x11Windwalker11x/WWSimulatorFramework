// LockpickWidget.cpp
// Sweetspot-based lockpick UI for lock minigames

#include "UI/LockpickWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Subsystems/SweetspotHandler.h"
#include "TimerManager.h"

void ULockpickWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize displays
	if (Bar_Feedback)
	{
		Bar_Feedback->SetPercent(0.0f);
	}
	if (Bar_TensionDamage)
	{
		Bar_TensionDamage->SetPercent(0.0f);
	}
}

void ULockpickWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedSweetspotHandler.IsValid())
	{
		return;
	}

	// Update pick position every frame
	UpdatePickPosition(CachedSweetspotHandler->GetCurrentPosition());

	// Update feedback intensity
	UpdateFeedbackBar(CachedSweetspotHandler->GetFeedbackIntensity());

	// Update pin progress
	UpdatePinDisplay(CachedSweetspotHandler->GetCurrentPinIndex(), CachedSweetspotHandler->GetTotalPins());

	// Update attempts
	UpdateAttemptsDisplay(CachedSweetspotHandler->GetAttemptsRemaining());

	// Update tension indicator color
	if (Image_PickIndicator)
	{
		if (CachedSweetspotHandler->IsTensioned())
		{
			Image_PickIndicator->SetColorAndOpacity(TensionColor);
		}
		else
		{
			Image_PickIndicator->SetColorAndOpacity(
				CachedSweetspotHandler->IsInSweetspot() ? InZoneColor : OutZoneColor
			);
		}
	}

	// Update status text
	if (Text_Status)
	{
		if (CachedSweetspotHandler->IsTensioned())
		{
			Text_Status->SetText(FText::FromString(TEXT("Tensioning...")));
			Text_Status->SetColorAndOpacity(FSlateColor(TensionColor));
		}
		else if (CachedSweetspotHandler->IsInSweetspot())
		{
			Text_Status->SetText(FText::FromString(TEXT("In Position")));
			Text_Status->SetColorAndOpacity(FSlateColor(InZoneColor));
		}
		else
		{
			Text_Status->SetText(FText::FromString(TEXT("Find Sweetspot")));
			Text_Status->SetColorAndOpacity(FSlateColor(OutZoneColor));
		}
	}
}

// ============================================================================
// MINIGAME HUD OVERRIDES
// ============================================================================

void ULockpickWidget::OnMiniGameStarted_Implementation(const FGameplayTag& MiniGameID, UMiniGameHandlerBase* Handler)
{
	Super::OnMiniGameStarted_Implementation(MiniGameID, Handler);

	// Cache sweetspot handler
	if (CachedHandler.IsValid())
	{
		CachedSweetspotHandler = Cast<USweetspotHandler>(CachedHandler.Get());
	}

	if (CachedSweetspotHandler.IsValid())
	{
		// Bind to handler-specific delegates
		CachedSweetspotHandler->OnFeedbackChanged.AddDynamic(this, &ULockpickWidget::HandleSweetspotFeedback);
		CachedSweetspotHandler->OnPinUnlocked.AddDynamic(this, &ULockpickWidget::HandlePinUnlocked);
		CachedSweetspotHandler->OnPickBroken.AddDynamic(this, &ULockpickWidget::HandlePickBroken);

		// Initial display
		UpdatePinDisplay(CachedSweetspotHandler->GetCurrentPinIndex(), CachedSweetspotHandler->GetTotalPins());
		UpdateAttemptsDisplay(CachedSweetspotHandler->GetAttemptsRemaining());
	}

	// Reset feedback
	if (Border_Feedback)
	{
		Border_Feedback->SetBrushColor(FLinearColor::Transparent);
	}
}

void ULockpickWidget::OnMiniGameEnded_Implementation(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus)
{
	Super::OnMiniGameEnded_Implementation(MiniGameID, bSuccess, bBonus);

	// Unbind from handler delegates
	if (CachedSweetspotHandler.IsValid())
	{
		CachedSweetspotHandler->OnFeedbackChanged.RemoveDynamic(this, &ULockpickWidget::HandleSweetspotFeedback);
		CachedSweetspotHandler->OnPinUnlocked.RemoveDynamic(this, &ULockpickWidget::HandlePinUnlocked);
		CachedSweetspotHandler->OnPickBroken.RemoveDynamic(this, &ULockpickWidget::HandlePickBroken);
	}

	// Final feedback
	ShowFeedback(bSuccess ? UnlockedColor : BrokenColor);

	// Update status
	if (Text_Status)
	{
		Text_Status->SetText(FText::FromString(bSuccess ? TEXT("Unlocked!") : TEXT("Failed")));
		Text_Status->SetColorAndOpacity(FSlateColor(bSuccess ? UnlockedColor : BrokenColor));
	}

	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FeedbackTimerHandle);
	}

	CachedSweetspotHandler.Reset();
}

void ULockpickWidget::OnProgressUpdated_Implementation(const FGameplayTag& ObjectiveTag, float Progress)
{
	Super::OnProgressUpdated_Implementation(ObjectiveTag, Progress);
}

// ============================================================================
// DISPLAY UPDATES
// ============================================================================

void ULockpickWidget::UpdatePickPosition(float Position)
{
	if (!Image_PickIndicator)
	{
		return;
	}

	float XPosition = Position * DialWidth;
	Image_PickIndicator->SetRenderTranslation(FVector2D(XPosition, 0.0f));
}

void ULockpickWidget::UpdateFeedbackBar(float Intensity)
{
	if (Bar_Feedback)
	{
		Bar_Feedback->SetPercent(Intensity);
	}
}

void ULockpickWidget::UpdatePinDisplay(int32 CurrentPin, int32 TotalPins)
{
	if (Text_PinProgress)
	{
		Text_PinProgress->SetText(FText::FromString(
			FString::Printf(TEXT("Pin %d / %d"), CurrentPin + 1, TotalPins)
		));
	}
}

void ULockpickWidget::UpdateAttemptsDisplay(int32 Remaining)
{
	if (Text_Attempts)
	{
		Text_Attempts->SetText(FText::FromString(
			FString::Printf(TEXT("Picks: %d"), Remaining)
		));

		// Warning color when low
		if (Remaining <= 1)
		{
			Text_Attempts->SetColorAndOpacity(FSlateColor(BrokenColor));
		}
		else
		{
			Text_Attempts->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}
}

void ULockpickWidget::ShowFeedback(const FLinearColor& Color)
{
	if (!GetWorld())
	{
		return;
	}

	if (Border_Feedback)
	{
		Border_Feedback->SetBrushColor(Color);
	}

	GetWorld()->GetTimerManager().ClearTimer(FeedbackTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		FeedbackTimerHandle,
		this,
		&ULockpickWidget::ClearFeedback,
		FeedbackDuration,
		false
	);
}

void ULockpickWidget::ClearFeedback()
{
	if (Border_Feedback)
	{
		Border_Feedback->SetBrushColor(FLinearColor::Transparent);
	}
}

// ============================================================================
// DELEGATE HANDLERS
// ============================================================================

void ULockpickWidget::HandleSweetspotFeedback(float Intensity, bool bInZone)
{
	OnSweetspotFeedback(Intensity, bInZone);
}

void ULockpickWidget::HandlePinUnlocked(int32 PinIndex)
{
	ShowFeedback(UnlockedColor);

	if (Text_Status)
	{
		Text_Status->SetText(FText::FromString(TEXT("Pin Unlocked!")));
		Text_Status->SetColorAndOpacity(FSlateColor(UnlockedColor));
	}

	OnPinUnlocked(PinIndex);
}

void ULockpickWidget::HandlePickBroken(int32 AttemptsRemaining)
{
	ShowFeedback(BrokenColor);

	if (Text_Status)
	{
		Text_Status->SetText(FText::FromString(TEXT("Pick Broken!")));
		Text_Status->SetColorAndOpacity(FSlateColor(BrokenColor));
	}

	OnPickBroken(AttemptsRemaining);
}

// ============================================================================
// BLUEPRINT EVENTS
// ============================================================================

void ULockpickWidget::OnPinUnlocked_Implementation(int32 PinIndex)
{
	// Default: no-op - override for custom effects (sounds, animations)
}

void ULockpickWidget::OnPickBroken_Implementation(int32 AttemptsRemaining)
{
	// Default: no-op
}

void ULockpickWidget::OnSweetspotFeedback_Implementation(float Intensity, bool bInZone)
{
	// Default: no-op
}
