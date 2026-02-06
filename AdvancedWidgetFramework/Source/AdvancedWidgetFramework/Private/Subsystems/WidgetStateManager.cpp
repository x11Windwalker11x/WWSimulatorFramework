// WidgetStateManager.cpp
// Widget State Machine Manager implementation

#include "Subsystems/WidgetStateManager.h"
#include "Subsystems/AdvancedWidgetFramework/WidgetManagerBase.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

DECLARE_STATS_GROUP(TEXT("WidgetStateManager"), STATGROUP_WidgetStateManager, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("WidgetStateManager::Tick"), STAT_WidgetStateManagerTick, STATGROUP_WidgetStateManager);

TStatId UWidgetStateManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWidgetStateManager, STATGROUP_WidgetStateManager);
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UWidgetStateManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Cache WidgetManagerBase ref (Rule #41)
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer)
	{
		CachedWidgetManager = LocalPlayer->GetSubsystem<UWidgetManagerBase>();

		// Register as transition interceptor
		if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
		{
			WM->OnWidgetTransitionRequested.BindUObject(this, &UWidgetStateManager::HandleTransitionIntercept);
		}
	}
}

void UWidgetStateManager::Deinitialize()
{
	// Unbind from WidgetManagerBase
	if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
	{
		WM->OnWidgetTransitionRequested.Unbind();
	}

	ManagedWidgets.Empty();
	WidgetQueue.Empty();
	PendingDestroy.Empty();

	Super::Deinitialize();
}

// ============================================================================
// TICK - Drives transitions and timeouts
// ============================================================================

void UWidgetStateManager::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_WidgetStateManagerTick);

	for (int32 i = ManagedWidgets.Num() - 1; i >= 0; --i)
	{
		FWidgetStateMachineEntry& Entry = ManagedWidgets[i];

		// Clean up invalid entries
		if (!Entry.Widget.IsValid())
		{
			ManagedWidgets.RemoveAtSwap(i);
			continue;
		}

		Entry.StateElapsed += DeltaTime;

		// AnimatingIn -> check if transition complete
		if (Entry.CurrentState == FWWTagLibrary::UI_Widget_State_AnimatingIn())
		{
			if (Entry.StateElapsed >= Entry.Config.TransitionInDuration)
			{
				CompleteAnimateIn(Entry);
			}
		}
		// AnimatingOut -> check if transition complete
		else if (Entry.CurrentState == FWWTagLibrary::UI_Widget_State_AnimatingOut())
		{
			if (Entry.StateElapsed >= Entry.Config.TransitionOutDuration)
			{
				CompleteAnimateOut(Entry);
			}
		}
		// Visible -> check auto-close timeout
		else if (Entry.CurrentState == FWWTagLibrary::UI_Widget_State_Visible())
		{
			Entry.VisibleElapsed += DeltaTime;

			if (Entry.Config.AutoCloseTimeout > 0.0f && Entry.VisibleElapsed >= Entry.Config.AutoCloseTimeout)
			{
				RequestHide(Entry.Widget.Get());
			}
		}
	}

	// Process pending destroy
	for (int32 i = PendingDestroy.Num() - 1; i >= 0; --i)
	{
		if (UUserWidget* Widget = PendingDestroy[i].Get())
		{
			Widget->RemoveFromParent();
			Widget->ConditionalBeginDestroy();
		}
		PendingDestroy.RemoveAtSwap(i);
	}
}

// ============================================================================
// REGISTRATION
// ============================================================================

void UWidgetStateManager::RegisterWidgetStateMachine(UUserWidget* Widget, const FWidgetStateConfig& Config)
{
	if (!Widget) return;

	// Check if already managed
	if (FindEntry(Widget)) return;

	FWidgetStateMachineEntry NewEntry;
	NewEntry.Widget = Widget;
	NewEntry.Config = Config;
	NewEntry.CurrentState = FWWTagLibrary::UI_Widget_State_Closed();
	NewEntry.StateElapsed = 0.0f;
	NewEntry.VisibleElapsed = 0.0f;

	ManagedWidgets.Add(NewEntry);
}

