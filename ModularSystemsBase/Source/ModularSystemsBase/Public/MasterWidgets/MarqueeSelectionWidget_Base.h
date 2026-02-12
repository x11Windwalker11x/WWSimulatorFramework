// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "Lib/Data/ModularSystemsBase/MarqueeSelectionData.h"
#include "Delegates/ModularSystemsBase/MarqueeSelectionDelegates.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "MarqueeSelectionWidget_Base.generated.h"

/**
 * Abstract base class for marquee (box) selection widgets.
 * Provides reusable drag-to-select rectangle logic with modifier key support.
 *
 * V1: UI-space widget selection only. World-space (RTS/4x) deferred to V2.
 *
 * Consumers MUST override GatherSelectableCandidates() to provide selectable items.
 * Input is forwarded from the owning widget via HandleMouseDown/Move/Up.
 *
 * bAutoRegister defaults to false — this is an overlay, not a managed window.
 */
UCLASS(Abstract, Blueprintable)
class MODULARSYSTEMSBASE_API UMarqueeSelectionWidget_Base : public UManagedWidget_Master
{
	GENERATED_BODY()

public:
	UMarqueeSelectionWidget_Base(const FObjectInitializer& ObjectInitializer);

	// ================================================================
	// PUBLIC API
	// ================================================================

	/** Activate marquee on the given canvas. Call before forwarding input. */
	UFUNCTION(BlueprintCallable, Category = "Marquee Selection")
	void ActivateMarquee(UCanvasPanel* ParentCanvas);

	/** Deactivate marquee and collapse the widget */
	UFUNCTION(BlueprintCallable, Category = "Marquee Selection")
	void DeactivateMarquee();

	/** Is the marquee ready to receive input? */
	UFUNCTION(BlueprintPure, Category = "Marquee Selection")
	bool IsMarqueeActive() const { return bIsActive; }

	/** Has the user dragged past the threshold? */
	UFUNCTION(BlueprintPure, Category = "Marquee Selection")
	bool IsCurrentlyDragging() const { return bIsDragging; }

	/** Current modifier mode tag (UI.Selection.Mode.*) */
	UFUNCTION(BlueprintPure, Category = "Marquee Selection")
	FGameplayTag GetCurrentModifierMode() const { return CurrentModifierMode; }

	/** Update configuration at runtime */
	UFUNCTION(BlueprintCallable, Category = "Marquee Selection")
	void SetMarqueeConfig(const FMarqueeConfig& NewConfig);

	// ================================================================
	// INPUT FORWARDING (called by owning widget)
	// ================================================================

	/** Forward mouse button down. Returns true if consumed. */
	UFUNCTION(BlueprintCallable, Category = "Marquee Selection|Input")
	bool HandleMouseDown(FVector2D LocalPosition, const FPointerEvent& PointerEvent);

	/** Forward mouse move. Returns true if consumed (dragging). */
	UFUNCTION(BlueprintCallable, Category = "Marquee Selection|Input")
	bool HandleMouseMove(FVector2D LocalPosition, const FPointerEvent& PointerEvent);

	/** Forward mouse button up. Returns true if consumed. */
	UFUNCTION(BlueprintCallable, Category = "Marquee Selection|Input")
	bool HandleMouseUp(FVector2D LocalPosition, const FPointerEvent& PointerEvent);

	/** Cancel the current marquee operation */
	UFUNCTION(BlueprintCallable, Category = "Marquee Selection|Input")
	void HandleMouseCancel();

	// ================================================================
	// L0 DELEGATE INSTANCES (cross-plugin communication)
	// ================================================================

	/** Broadcast when marquee selection completes successfully */
	UPROPERTY(BlueprintAssignable, Category = "Marquee Selection|Delegates")
	FOnMarqueeSelectionComplete OnMarqueeSelectionComplete;

