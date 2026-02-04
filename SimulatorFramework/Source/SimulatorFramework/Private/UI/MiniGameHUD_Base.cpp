// MiniGameHUD_Base.cpp
#include "UI/MiniGameHUD_Base.h"
#include "Components/MiniGameComponent.h"
#include "Subsystems/MiniGameHandlerBase.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "TimerManager.h"

UMiniGameHUD_Base::UMiniGameHUD_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set widget category for ESC priority system (HUD level - never closes via ESC)
	WidgetCategoryTag = FWWTagLibrary::UI_Widget_Category_HUD();
	bAutoRegister = true;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMiniGameHUD_Base::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden - will show when mini-game starts
	SetVisibility(ESlateVisibility::Collapsed);
	bIsHUDVisible = false;
}

void UMiniGameHUD_Base::NativeDestruct()
{
	// Clear any pending timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
	}

	// Unbind from component
	UnbindFromMiniGameComponent();

	Super::NativeDestruct();
}

void UMiniGameHUD_Base::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Only tick if mini-game is active and HUD is visible
	if (bIsMiniGameActive && bIsHUDVisible)
	{
		OnMiniGameTick(InDeltaTime);
	}
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UMiniGameHUD_Base::BindToMiniGameComponent(UMiniGameComponent* InMiniGameComponent)
{
	if (!InMiniGameComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MiniGameHUD_Base: Cannot bind to null component"));
		return;
	}

	// Unbind from previous if any
	UnbindFromMiniGameComponent();

	// Store reference
	CachedMiniGameComponent = InMiniGameComponent;

	// Bind to component delegates
	InMiniGameComponent->OnMiniGameStarted.AddDynamic(this, &UMiniGameHUD_Base::HandleMiniGameStarted);
	InMiniGameComponent->OnMiniGameEnded.AddDynamic(this, &UMiniGameHUD_Base::HandleMiniGameEnded);
	InMiniGameComponent->OnMiniGameCancelled.AddDynamic(this, &UMiniGameHUD_Base::HandleMiniGameCancelled);

	// If mini-game already active, sync state
	if (InMiniGameComponent->IsMiniGameActive())
	{
		HandleMiniGameStarted(InMiniGameComponent->GetCurrentMiniGameID(), InMiniGameComponent->GetCurrentHandler());
	}

	// Notify subclasses
	OnBoundToComponent();

	UE_LOG(LogTemp, Log, TEXT("MiniGameHUD_Base: Bound to MiniGameComponent"));
}

void UMiniGameHUD_Base::UnbindFromMiniGameComponent()
{
	if (UMiniGameComponent* Component = CachedMiniGameComponent.Get())
	{
		// Unbind from component delegates
		Component->OnMiniGameStarted.RemoveDynamic(this, &UMiniGameHUD_Base::HandleMiniGameStarted);
		Component->OnMiniGameEnded.RemoveDynamic(this, &UMiniGameHUD_Base::HandleMiniGameEnded);
		Component->OnMiniGameCancelled.RemoveDynamic(this, &UMiniGameHUD_Base::HandleMiniGameCancelled);
	}

	// Unbind from handler if any
	UnbindFromHandler();

	// Notify subclasses
	OnUnboundFromComponent();

	// Clear cached refs
	CachedMiniGameComponent.Reset();
	CachedHandler.Reset();
	CachedMiniGameID = FGameplayTag();
	CachedMiniGameType = FGameplayTag();
	CachedConfig = FMiniGameData();
	bIsMiniGameActive = false;
}

// ============================================================================
// STATE QUERIES
// ============================================================================

bool UMiniGameHUD_Base::IsMiniGameActive() const
{
	return bIsMiniGameActive;
}

FGameplayTag UMiniGameHUD_Base::GetCurrentMiniGameID() const
{
	return CachedMiniGameID;
}

FGameplayTag UMiniGameHUD_Base::GetCurrentMiniGameType() const
{
	return CachedMiniGameType;
}

float UMiniGameHUD_Base::GetCurrentProgress() const
{
	if (UMiniGameHandlerBase* Handler = CachedHandler.Get())
	{
		return Handler->GetProgress();
	}
	return 0.0f;
}

float UMiniGameHUD_Base::GetElapsedTime() const
{
	if (UMiniGameComponent* Component = CachedMiniGameComponent.Get())
	{
		return Component->GetCurrentProgress();
	}
	return 0.0f;
}

float UMiniGameHUD_Base::GetTimeRemaining() const
{
	if (!HasTimeout()) return 0.0f;

	float Elapsed = GetElapsedTime();
	return FMath::Max(0.0f, CachedConfig.TimeoutSeconds - Elapsed);
}

bool UMiniGameHUD_Base::HasTimeout() const
{
	return CachedConfig.TimeoutSeconds > 0.0f;
}

UMiniGameHandlerBase* UMiniGameHUD_Base::GetActiveHandler() const
{
	return CachedHandler.Get();
}

// ============================================================================
// VISIBILITY CONTROL
// ============================================================================

void UMiniGameHUD_Base::ShowHUD()
{
	if (bIsHUDVisible) return;

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	bIsHUDVisible = true;

	UE_LOG(LogTemp, Log, TEXT("MiniGameHUD_Base: HUD shown"));
}