void UWidgetStateManager::UnregisterWidgetStateMachine(UUserWidget* Widget)
{
	if (!Widget) return;

	for (int32 i = ManagedWidgets.Num() - 1; i >= 0; --i)
	{
		if (ManagedWidgets[i].Widget.Get() == Widget)
		{
			ManagedWidgets.RemoveAtSwap(i);
			break;
		}
	}

	// Also remove from any queues
	for (auto& Pair : WidgetQueue)
	{
		Pair.Value.RemoveAll([Widget](const TWeakObjectPtr<UUserWidget>& Queued)
		{
			return Queued.Get() == Widget;
		});
	}
}

bool UWidgetStateManager::IsWidgetManaged(UUserWidget* Widget) const
{
	return FindEntry(Widget) != nullptr;
}

// ============================================================================
// STATE QUERIES
// ============================================================================

FGameplayTag UWidgetStateManager::GetWidgetState(UUserWidget* Widget) const
{
	if (const FWidgetStateMachineEntry* Entry = FindEntry(Widget))
	{
		return Entry->CurrentState;
	}
	return FGameplayTag();
}

TArray<UUserWidget*> UWidgetStateManager::GetWidgetsInState(FGameplayTag StateTag) const
{
	TArray<UUserWidget*> Result;
	for (const FWidgetStateMachineEntry& Entry : ManagedWidgets)
	{
		if (Entry.CurrentState == StateTag && Entry.Widget.IsValid())
		{
			Result.Add(Entry.Widget.Get());
		}
	}
	return Result;
}

TArray<UUserWidget*> UWidgetStateManager::GetVisibleWidgetsInCategory(FGameplayTag CategoryTag) const
{
	TArray<UUserWidget*> Result;
	for (const FWidgetStateMachineEntry& Entry : ManagedWidgets)
	{
		if (Entry.Config.CategoryTag == CategoryTag && Entry.Widget.IsValid())
		{
			if (Entry.CurrentState == FWWTagLibrary::UI_Widget_State_Visible() ||
				Entry.CurrentState == FWWTagLibrary::UI_Widget_State_AnimatingIn())
			{
				Result.Add(Entry.Widget.Get());
			}
		}
	}
	return Result;
}

FWidgetStateMachineEntry UWidgetStateManager::GetWidgetStateMachineEntry(UUserWidget* Widget) const
{
	if (const FWidgetStateMachineEntry* Entry = FindEntry(Widget))
	{
		return *Entry;
	}
	return FWidgetStateMachineEntry();
}

// ============================================================================
// MANUAL TRANSITIONS
// ============================================================================

void UWidgetStateManager::ForceWidgetState(UUserWidget* Widget, FGameplayTag NewState)
{
	if (FWidgetStateMachineEntry* Entry = FindEntry(Widget))
	{
		TransitionTo(*Entry, NewState);
	}
}

void UWidgetStateManager::RequestShow(UUserWidget* Widget)
{
	FWidgetStateMachineEntry* Entry = FindEntry(Widget);
	if (!Entry) return;

	// Only show from Closed or Paused states
	if (Entry->CurrentState != FWWTagLibrary::UI_Widget_State_Closed() &&
		Entry->CurrentState != FWWTagLibrary::UI_Widget_State_Paused())
	{
		return;
	}

	// Resolve conflicts before showing
	ResolveConflicts(*Entry);

	// Begin show transition
	BeginAnimateIn(*Entry);
}

void UWidgetStateManager::RequestHide(UUserWidget* Widget)
{
	FWidgetStateMachineEntry* Entry = FindEntry(Widget);
	if (!Entry) return;

	// Only hide from Visible or AnimatingIn states
	if (Entry->CurrentState != FWWTagLibrary::UI_Widget_State_Visible() &&
		Entry->CurrentState != FWWTagLibrary::UI_Widget_State_AnimatingIn())
	{
		return;
	}

	BeginAnimateOut(*Entry);
}

void UWidgetStateManager::ResumeWidget(UUserWidget* Widget)
{
	FWidgetStateMachineEntry* Entry = FindEntry(Widget);
	if (!Entry) return;

	if (Entry->CurrentState == FWWTagLibrary::UI_Widget_State_Paused())
	{
		BeginAnimateIn(*Entry);
	}
}

// ============================================================================
// INTERCEPT HANDLER
// ============================================================================

