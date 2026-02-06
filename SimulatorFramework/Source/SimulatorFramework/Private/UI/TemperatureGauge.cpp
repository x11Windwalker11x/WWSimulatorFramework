// TemperatureGauge.cpp
// Temperature control UI for cooking/smithing minigames

#include "UI/TemperatureGauge.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Subsystems/TemperatureHandler.h"
#include "TimerManager.h"

void UTemperatureGauge::NativeConstruct()
{
	Super::NativeConstruct();

	if (Bar_Temperature)
	{
		Bar_Temperature->SetPercent(0.0f);
	}
	if (Bar_HoldProgress)
	{
		Bar_HoldProgress->SetPercent(0.0f);
	}
	if (Bar_HeatInput)
	{
		Bar_HeatInput->SetPercent(0.0f);
	}
}

void UTemperatureGauge::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedTemperatureHandler.IsValid())
	{
		return;
	}

	UpdateGaugeDisplay();

	// Edge detection for optimal zone
	bool bInOptimal = CachedTemperatureHandler->IsInOptimalZone();
	if (bInOptimal && !bWasInOptimalZone)
	{
		OnEnteredOptimalZone();
	}
	else if (!bInOptimal && bWasInOptimalZone)
	{
		OnLeftOptimalZone();
	}
	bWasInOptimalZone = bInOptimal;
}

// ============================================================================
// MINIGAME HUD OVERRIDES
// ============================================================================

void UTemperatureGauge::OnMiniGameStarted_Implementation(const FGameplayTag& MiniGameID, UMiniGameHandlerBase* Handler)
{
	Super::OnMiniGameStarted_Implementation(MiniGameID, Handler);

	// Cache temperature handler
	if (CachedHandler.IsValid())
	{
		CachedTemperatureHandler = Cast<UTemperatureHandler>(CachedHandler.Get());
	}

	if (CachedTemperatureHandler.IsValid())
	{
		// Bind to handler-specific delegates
		CachedTemperatureHandler->OnZoneEntered.AddDynamic(this, &UTemperatureGauge::HandleZoneEntered);
		CachedTemperatureHandler->OnItemRuined.AddDynamic(this, &UTemperatureGauge::HandleItemRuined);
	}

	// Reset state
	bWasInOptimalZone = false;

	if (Border_Feedback)
	{
		Border_Feedback->SetBrushColor(FLinearColor::Transparent);
	}
}

void UTemperatureGauge::OnMiniGameEnded_Implementation(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus)
{
	Super::OnMiniGameEnded_Implementation(MiniGameID, bSuccess, bBonus);

	// Unbind from handler delegates
	if (CachedTemperatureHandler.IsValid())
	{
		CachedTemperatureHandler->OnZoneEntered.RemoveDynamic(this, &UTemperatureGauge::HandleZoneEntered);
		CachedTemperatureHandler->OnItemRuined.RemoveDynamic(this, &UTemperatureGauge::HandleItemRuined);
	}

	// Final feedback
	ShowFeedback(bSuccess ? OptimalColor : DangerHotColor);

	if (Text_ZoneStatus)
	{
		Text_ZoneStatus->SetText(FText::FromString(bSuccess ? TEXT("Complete!") : TEXT("Failed")));
		Text_ZoneStatus->SetColorAndOpacity(FSlateColor(bSuccess ? OptimalColor : DangerHotColor));
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FeedbackTimerHandle);
	}

	CachedTemperatureHandler.Reset();
}

void UTemperatureGauge::OnProgressUpdated_Implementation(const FGameplayTag& ObjectiveTag, float Progress)
{
	Super::OnProgressUpdated_Implementation(ObjectiveTag, Progress);
}

// ============================================================================
// DISPLAY UPDATES
// ============================================================================

