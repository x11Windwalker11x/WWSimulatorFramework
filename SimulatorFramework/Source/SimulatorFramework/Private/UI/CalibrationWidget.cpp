// CalibrationWidget.cpp
// Precision meter widget for calibration minigames

#include "UI/CalibrationWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Subsystems/CalibrationHandler.h"
#include "TimerManager.h"

void UCalibrationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Bar_HoldProgress)
	{
		Bar_HoldProgress->SetPercent(0.0f);
	}
}

void UCalibrationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedCalibrationHandler.IsValid())
	{
		return;
	}

	// Update value indicator position
	UpdateValuePosition(CachedCalibrationHandler->GetCurrentValue());

	// Update target indicator position
	UpdateTargetPosition(CachedCalibrationHandler->GetTargetValue());

	// Update tolerance zone
	UpdateToleranceZone(CachedCalibrationHandler->GetTargetValue(), CachedCalibrationHandler->GetCurrentTolerance());

	// Update hold progress
	UpdateHoldDisplay(CachedCalibrationHandler->GetTimeHeld(), CachedCalibrationHandler->GetRequiredHoldDuration());

	// Update value indicator color based on lock state
	bool bLockedOn = CachedCalibrationHandler->IsLockedOn();
	if (Image_ValueIndicator)
	{
		Image_ValueIndicator->SetColorAndOpacity(bLockedOn ? LockedColor : UnlockedColor);
	}

	// Edge detection for lock state
	if (bLockedOn && !bWasLockedOn)
	{
		OnLockGained();
	}
	else if (!bLockedOn && bWasLockedOn)
	{
		OnLockLost();
	}
	bWasLockedOn = bLockedOn;

	// Update distance text
	if (Text_Distance)
	{
		float Distance = CachedCalibrationHandler->GetDistanceFromTarget();
		Text_Distance->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f%%"), Distance * 100.0f)
		));

		FLinearColor DistColor = FMath::Lerp(LockedColor, UnlockedColor, Distance);
		Text_Distance->SetColorAndOpacity(FSlateColor(DistColor));
	}

	// Update lock status text
	if (Text_LockStatus)
	{
		if (bLockedOn)
		{
			Text_LockStatus->SetText(FText::FromString(TEXT("LOCKED")));
			Text_LockStatus->SetColorAndOpacity(FSlateColor(LockedColor));
		}
		else
		{
			Text_LockStatus->SetText(FText::FromString(TEXT("Calibrating...")));
			Text_LockStatus->SetColorAndOpacity(FSlateColor(UnlockedColor));
		}
	}
}

// ============================================================================
// MINIGAME HUD OVERRIDES
// ============================================================================

void UCalibrationWidget::OnMiniGameStarted_Implementation(const FGameplayTag& MiniGameID, UMiniGameHandlerBase* Handler)
{
	Super::OnMiniGameStarted_Implementation(MiniGameID, Handler);

	// Cache calibration handler
	if (CachedHandler.IsValid())
	{
		CachedCalibrationHandler = Cast<UCalibrationHandler>(CachedHandler.Get());
	}

	if (CachedCalibrationHandler.IsValid())
	{
		// Bind to handler-specific delegates
		CachedCalibrationHandler->OnLocked.AddDynamic(this, &UCalibrationWidget::HandleCalibrationLocked);
		CachedCalibrationHandler->OnLost.AddDynamic(this, &UCalibrationWidget::HandleCalibrationLost);

		// Set target indicator
		if (Image_TargetIndicator)
		{
			Image_TargetIndicator->SetColorAndOpacity(TargetColor);
		}
	}

	// Reset state
	bWasLockedOn = false;

	if (Border_Feedback)
	{
		Border_Feedback->SetBrushColor(FLinearColor::Transparent);
	}

	if (Bar_HoldProgress)
	{
		Bar_HoldProgress->SetPercent(0.0f);
	}
}

