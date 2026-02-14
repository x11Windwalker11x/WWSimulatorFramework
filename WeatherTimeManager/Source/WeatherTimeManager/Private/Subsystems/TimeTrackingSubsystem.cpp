// Copyright Windwalker Productions. All Rights Reserved.

#include "Subsystems/TimeTrackingSubsystem.h"
#include "Interfaces/WeatherTimeManager/TimeWeatherProviderInterface.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"
#include "EngineUtils.h"

// ============================================================================
// CONSOLE COMMAND REGISTRATION
// ============================================================================

static FAutoConsoleCommandWithWorldAndArgs GCmdSetTime(
	TEXT("WW.SetTime"),
	TEXT("Set time of day (0-24). Usage: WW.SetTime 12"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UTimeTrackingSubsystem::CmdSetTime)
);

static FAutoConsoleCommandWithWorldAndArgs GCmdSetSpeed(
	TEXT("WW.SetSpeed"),
	TEXT("Set time speed multiplier. Usage: WW.SetSpeed 2.0"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UTimeTrackingSubsystem::CmdSetSpeed)
);

static FAutoConsoleCommandWithWorldAndArgs GCmdPauseTime(
	TEXT("WW.PauseTime"),
	TEXT("Toggle time pause. Usage: WW.PauseTime"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UTimeTrackingSubsystem::CmdPauseTime)
);

static FAutoConsoleCommandWithWorldAndArgs GCmdSetWeather(
	TEXT("WW.SetWeather"),
	TEXT("Set weather immediately. Usage: WW.SetWeather Weather.Type.Rain"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UTimeTrackingSubsystem::CmdSetWeather)
);

// ============================================================================
// LIFECYCLE
// ============================================================================

UTimeTrackingSubsystem::UTimeTrackingSubsystem()
{
}

void UTimeTrackingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitDefaultThresholds();

	// Set initial state
	TimeState.CurrentHour = 6.0f;
	TimeState.DayNumber = 1;
	TimeState.TimeSpeedMultiplier = DefaultGameHoursPerRealMinute;
	TimeState.bTimePaused = false;

	WeatherState.CurrentWeatherTag = FWWTagLibrary::Weather_Type_Clear();
	WeatherState.CurrentIntensity = 1.0f;
	WeatherState.bTransitioning = false;

	CachedHour = FMath::FloorToInt32(TimeState.CurrentHour);

	// Evaluate initial period
	EvaluateTimeOfDayPeriod();
}

void UTimeTrackingSubsystem::Deinitialize()
{
	StopTimeProgression();
	SkyProviders.Empty();

	Super::Deinitialize();
}

UTimeTrackingSubsystem* UTimeTrackingSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UTimeTrackingSubsystem>();
}

// ============================================================================
// TIME API
// ============================================================================

void UTimeTrackingSubsystem::SetTimeOfDay(float Hour)
{
	Hour = FMath::Fmod(Hour, 24.0f);
	if (Hour < 0.0f)
	{
		Hour += 24.0f;
	}

	const int32 OldHour = FMath::FloorToInt32(TimeState.CurrentHour);
	TimeState.CurrentHour = Hour;

	const int32 NewHour = FMath::FloorToInt32(Hour);
	if (OldHour != NewHour)
	{
		CachedHour = NewHour;
		OnHourChanged.Broadcast(OldHour, NewHour);
	}

	EvaluateTimeOfDayPeriod();
	PushStateToProviders();
}

void UTimeTrackingSubsystem::SetTimeSpeed(float Multiplier)
{
	TimeState.TimeSpeedMultiplier = FMath::Max(0.0f, Multiplier);
}

void UTimeTrackingSubsystem::PauseTime()
{
	if (!TimeState.bTimePaused)
	{
		TimeState.bTimePaused = true;
		OnTimePaused.Broadcast();
	}
}

void UTimeTrackingSubsystem::ResumeTime()
{
	if (TimeState.bTimePaused)
	{
		TimeState.bTimePaused = false;
		OnTimeResumed.Broadcast();
	}
}

void UTimeTrackingSubsystem::StartTimeProgression()
{
	UWorld* World = GetWorldForTimers();
	if (!World)
	{
		return;
	}

	bTimeProgressionActive = true;

	World->GetTimerManager().SetTimer(
		TimeProgressionHandle,
		this,
		&UTimeTrackingSubsystem::OnTimerTick,
		TickInterval,
		true
	);
}

