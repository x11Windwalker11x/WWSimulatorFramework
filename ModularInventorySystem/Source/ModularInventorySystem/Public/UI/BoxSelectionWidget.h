// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/MarqueeSelectionWidget_Base.h"
#include "BoxSelectionWidget.generated.h"

// Forward declarations
class UInventoryGridWidget;

/**
 * Inventory-specific marquee selection widget.
 * Re-parents from UUserWidget to UMarqueeSelectionWidget_Base (V2.13.6).
 *
 * Overrides GatherSelectableCandidates to provide inventory grid slots.
 * Overrides OnMarqueeEnded to apply selection via InventoryWidgetManager.
 * Overrides OnCandidatePreviewEnter/Exit for slot highlight feedback.
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UBoxSelectionWidget : public UMarqueeSelectionWidget_Base
{
	GENERATED_BODY()

public:
	/** Set the owning grid widget (must be called before ActivateMarquee) */
	void SetOwningGrid(UInventoryGridWidget* InOwningGrid);

protected:
	virtual void NativeConstruct() override;

	// UMarqueeSelectionWidget_Base overrides
	virtual void GatherSelectableCandidates_Implementation(TArray<FMarqueeCandidate>& OutCandidates) override;
	virtual void OnMarqueeEnded_Implementation(const FMarqueeSelectionResult& Result) override;
	virtual void OnCandidatePreviewEnter_Implementation(UObject* Candidate) override;
	virtual void OnCandidatePreviewExit_Implementation(UObject* Candidate) override;

private:
	/** Weak reference to the owning inventory grid */
	TWeakObjectPtr<UInventoryGridWidget> OwningGrid;
};