void UMiniGameHUD_Base::HideHUD()
{
	if (!bIsHUDVisible) return;

	SetVisibility(ESlateVisibility::Collapsed);
	bIsHUDVisible = false;

	UE_LOG(LogTemp, Log, TEXT("MiniGameHUD_Base: HUD hidden"));
}

// ============================================================================
// DELEGATE HANDLERS
// ============================================================================

void UMiniGameHUD_Base::HandleMiniGameStarted(const FGameplayTag& MiniGameID, UMiniGameHandlerBase* Handler)
{
	if (!Handler) return;

	// Cache state
	CachedHandler = Handler;
	CachedMiniGameID = MiniGameID;
	CachedConfig = Handler->Config;
	CachedMiniGameType = CachedConfig.TypeTag;

	// Check if this HUD should handle this type
	if (!ShouldHandleMiniGameType(CachedMiniGameType))
	{
		return;
	}

	bIsMiniGameActive = true;

	// Bind to handler delegates
	BindToHandler(Handler);

	// Clear any pending hide timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
	}

	// Show HUD
	ShowHUD();

	// Notify Blueprint/subclasses
	OnMiniGameStarted(MiniGameID, Handler);
}

void UMiniGameHUD_Base::HandleMiniGameEnded(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus)
{
	if (!bIsMiniGameActive) return;

	// Unbind from handler
	UnbindFromHandler();

	bIsMiniGameActive = false;

	// Notify Blueprint/subclasses
	OnMiniGameEnded(MiniGameID, bSuccess, bBonus);

	// Auto-hide with delay
	if (bAutoHideOnEnd)
	{
		if (AutoHideDelay > 0.0f)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					AutoHideTimerHandle,
					this,
					&UMiniGameHUD_Base::PerformDelayedHide,
					AutoHideDelay,
					false
				);
			}
		}
		else
		{
			HideHUD();
		}
	}

	// Clear cached handler (keep config for end animations)
	CachedHandler.Reset();
}

void UMiniGameHUD_Base::HandleMiniGameCancelled(const FGameplayTag& MiniGameID, const FString& Reason)
{
	if (!bIsMiniGameActive) return;

	// Unbind from handler
	UnbindFromHandler();

	bIsMiniGameActive = false;

	// Notify Blueprint/subclasses
	OnMiniGameCancelled(MiniGameID, Reason);

	// Hide immediately on cancel (no delay)
	HideHUD();

	// Clear cached state
	CachedHandler.Reset();
	CachedMiniGameID = FGameplayTag();
	CachedMiniGameType = FGameplayTag();
	CachedConfig = FMiniGameData();
}

void UMiniGameHUD_Base::HandleProgressUpdated(const FGameplayTag& ObjectiveTag, float Progress)
{
	// Forward to Blueprint event
	OnProgressUpdated(ObjectiveTag, Progress);
}

// ============================================================================
// BLUEPRINT NATIVE EVENTS (Default implementations)
// ============================================================================

void UMiniGameHUD_Base::OnMiniGameStarted_Implementation(const FGameplayTag& MiniGameID, UMiniGameHandlerBase* Handler)
{
	// Default: nothing - override in subclasses/Blueprints
}

void UMiniGameHUD_Base::OnMiniGameEnded_Implementation(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus)
{
	// Default: nothing - override in subclasses/Blueprints
}

void UMiniGameHUD_Base::OnMiniGameCancelled_Implementation(const FGameplayTag& MiniGameID, const FString& Reason)
{
	// Default: nothing - override in subclasses/Blueprints
}

void UMiniGameHUD_Base::OnProgressUpdated_Implementation(const FGameplayTag& ObjectiveTag, float Progress)
{
	// Default: nothing - override in subclasses/Blueprints
}

// ============================================================================
// VIRTUAL METHODS
// ============================================================================

bool UMiniGameHUD_Base::ShouldHandleMiniGameType(const FGameplayTag& TypeTag) const
{
	// If no filter set, handle all types
	if (HandledMiniGameTypes.IsEmpty())
	{
		return true;
	}

	// Check if type matches any in our filter
	return HandledMiniGameTypes.HasTag(TypeTag);
}

void UMiniGameHUD_Base::OnBoundToComponent()
{
	// Override in subclasses for additional setup
}

void UMiniGameHUD_Base::OnUnboundFromComponent()
{
	// Override in subclasses for additional cleanup
}

// ============================================================================
// INTERNAL
// ============================================================================

void UMiniGameHUD_Base::BindToHandler(UMiniGameHandlerBase* Handler)
{
	if (!Handler) return;

	Handler->OnProgress.AddDynamic(this, &UMiniGameHUD_Base::HandleProgressUpdated);
}

void UMiniGameHUD_Base::UnbindFromHandler()
{
	if (UMiniGameHandlerBase* Handler = CachedHandler.Get())
	{
		Handler->OnProgress.RemoveDynamic(this, &UMiniGameHUD_Base::HandleProgressUpdated);
	}
}

void UMiniGameHUD_Base::PerformDelayedHide()
{
	HideHUD();

	// Clear remaining cached state
	CachedMiniGameID = FGameplayTag();
	CachedMiniGameType = FGameplayTag();
	CachedConfig = FMiniGameData();
}