void UTimeTrackingSubsystem::StopTimeProgression()
{
	bTimeProgressionActive = false;

	UWorld* World = GetWorldForTimers();
	if (World)
	{
		World->GetTimerManager().ClearTimer(TimeProgressionHandle);
	}
}

// ============================================================================
// WEATHER API
// ============================================================================

void UTimeTrackingSubsystem::SetWeatherImmediate(FGameplayTag WeatherTag, float Intensity)
{
	if (!WeatherTag.IsValid())
	{
		return;
	}

	const FGameplayTag OldWeather = WeatherState.CurrentWeatherTag;

	WeatherState.CurrentWeatherTag = WeatherTag;
	WeatherState.TargetWeatherTag = FGameplayTag();
	WeatherState.TransitionAlpha = 0.0f;
	WeatherState.bTransitioning = false;
	WeatherState.CurrentIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	if (OldWeather != WeatherTag)
	{
		OnWeatherChanged.Broadcast(OldWeather, WeatherTag);
	}

	PushStateToProviders();
}

void UTimeTrackingSubsystem::TransitionToWeather(FGameplayTag TargetWeather, float Duration, float TargetIntensity)
{
	if (!TargetWeather.IsValid() || Duration <= 0.0f)
	{
		return;
	}

	// If already this weather, skip
	if (WeatherState.CurrentWeatherTag == TargetWeather && !WeatherState.bTransitioning)
	{
		return;
	}

	WeatherState.TargetWeatherTag = TargetWeather;
	WeatherState.TransitionAlpha = 0.0f;
	WeatherState.bTransitioning = true;
	WeatherTransitionDuration = Duration;
	WeatherTransitionElapsed = 0.0f;
	WeatherTargetIntensity = FMath::Clamp(TargetIntensity, 0.0f, 1.0f);

	OnWeatherTransitionStarted.Broadcast(WeatherState.CurrentWeatherTag, TargetWeather);
}

void UTimeTrackingSubsystem::CancelWeatherTransition()
{
	if (WeatherState.bTransitioning)
	{
		WeatherState.TargetWeatherTag = FGameplayTag();
		WeatherState.TransitionAlpha = 0.0f;
		WeatherState.bTransitioning = false;
		WeatherTransitionElapsed = 0.0f;
	}
}

// ============================================================================
// PROVIDER API
// ============================================================================

void UTimeTrackingSubsystem::RegisterSkyProvider(AActor* Provider)
{
	if (!Provider)
	{
		return;
	}

	if (!Provider->GetClass()->ImplementsInterface(UTimeWeatherProviderInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeTrackingSubsystem::RegisterSkyProvider - Actor %s does not implement ITimeWeatherProviderInterface"), *Provider->GetName());
		return;
	}

	// Avoid duplicates
	for (const TWeakObjectPtr<AActor>& Existing : SkyProviders)
	{
		if (Existing.Get() == Provider)
		{
			return;
		}
	}

	SkyProviders.Add(Provider);

	// Push current state immediately to new provider
	ITimeWeatherProviderInterface::Execute_SetTimeOfDay(Provider, TimeState.CurrentHour);
	ITimeWeatherProviderInterface::Execute_SetWeatherState(Provider, WeatherState.CurrentWeatherTag, WeatherState.CurrentIntensity);
}

void UTimeTrackingSubsystem::UnregisterSkyProvider(AActor* Provider)
{
	if (!Provider)
	{
		return;
	}

	SkyProviders.RemoveAll([Provider](const TWeakObjectPtr<AActor>& Weak)
	{
		return Weak.Get() == Provider;
	});
}

void UTimeTrackingSubsystem::DiscoverSkyProviders()
{
	UWorld* World = GetWorldForTimers();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor->GetClass()->ImplementsInterface(UTimeWeatherProviderInterface::StaticClass()))
		{
			RegisterSkyProvider(Actor);
		}
	}
}

// ============================================================================
// INTERNAL - TIMER TICK
// ============================================================================

