// Copyright Windwalker Productions. All Rights Reserved.

#include "MasterWidgets/MarqueeSelectionWidget_Base.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

UMarqueeSelectionWidget_Base::UMarqueeSelectionWidget_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Overlay widget — do not register with WidgetManager ESC stack
	bAutoRegister = false;
}

// ================================================================
// PUBLIC API
// ================================================================

void UMarqueeSelectionWidget_Base::ActivateMarquee(UCanvasPanel* ParentCanvas)
{
	if (!ParentCanvas || bIsActive)
	{
		return;
	}

	CachedParentCanvas = ParentCanvas;
	bIsActive = true;

	// Apply config visuals to the border
	if (Border_SelectionBox)
	{
		Border_SelectionBox->SetBrushColor(MarqueeConfig.FillColor);
		Border_SelectionBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMarqueeSelectionWidget_Base::DeactivateMarquee()
{
	if (bIsDragging)
	{
		CancelMarquee();
	}

	bIsActive = false;

	if (Border_SelectionBox)
	{
		Border_SelectionBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetVisibility(ESlateVisibility::Collapsed);
	CachedParentCanvas.Reset();
}

void UMarqueeSelectionWidget_Base::SetMarqueeConfig(const FMarqueeConfig& NewConfig)
{
	MarqueeConfig = NewConfig;

	if (Border_SelectionBox)
	{
		Border_SelectionBox->SetBrushColor(MarqueeConfig.FillColor);
	}
}

// ================================================================
// INPUT FORWARDING
// ================================================================

bool UMarqueeSelectionWidget_Base::HandleMouseDown(FVector2D LocalPosition, const FPointerEvent& PointerEvent)
{
	if (!bIsActive)
	{
		return false;
	}

	if (!ShouldActivateMarquee(LocalPosition, PointerEvent))
	{
		return false;
	}

	DragStartLocal = LocalPosition;
	DragCurrentLocal = LocalPosition;
	bIsDragging = false;

	// Detect modifier mode from key state
	DetectModifierMode(PointerEvent);

	return true;
}

bool UMarqueeSelectionWidget_Base::HandleMouseMove(FVector2D LocalPosition, const FPointerEvent& PointerEvent)
{
	if (!bIsActive || DragStartLocal.IsZero())
	{
		return false;
	}

	DragCurrentLocal = LocalPosition;

	const float Distance = FVector2D::Distance(DragStartLocal, DragCurrentLocal);

	// Check if we've passed the drag threshold
	if (!bIsDragging)
	{
		if (Distance < MarqueeConfig.MinDragThreshold)
		{
			return false;
		}

		// First time past threshold — begin marquee
		bIsDragging = true;

		if (Border_SelectionBox)
		{
			Border_SelectionBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}

		OnMarqueeStarted(DragStartLocal);
	}

	// Calculate marquee rectangle
	const FVector2D TopLeft(
		FMath::Min(DragStartLocal.X, DragCurrentLocal.X),
		FMath::Min(DragStartLocal.Y, DragCurrentLocal.Y)
	);
	const FVector2D BottomRight(
		FMath::Max(DragStartLocal.X, DragCurrentLocal.X),
		FMath::Max(DragStartLocal.Y, DragCurrentLocal.Y)
	);
	const FVector2D Size = BottomRight - TopLeft;

	// Update visual rectangle
	UpdateMarqueeVisual(TopLeft, Size);

	// Gather candidates and test overlaps
	const int32 OverlappingCount = UpdateCandidateOverlaps(TopLeft, BottomRight);

	OnMarqueeUpdated(TopLeft, BottomRight, OverlappingCount);

	return true;
}

bool UMarqueeSelectionWidget_Base::HandleMouseUp(FVector2D LocalPosition, const FPointerEvent& PointerEvent)
{
	if (!bIsActive || DragStartLocal.IsZero())
	{
		return false;
	}

	// If never started dragging, cancel
	if (!bIsDragging)
	{
		CancelMarquee();
		return true;
	}

	DragCurrentLocal = LocalPosition;

	// Calculate final marquee rectangle
	const FVector2D TopLeft(
		FMath::Min(DragStartLocal.X, DragCurrentLocal.X),
		FMath::Min(DragStartLocal.Y, DragCurrentLocal.Y)
	);
	const FVector2D BottomRight(
		FMath::Max(DragStartLocal.X, DragCurrentLocal.X),
		FMath::Max(DragStartLocal.Y, DragCurrentLocal.Y)
	);
	const FVector2D Size = BottomRight - TopLeft;

	// Check minimum marquee size
	if (Size.X < MarqueeConfig.MinMarqueeSize && Size.Y < MarqueeConfig.MinMarqueeSize)
	{
		CancelMarquee();
		return true;
	}

	// Final gather + overlap test
	TArray<TWeakObjectPtr<UObject>> FinalOverlapping;
	const int32 TotalTested = UpdateCandidateOverlaps(TopLeft, BottomRight, &FinalOverlapping);

	// Cap at MaxSelectionCount if set
	if (MarqueeConfig.MaxSelectionCount > 0 && FinalOverlapping.Num() > MarqueeConfig.MaxSelectionCount)
	{
		FinalOverlapping.SetNum(MarqueeConfig.MaxSelectionCount);
	}

	// Build result
	FMarqueeSelectionResult Result;
	Result.SelectedObjects = MoveTemp(FinalOverlapping);
	Result.ModifierMode = CurrentModifierMode;
	Result.MarqueeTopLeft = TopLeft;
	Result.MarqueeBottomRight = BottomRight;
	Result.TotalCandidatesTested = TotalTested;

	// Clear preview highlights
	ClearAllPreviews();

	// Fire lifecycle event
	OnMarqueeEnded(Result);

	// Broadcast L0 delegate
	OnMarqueeSelectionComplete.Broadcast(Result);

	// Reset state
	ResetState();

	return true;
}

void UMarqueeSelectionWidget_Base::HandleMouseCancel()
{
	if (bIsActive && (bIsDragging || !DragStartLocal.IsZero()))
	{
		CancelMarquee();
	}
}

// ================================================================
// VIRTUAL DEFAULTS
// ================================================================

bool UMarqueeSelectionWidget_Base::TestCandidateOverlap_Implementation(
	const FMarqueeCandidate& Candidate, FVector2D TopLeft, FVector2D BottomRight)
{
	// Screen-space AABB intersection test
	const FVector2D CandidateBR = Candidate.ScreenPosition + Candidate.ScreenSize;

	// No overlap if one rect is completely to the side of the other
	if (Candidate.ScreenPosition.X > BottomRight.X || CandidateBR.X < TopLeft.X)
	{
		return false;
	}
	if (Candidate.ScreenPosition.Y > BottomRight.Y || CandidateBR.Y < TopLeft.Y)
	{
		return false;
	}

	return true;
}

bool UMarqueeSelectionWidget_Base::ShouldActivateMarquee_Implementation(
	FVector2D LocalPosition, const FPointerEvent& PointerEvent)
{
	return PointerEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}

// ================================================================
// BLUEPRINT NATIVE EVENT DEFAULTS
// ================================================================

void UMarqueeSelectionWidget_Base::OnMarqueeStarted_Implementation(FVector2D StartPosition)
{
	// Default: no-op. Consumers override for custom behavior.
}

void UMarqueeSelectionWidget_Base::OnMarqueeUpdated_Implementation(
	FVector2D TopLeft, FVector2D BottomRight, int32 OverlappingCount)
{
	// Default: no-op.
}

void UMarqueeSelectionWidget_Base::OnMarqueeEnded_Implementation(const FMarqueeSelectionResult& Result)
{
	// Default: no-op. Consumers override to apply selection.
}

void UMarqueeSelectionWidget_Base::OnMarqueeCancelled_Implementation()
{
	// Default: no-op.
}

void UMarqueeSelectionWidget_Base::OnCandidatePreviewEnter_Implementation(UObject* Candidate)
{
	// Default: no-op. Consumers override for highlight effects.
}

void UMarqueeSelectionWidget_Base::OnCandidatePreviewExit_Implementation(UObject* Candidate)
{
	// Default: no-op. Consumers override to remove highlights.
}

// ================================================================
// INTERNAL HELPERS
// ================================================================

void UMarqueeSelectionWidget_Base::DetectModifierMode(const FPointerEvent& PointerEvent)
{
	if (!MarqueeConfig.bEnableModifierKeys)
	{
		CurrentModifierMode = FWWTagLibrary::UI_Selection_Mode_Replace();
		return;
	}

	const bool bShift = PointerEvent.IsShiftDown();
	const bool bCtrl = PointerEvent.IsControlDown();

	if (bShift && bCtrl)
	{
		CurrentModifierMode = FWWTagLibrary::UI_Selection_Mode_Subtractive();
	}
	else if (bShift)
	{
		CurrentModifierMode = FWWTagLibrary::UI_Selection_Mode_Additive();
	}
	else if (bCtrl)
	{
		CurrentModifierMode = FWWTagLibrary::UI_Selection_Mode_Toggle();
	}
	else
	{
		CurrentModifierMode = FWWTagLibrary::UI_Selection_Mode_Replace();
	}
}

void UMarqueeSelectionWidget_Base::UpdateMarqueeVisual(FVector2D TopLeft, FVector2D Size)
{
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CanvasSlot)
	{
		return;
	}

	CanvasSlot->SetPosition(TopLeft);
	CanvasSlot->SetSize(Size);
	CanvasSlot->SetAutoSize(false);
}

int32 UMarqueeSelectionWidget_Base::UpdateCandidateOverlaps(
	FVector2D TopLeft, FVector2D BottomRight, TArray<TWeakObjectPtr<UObject>>* OutOverlapping)
{
	TArray<FMarqueeCandidate> Candidates;
	GatherSelectableCandidates(Candidates);

	TSet<TWeakObjectPtr<UObject>> CurrentlyOverlapping;

	for (const FMarqueeCandidate& Candidate : Candidates)
	{
		if (!Candidate.IsValid())
		{
			continue;
		}

		if (TestCandidateOverlap(Candidate, TopLeft, BottomRight))
		{
			CurrentlyOverlapping.Add(Candidate.CandidateObject);

			if (OutOverlapping)
			{
				OutOverlapping->Add(Candidate.CandidateObject);
			}
		}
	}

	// Diff: fire enter/exit events
	// New overlaps (entered)
	for (const TWeakObjectPtr<UObject>& Obj : CurrentlyOverlapping)
	{
		if (!PreviouslyOverlapping.Contains(Obj) && Obj.IsValid())
		{
			OnCandidatePreviewEnter(Obj.Get());
		}
	}

	// Lost overlaps (exited)
	for (const TWeakObjectPtr<UObject>& Obj : PreviouslyOverlapping)
	{
		if (!CurrentlyOverlapping.Contains(Obj) && Obj.IsValid())
		{
			OnCandidatePreviewExit(Obj.Get());
		}
	}

	PreviouslyOverlapping = MoveTemp(CurrentlyOverlapping);

	return Candidates.Num();
}

void UMarqueeSelectionWidget_Base::ClearAllPreviews()
{
	for (const TWeakObjectPtr<UObject>& Obj : PreviouslyOverlapping)
	{
		if (Obj.IsValid())
		{
			OnCandidatePreviewExit(Obj.Get());
		}
	}
	PreviouslyOverlapping.Empty();
}

void UMarqueeSelectionWidget_Base::ResetState()
{
	bIsDragging = false;
	DragStartLocal = FVector2D::ZeroVector;
	DragCurrentLocal = FVector2D::ZeroVector;
	CurrentModifierMode = FGameplayTag();

	if (Border_SelectionBox)
	{
		Border_SelectionBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMarqueeSelectionWidget_Base::CancelMarquee()
{
	ClearAllPreviews();
	OnMarqueeCancelled();
	OnMarqueeSelectionCancelled.Broadcast();
	ResetState();
}