bool UWidgetStateManager::HandleTransitionIntercept(UUserWidget* Widget, bool bShow)
{
	FWidgetStateMachineEntry* Entry = FindEntry(Widget);
	if (!Entry)
	{
		// Widget not managed by state machine - let default behavior happen
		return false;
	}

	if (bShow)
	{
		RequestShow(Widget);
	}
	else
	{
		RequestHide(Widget);
	}

	return true; // We handle the transition
}

// ============================================================================
// STATE TRANSITIONS
// ============================================================================

void UWidgetStateManager::TransitionTo(FWidgetStateMachineEntry& Entry, const FGameplayTag& NewState)
{
	if (Entry.CurrentState == NewState) return;

	FGameplayTag OldState = Entry.CurrentState;
	Entry.CurrentState = NewState;
	Entry.StateElapsed = 0.0f;

	// Reset visible elapsed when entering Visible
	if (NewState == FWWTagLibrary::UI_Widget_State_Visible())
	{
		Entry.VisibleElapsed = 0.0f;
	}

	// Fire delegates
	OnWidgetStateChanged.Broadcast(Entry.Widget.Get(), OldState, NewState);

	// Also fire on WidgetManagerBase for external listeners
	if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
	{
		WM->OnWidgetStateChanged.Broadcast(Entry.Widget.Get(), OldState, NewState);
	}
}

void UWidgetStateManager::BeginAnimateIn(FWidgetStateMachineEntry& Entry)
{
	if (!Entry.Widget.IsValid()) return;

	UUserWidget* Widget = Entry.Widget.Get();

	// Add to viewport if not already
	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(0);
	}

	if (Entry.Config.TransitionInDuration > 0.0f)
	{
		// Start hidden, transition will reveal
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		TransitionTo(Entry, FWWTagLibrary::UI_Widget_State_AnimatingIn());
	}
	else
	{
		// Instant show - skip AnimatingIn
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		TransitionTo(Entry, FWWTagLibrary::UI_Widget_State_Visible());

		// Notify WidgetManagerBase
		if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
		{
			WM->OnWidgetShown.Broadcast(Widget);
		}

		OnWidgetTransitionComplete.Broadcast(Widget, FWWTagLibrary::UI_Widget_State_Visible());
	}
}

void UWidgetStateManager::CompleteAnimateIn(FWidgetStateMachineEntry& Entry)
{
	if (!Entry.Widget.IsValid()) return;

	UUserWidget* Widget = Entry.Widget.Get();
	Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	TransitionTo(Entry, FWWTagLibrary::UI_Widget_State_Visible());

	// Notify WidgetManagerBase
	if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
	{
		WM->OnWidgetShown.Broadcast(Widget);
	}

	OnWidgetTransitionComplete.Broadcast(Widget, FWWTagLibrary::UI_Widget_State_Visible());
}

void UWidgetStateManager::BeginAnimateOut(FWidgetStateMachineEntry& Entry)
{
	if (!Entry.Widget.IsValid()) return;

	if (Entry.Config.TransitionOutDuration > 0.0f)
	{
		TransitionTo(Entry, FWWTagLibrary::UI_Widget_State_AnimatingOut());
	}
	else
	{
		// Instant hide
		CompleteAnimateOut(Entry);
	}
}

void UWidgetStateManager::CompleteAnimateOut(FWidgetStateMachineEntry& Entry)
{
	if (!Entry.Widget.IsValid()) return;

	UUserWidget* Widget = Entry.Widget.Get();

	Widget->SetVisibility(ESlateVisibility::Collapsed);
	TransitionTo(Entry, FWWTagLibrary::UI_Widget_State_Closed());

	// Notify WidgetManagerBase
	if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
	{
		WM->OnWidgetHidden.Broadcast(Widget);
	}

	OnWidgetTransitionComplete.Broadcast(Widget, FWWTagLibrary::UI_Widget_State_Closed());

	// Process queue for this category - next widget in line can show
	if (Entry.Config.CategoryTag.IsValid())
	{
		ProcessQueue(Entry.Config.CategoryTag);
	}
}

void UWidgetStateManager::PauseWidget(FWidgetStateMachineEntry& Entry)
{
	if (!Entry.Widget.IsValid()) return;

	UUserWidget* Widget = Entry.Widget.Get();
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	TransitionTo(Entry, FWWTagLibrary::UI_Widget_State_Paused());
}

// ============================================================================
// CONFLICT RESOLUTION
// ============================================================================