void UTimeTrackingSubsystem::OnTimerTick()
{
	if (!TimeState.bTimePaused)
	{
		AdvanceTime(TickInterval);
	}

	if (WeatherState.bTransitioning)
	{
		UpdateWeatherTransition(TickInterval);
	}
}

void UTimeTrackingSubsystem::AdvanceTime(float DeltaSeconds)
{
	// Convert real seconds to game hours:
	// TimeSpeedMultiplier = game-hours per real-minute
	// So game-hours per real-second = TimeSpeedMultiplier / 60.0
	const float GameHoursPerSecond = TimeState.TimeSpeedMultiplier / 60.0f;
	const float HourDelta = GameHoursPerSecond * DeltaSeconds;

	const float OldHour = TimeState.CurrentHour;
	TimeState.CurrentHour += HourDelta;

	// Handle day rollover
	if (TimeState.CurrentHour >= 24.0f)
	{
		TimeState.CurrentHour -= 24.0f;
		const int32 OldDay = TimeState.DayNumber;
		TimeState.DayNumber++;
		OnDayChanged.Broadcast(OldDay, TimeState.DayNumber);
	}

	// Detect integer hour change
	const int32 NewHour = FMath::FloorToInt32(TimeState.CurrentHour);
	if (NewHour != CachedHour)
	{
		const int32 OldHourInt = CachedHour;
		CachedHour = NewHour;
		OnHourChanged.Broadcast(OldHourInt, NewHour);
	}

	EvaluateTimeOfDayPeriod();
	PushStateToProviders();
}

void UTimeTrackingSubsystem::UpdateWeatherTransition(float DeltaSeconds)
{
	WeatherTransitionElapsed += DeltaSeconds;
	WeatherState.TransitionAlpha = FMath::Clamp(WeatherTransitionElapsed / WeatherTransitionDuration, 0.0f, 1.0f);

	// Push transition alpha to providers
	CleanStaleProviders();
	for (const TWeakObjectPtr<AActor>& Weak : SkyProviders)
	{
		if (AActor* Provider = Weak.Get())
		{
			ITimeWeatherProviderInterface::Execute_SetWeatherTransitionAlpha(Provider, WeatherState.TransitionAlpha);
		}
	}

	// Transition complete?
	if (WeatherTransitionElapsed >= WeatherTransitionDuration)
	{
		const FGameplayTag OldWeather = WeatherState.CurrentWeatherTag;
		const FGameplayTag NewWeather = WeatherState.TargetWeatherTag;

		WeatherState.CurrentWeatherTag = NewWeather;
		WeatherState.TargetWeatherTag = FGameplayTag();
		WeatherState.TransitionAlpha = 0.0f;
		WeatherState.bTransitioning = false;
		WeatherState.CurrentIntensity = WeatherTargetIntensity;
		WeatherTransitionElapsed = 0.0f;

		OnWeatherTransitionComplete.Broadcast(NewWeather);
		OnWeatherChanged.Broadcast(OldWeather, NewWeather);

		PushStateToProviders();
	}
}

void UTimeTrackingSubsystem::EvaluateTimeOfDayPeriod()
{
	const float Hour = TimeState.CurrentHour;
	FGameplayTag NewPeriod;

	for (const FTimeThreshold& Threshold : TimeThresholds)
	{
		// Handle wrap-around (Night: 21:00 -> 5:00)
		if (Threshold.StartHour > Threshold.EndHour)
		{
			if (Hour >= Threshold.StartHour || Hour < Threshold.EndHour)
			{
				NewPeriod = Threshold.TimeOfDayTag;
				break;
			}
		}
		else
		{
			if (Hour >= Threshold.StartHour && Hour < Threshold.EndHour)
			{
				NewPeriod = Threshold.TimeOfDayTag;
				break;
			}
		}
	}

	if (NewPeriod.IsValid() && NewPeriod != TimeState.TimeOfDayTag)
	{
		const FGameplayTag OldPeriod = TimeState.TimeOfDayTag;
		TimeState.TimeOfDayTag = NewPeriod;
		OnTimeOfDayChanged.Broadcast(OldPeriod, NewPeriod);
	}
}