void UCalibrationWidget::OnMiniGameEnded_Implementation(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus)
{
	Super::OnMiniGameEnded_Implementation(MiniGameID, bSuccess, bBonus);

	// Unbind from handler delegates
	if (CachedCalibrationHandler.IsValid())
	{
		CachedCalibrationHandler->OnLocked.RemoveDynamic(this, &UCalibrationWidget::HandleCalibrationLocked);
		CachedCalibrationHandler->OnLost.RemoveDynamic(this, &UCalibrationWidget::HandleCalibrationLost);
	}

	// Final feedback
	ShowFeedback(bSuccess ? SuccessColor : LostColor);

	if (Text_LockStatus)
	{
		Text_LockStatus->SetText(FText::FromString(bSuccess ? TEXT("Calibrated!") : TEXT("Failed")));
		Text_LockStatus->SetColorAndOpacity(FSlateColor(bSuccess ? SuccessColor : LostColor));
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FeedbackTimerHandle);
	}

	CachedCalibrationHandler.Reset();
}

void UCalibrationWidget::OnProgressUpdated_Implementation(const FGameplayTag& ObjectiveTag, float Progress)
{
	Super::OnProgressUpdated_Implementation(ObjectiveTag, Progress);
}

// ============================================================================
// DISPLAY UPDATES
// ============================================================================

void UCalibrationWidget::UpdateValuePosition(float Value)
{
	if (!Image_ValueIndicator)
	{
		return;
	}

	float XPosition = Value * MeterWidth;
	Image_ValueIndicator->SetRenderTranslation(FVector2D(XPosition, 0.0f));
}

void UCalibrationWidget::UpdateTargetPosition(float Target)
{
	if (!Image_TargetIndicator)
	{
		return;
	}

	float XPosition = Target * MeterWidth;
	Image_TargetIndicator->SetRenderTranslation(FVector2D(XPosition, 0.0f));
}

void UCalibrationWidget::UpdateToleranceZone(float Target, float Tolerance)
{
	if (!Border_ToleranceZone)
	{
		return;
	}

	float StartX = (Target - Tolerance) * MeterWidth;
	float ZoneWidth = Tolerance * 2.0f * MeterWidth;

	Border_ToleranceZone->SetRenderTranslation(FVector2D(StartX, 0.0f));

	// Color based on tolerance size (shrinks over time = more yellow/red)
	float ToleranceNormalized = FMath::Clamp(Tolerance / 0.2f, 0.0f, 1.0f);
	FLinearColor ZoneColor = FMath::Lerp(LostColor, LockedColor, ToleranceNormalized);
	ZoneColor.A = 0.3f;
	Border_ToleranceZone->SetBrushColor(ZoneColor);
}

void UCalibrationWidget::UpdateHoldDisplay(float TimeHeld, float Required)
{
	float HoldPercent = (Required > 0.0f) ? FMath::Clamp(TimeHeld / Required, 0.0f, 1.0f) : 0.0f;

	if (Bar_HoldProgress)
	{
		Bar_HoldProgress->SetPercent(HoldPercent);
		Bar_HoldProgress->SetFillColorAndOpacity(LockedColor);
	}

	if (Text_HoldProgress)
	{
		Text_HoldProgress->SetText(FText::FromString(
			FString::Printf(TEXT("%.1fs / %.1fs"), TimeHeld, Required)
		));
	}
}

void UCalibrationWidget::ShowFeedback(const FLinearColor& Color)
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
		&UCalibrationWidget::ClearFeedback,
		FeedbackDuration,
		false
	);
}

void UCalibrationWidget::ClearFeedback()
{
	if (Border_Feedback)
	{
		Border_Feedback->SetBrushColor(FLinearColor::Transparent);
	}
}

// ============================================================================
// DELEGATE HANDLERS
// ============================================================================

void UCalibrationWidget::HandleCalibrationLocked()
{
	ShowFeedback(LockedColor);
}

void UCalibrationWidget::HandleCalibrationLost()
{
	ShowFeedback(LostColor);
}

// ============================================================================
// BLUEPRINT EVENTS
// ============================================================================

void UCalibrationWidget::OnLockGained_Implementation()
{
	// Default: no-op - override for custom effects (sounds, haptics)
}

void UCalibrationWidget::OnLockLost_Implementation()
{
	// Default: no-op
}