void UWidgetStateManager::ResolveConflicts(FWidgetStateMachineEntry& IncomingEntry)
{
	if (!IncomingEntry.Config.CategoryTag.IsValid()) return;
	if (IncomingEntry.Config.bAllowConcurrent) return;

	FGameplayTag Category = IncomingEntry.Config.CategoryTag;

	for (FWidgetStateMachineEntry& Existing : ManagedWidgets)
	{
		// Skip self
		if (Existing.Widget == IncomingEntry.Widget) continue;

		// Skip different categories
		if (Existing.Config.CategoryTag != Category) continue;

		// Skip already closed/animating-out widgets
		if (Existing.CurrentState == FWWTagLibrary::UI_Widget_State_Closed() ||
			Existing.CurrentState == FWWTagLibrary::UI_Widget_State_AnimatingOut())
		{
			continue;
		}

		// Skip if concurrent is allowed on existing
		if (Existing.Config.bAllowConcurrent) continue;

		// Conflict detected - incoming has higher or equal priority
		if (IncomingEntry.Config.Priority >= Existing.Config.Priority)
		{
			switch (Existing.Config.InterruptMode)
			{
			case EWidgetInterruptMode::Cancel:
				// Close the existing widget entirely
				BeginAnimateOut(Existing);
				break;

			case EWidgetInterruptMode::Queue:
				// Queue existing to reopen after incoming closes
				{
					TArray<TWeakObjectPtr<UUserWidget>>& Queue = WidgetQueue.FindOrAdd(Category);
					Queue.Add(Existing.Widget);
					BeginAnimateOut(Existing);
				}
				break;

			case EWidgetInterruptMode::Pause:
				// Pause existing (will resume when incoming closes)
				PauseWidget(Existing);
				break;
			}
		}
		else
		{
			// Existing has higher priority - queue the incoming widget instead
			TArray<TWeakObjectPtr<UUserWidget>>& Queue = WidgetQueue.FindOrAdd(Category);
			Queue.Add(IncomingEntry.Widget);
			// Don't proceed with showing incoming
			return;
		}
	}
}

void UWidgetStateManager::ProcessQueue(FGameplayTag CategoryTag)
{
	if (!CategoryTag.IsValid()) return;

	// First check for paused widgets to resume
	for (FWidgetStateMachineEntry& Entry : ManagedWidgets)
	{
		if (Entry.Config.CategoryTag == CategoryTag &&
			Entry.CurrentState == FWWTagLibrary::UI_Widget_State_Paused() &&
			Entry.Widget.IsValid())
		{
			ResumeWidget(Entry.Widget.Get());
			return; // Resume one at a time
		}
	}

	// Then check queue
	TArray<TWeakObjectPtr<UUserWidget>>* Queue = WidgetQueue.Find(CategoryTag);
	if (!Queue || Queue->Num() == 0) return;

	// Find highest priority queued widget
	int32 BestIndex = -1;
	int32 BestPriority = -1;

	for (int32 i = Queue->Num() - 1; i >= 0; --i)
	{
		if (!(*Queue)[i].IsValid())
		{
			Queue->RemoveAtSwap(i);
			continue;
		}

		if (const FWidgetStateMachineEntry* Entry = FindEntry((*Queue)[i].Get()))
		{
			if (Entry->Config.Priority > BestPriority)
			{
				BestPriority = Entry->Config.Priority;
				BestIndex = i;
			}
		}
	}

	if (BestIndex >= 0)
	{
		UUserWidget* NextWidget = (*Queue)[BestIndex].Get();
		Queue->RemoveAtSwap(BestIndex);
		RequestShow(NextWidget);
	}
}

// ============================================================================
// INTERNAL
// ============================================================================

FWidgetStateMachineEntry* UWidgetStateManager::FindEntry(UUserWidget* Widget)
{
	if (!Widget) return nullptr;

	for (FWidgetStateMachineEntry& Entry : ManagedWidgets)
	{
		if (Entry.Widget.Get() == Widget)
		{
			return &Entry;
		}
	}
	return nullptr;
}

const FWidgetStateMachineEntry* UWidgetStateManager::FindEntry(UUserWidget* Widget) const
{
	if (!Widget) return nullptr;

	for (const FWidgetStateMachineEntry& Entry : ManagedWidgets)
	{
		if (Entry.Widget.Get() == Widget)
		{
			return &Entry;
		}
	}
	return nullptr;
}