void UTemperatureGauge::UpdateGaugeDisplay()
{
	if (!CachedTemperatureHandler.IsValid())
	{
		return;
	}

	float CurrentTemp = CachedTemperatureHandler->GetCurrentTemperature();
	float NormalizedTemp = NormalizeTemperature(CurrentTemp);
	FLinearColor ZoneColor = GetZoneColor();

	// Temperature bar
	if (Bar_Temperature)
	{
		Bar_Temperature->SetPercent(NormalizedTemp);
		Bar_Temperature->SetFillColorAndOpacity(ZoneColor);
	}

	// Temperature text
	if (Text_Temperature)
	{
		Text_Temperature->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f\u00B0"), CurrentTemp)
		));
		Text_Temperature->SetColorAndOpacity(FSlateColor(ZoneColor));
	}

	// Heat input bar
	if (Bar_HeatInput)
	{
		Bar_HeatInput->SetPercent(CachedTemperatureHandler->GetHeatInput());
	}

	// Hold progress
	float TimeInZone = CachedTemperatureHandler->GetTimeInOptimalZone();
	float RequiredTime = CachedTemperatureHandler->GetRequiredTimeInZone();

	if (Bar_HoldProgress)
	{
		float HoldPercent = (RequiredTime > 0.0f) ? FMath::Clamp(TimeInZone / RequiredTime, 0.0f, 1.0f) : 0.0f;
		Bar_HoldProgress->SetPercent(HoldPercent);
		Bar_HoldProgress->SetFillColorAndOpacity(OptimalColor);
	}

	if (Text_HoldProgress)
	{
		Text_HoldProgress->SetText(FText::FromString(
			FString::Printf(TEXT("%.1fs / %.1fs"), TimeInZone, RequiredTime)
		));
	}

	// Zone status text
	if (Text_ZoneStatus)
	{
		if (CachedTemperatureHandler->IsRuined())
		{
			Text_ZoneStatus->SetText(FText::FromString(TEXT("RUINED!")));
			Text_ZoneStatus->SetColorAndOpacity(FSlateColor(RuinedColor));
		}
		else if (CachedTemperatureHandler->IsInDangerZone())
		{
			bool bTooHot = CurrentTemp > CachedTemperatureHandler->GetOptimalMax();
			Text_ZoneStatus->SetText(FText::FromString(bTooHot ? TEXT("TOO HOT!") : TEXT("TOO COLD!")));
			Text_ZoneStatus->SetColorAndOpacity(FSlateColor(bTooHot ? DangerHotColor : DangerColdColor));
		}
		else if (CachedTemperatureHandler->IsInOptimalZone())
		{
			Text_ZoneStatus->SetText(FText::FromString(TEXT("Optimal")));
			Text_ZoneStatus->SetColorAndOpacity(FSlateColor(OptimalColor));
		}
		else
		{
			Text_ZoneStatus->SetText(FText::FromString(TEXT("Heating...")));
			Text_ZoneStatus->SetColorAndOpacity(FSlateColor(NormalColor));
		}
	}
}

float UTemperatureGauge::NormalizeTemperature(float Temp) const
{
	if (!CachedTemperatureHandler.IsValid())
	{
		return 0.0f;
	}

	float DangerMin = CachedTemperatureHandler->GetDangerMin();
	float DangerMax = CachedTemperatureHandler->GetDangerMax();
	float Range = DangerMax - DangerMin;

	if (Range <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp((Temp - DangerMin) / Range, 0.0f, 1.0f);
}

FLinearColor UTemperatureGauge::GetZoneColor() const
{
	if (!CachedTemperatureHandler.IsValid())
	{
		return NormalColor;
	}

	if (CachedTemperatureHandler->IsRuined())
	{
		return RuinedColor;
	}

	if (CachedTemperatureHandler->IsInDangerZone())
	{
		float CurrentTemp = CachedTemperatureHandler->GetCurrentTemperature();
		return (CurrentTemp > CachedTemperatureHandler->GetOptimalMax()) ? DangerHotColor : DangerColdColor;
	}

	if (CachedTemperatureHandler->IsInOptimalZone())
	{
		return OptimalColor;
	}

	return NormalColor;
}

void UTemperatureGauge::ShowFeedback(const FLinearColor& Color)
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
		&UTemperatureGauge::ClearFeedback,
		FeedbackDuration,
		false
	);
}

void UTemperatureGauge::ClearFeedback()
{
	if (Border_Feedback)
	{
		Border_Feedback->SetBrushColor(FLinearColor::Transparent);
	}
}

// ============================================================================
// DELEGATE HANDLERS
// ============================================================================

void UTemperatureGauge::HandleZoneEntered(bool bOptimalZone)
{
	if (bOptimalZone)
	{
		ShowFeedback(OptimalColor);
	}
}

void UTemperatureGauge::HandleItemRuined(const FString& Reason)
{
	ShowFeedback(RuinedColor);
	OnItemRuined(Reason);
}

// ============================================================================
// BLUEPRINT EVENTS
// ============================================================================

void UTemperatureGauge::OnEnteredOptimalZone_Implementation()
{
	// Default: no-op - override for custom effects (sounds, particles)
}

void UTemperatureGauge::OnLeftOptimalZone_Implementation()
{
	// Default: no-op
}

void UTemperatureGauge::OnItemRuined_Implementation(const FString& Reason)
{
	// Default: no-op
}