	/** Broadcast when marquee selection is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Marquee Selection|Delegates")
	FOnMarqueeCancelled OnMarqueeSelectionCancelled;

protected:
	// ================================================================
	// CONFIGURATION
	// ================================================================

	/** Marquee visual and behavior configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Marquee Selection")
	FMarqueeConfig MarqueeConfig;

	// ================================================================
	// BOUND WIDGET (Blueprint must have a UBorder named Border_SelectionBox)
	// ================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> Border_SelectionBox;

	// ================================================================
	// PURE VIRTUAL — consumers MUST override
	// ================================================================

	/**
	 * Gather all objects that can potentially be selected by the marquee.
	 * Consumer fills OutCandidates with screen-space positions and sizes.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Marquee Selection")
	void GatherSelectableCandidates(TArray<FMarqueeCandidate>& OutCandidates);
	virtual void GatherSelectableCandidates_Implementation(TArray<FMarqueeCandidate>& OutCandidates) PURE_VIRTUAL(UMarqueeSelectionWidget_Base::GatherSelectableCandidates_Implementation, );

	// ================================================================
	// VIRTUAL — consumers CAN override
	// ================================================================

	/**
	 * Test if a candidate overlaps the marquee rectangle.
	 * Default: screen-space AABB intersection.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Marquee Selection")
	bool TestCandidateOverlap(const FMarqueeCandidate& Candidate, FVector2D TopLeft, FVector2D BottomRight);
	virtual bool TestCandidateOverlap_Implementation(const FMarqueeCandidate& Candidate, FVector2D TopLeft, FVector2D BottomRight);

	/**
	 * Should the marquee activate for this input event?
	 * Default: left mouse button only.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Marquee Selection")
	bool ShouldActivateMarquee(FVector2D LocalPosition, const FPointerEvent& PointerEvent);
	virtual bool ShouldActivateMarquee_Implementation(FVector2D LocalPosition, const FPointerEvent& PointerEvent);

	// ================================================================
	// BLUEPRINT NATIVE EVENTS (lifecycle callbacks)
	// ================================================================

	/** Called when marquee drag begins (past threshold) */
	UFUNCTION(BlueprintNativeEvent, Category = "Marquee Selection|Events")
	void OnMarqueeStarted(FVector2D StartPosition);
	virtual void OnMarqueeStarted_Implementation(FVector2D StartPosition);

	/** Called every frame during marquee drag */
	UFUNCTION(BlueprintNativeEvent, Category = "Marquee Selection|Events")
	void OnMarqueeUpdated(FVector2D TopLeft, FVector2D BottomRight, int32 OverlappingCount);
	virtual void OnMarqueeUpdated_Implementation(FVector2D TopLeft, FVector2D BottomRight, int32 OverlappingCount);

	/** Called when marquee drag ends with a valid selection */
	UFUNCTION(BlueprintNativeEvent, Category = "Marquee Selection|Events")
	void OnMarqueeEnded(const FMarqueeSelectionResult& Result);
	virtual void OnMarqueeEnded_Implementation(const FMarqueeSelectionResult& Result);

	/** Called when marquee is cancelled (too small, right-click, etc.) */
	UFUNCTION(BlueprintNativeEvent, Category = "Marquee Selection|Events")
	void OnMarqueeCancelled();
	virtual void OnMarqueeCancelled_Implementation();

	/** Called when a candidate enters the marquee preview area */
	UFUNCTION(BlueprintNativeEvent, Category = "Marquee Selection|Events")
	void OnCandidatePreviewEnter(UObject* Candidate);
	virtual void OnCandidatePreviewEnter_Implementation(UObject* Candidate);

	/** Called when a candidate exits the marquee preview area */
	UFUNCTION(BlueprintNativeEvent, Category = "Marquee Selection|Events")
	void OnCandidatePreviewExit(UObject* Candidate);
	virtual void OnCandidatePreviewExit_Implementation(UObject* Candidate);

private:
	// ================================================================
	// INTERNAL STATE
	// ================================================================

	/** Whether marquee is activated and ready for input */
	bool bIsActive = false;

	/** Whether user has dragged past the threshold */
	bool bIsDragging = false;

	/** Screen position where drag started (local to parent canvas) */
	FVector2D DragStartLocal = FVector2D::ZeroVector;

	/** Current drag position (local to parent canvas) */
	FVector2D DragCurrentLocal = FVector2D::ZeroVector;

	/** Currently active modifier mode tag */
	FGameplayTag CurrentModifierMode;

	/** Set of candidates currently under the marquee (for enter/exit diff) */
	TSet<TWeakObjectPtr<UObject>> PreviouslyOverlapping;

	/** Cached reference to the parent canvas panel */
	TWeakObjectPtr<UCanvasPanel> CachedParentCanvas;

	// ================================================================
	// INTERNAL HELPERS
	// ================================================================

	/** Detect modifier keys and set CurrentModifierMode tag */
	void DetectModifierMode(const FPointerEvent& PointerEvent);

	/** Update the visual rectangle on the canvas */
	void UpdateMarqueeVisual(FVector2D TopLeft, FVector2D Size);

	/** Perform candidate gathering and overlap testing, fire preview enter/exit */
	int32 UpdateCandidateOverlaps(FVector2D TopLeft, FVector2D BottomRight, TArray<TWeakObjectPtr<UObject>>* OutOverlapping = nullptr);

	/** Clear all preview highlights and reset tracking */
	void ClearAllPreviews();

	/** Reset all internal state to defaults */
	void ResetState();

	/** Cancel the current operation and broadcast */
	void CancelMarquee();
};