void UTimeTrackingSubsystem::PushStateToProviders()
{
	CleanStaleProviders();

	for (const TWeakObjectPtr<AActor>& Weak : SkyProviders)
	{
		if (AActor* Provider = Weak.Get())
		{
			ITimeWeatherProviderInterface::Execute_SetTimeOfDay(Provider, TimeState.CurrentHour);
			ITimeWeatherProviderInterface::Execute_SetWeatherState(Provider, WeatherState.CurrentWeatherTag, WeatherState.CurrentIntensity);
		}
	}
}

void UTimeTrackingSubsystem::InitDefaultThresholds()
{
	TimeThresholds.Empty();

	auto AddThreshold = [this](const FGameplayTag& Tag, float Start, float End)
	{
		FTimeThreshold T;
		T.TimeOfDayTag = Tag;
		T.StartHour = Start;
		T.EndHour = End;
		TimeThresholds.Add(T);
	};

	// Night wraps around (21:00 -> 5:00) — must be first for priority
	AddThreshold(FWWTagLibrary::Time_State_Night(), 21.0f, 5.0f);
	AddThreshold(FWWTagLibrary::Time_State_Dawn(), 5.0f, 7.0f);
	AddThreshold(FWWTagLibrary::Time_State_Morning(), 7.0f, 12.0f);
	AddThreshold(FWWTagLibrary::Time_State_Afternoon(), 12.0f, 17.0f);
	AddThreshold(FWWTagLibrary::Time_State_Evening(), 17.0f, 20.0f);
	AddThreshold(FWWTagLibrary::Time_State_Dusk(), 20.0f, 21.0f);
}

void UTimeTrackingSubsystem::CleanStaleProviders()
{
	SkyProviders.RemoveAll([](const TWeakObjectPtr<AActor>& Weak)
	{
		return !Weak.IsValid();
	});
}

UWorld* UTimeTrackingSubsystem::GetWorldForTimers() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}

// ============================================================================
// CONSOLE COMMANDS
// ============================================================================

void UTimeTrackingSubsystem::CmdSetTime(const TArray<FString>& Args, UWorld* World)
{
	if (Args.Num() < 1 || !World)
	{
		return;
	}

	const float Hour = FCString::Atof(*Args[0]);

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (UTimeTrackingSubsystem* Sub = GI->GetSubsystem<UTimeTrackingSubsystem>())
	{
		Sub->SetTimeOfDay(Hour);
		UE_LOG(LogTemp, Log, TEXT("WW.SetTime: Set to %.1f"), Hour);
	}
}

void UTimeTrackingSubsystem::CmdSetSpeed(const TArray<FString>& Args, UWorld* World)
{
	if (Args.Num() < 1 || !World)
	{
		return;
	}

	const float Speed = FCString::Atof(*Args[0]);

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (UTimeTrackingSubsystem* Sub = GI->GetSubsystem<UTimeTrackingSubsystem>())
	{
		Sub->SetTimeSpeed(Speed);
		UE_LOG(LogTemp, Log, TEXT("WW.SetSpeed: Set to %.2f"), Speed);
	}
}

void UTimeTrackingSubsystem::CmdPauseTime(const TArray<FString>& Args, UWorld* World)
{
	if (!World)
	{
		return;
	}

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (UTimeTrackingSubsystem* Sub = GI->GetSubsystem<UTimeTrackingSubsystem>())
	{
		if (Sub->IsTimePaused())
		{
			Sub->ResumeTime();
			UE_LOG(LogTemp, Log, TEXT("WW.PauseTime: Resumed"));
		}
		else
		{
			Sub->PauseTime();
			UE_LOG(LogTemp, Log, TEXT("WW.PauseTime: Paused"));
		}
	}
}

void UTimeTrackingSubsystem::CmdSetWeather(const TArray<FString>& Args, UWorld* World)
{
	if (Args.Num() < 1 || !World)
	{
		return;
	}

	const FGameplayTag WeatherTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
	if (!WeatherTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WW.SetWeather: Invalid tag '%s'"), *Args[0]);
		return;
	}

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (UTimeTrackingSubsystem* Sub = GI->GetSubsystem<UTimeTrackingSubsystem>())
	{
		Sub->SetWeatherImmediate(WeatherTag);
		UE_LOG(LogTemp, Log, TEXT("WW.SetWeather: Set to %s"), *WeatherTag.ToString());
	}
}
