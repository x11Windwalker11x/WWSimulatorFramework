// Copyright Windwalker Productions. All Rights Reserved.

#include "UI/BoxSelectionWidget.h"
#include "UI/InventoryGridWidget.h"
#include "UI/InventorySlotWidget.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Subsystems/InventoryWidgetManager.h"
#include "Components/Border.h"
#include "Components/UniformGridPanel.h"

void UBoxSelectionWidget::SetOwningGrid(UInventoryGridWidget* InOwningGrid)
{
	OwningGrid = InOwningGrid;
}

void UBoxSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Preserve original visual style: orange border, 2px padding
	if (Border_SelectionBox)
	{
		FLinearColor BorderColor(1.0f, 0.7f, 0.0f, 0.9f);
		Border_SelectionBox->SetBrushColor(BorderColor);
		Border_SelectionBox->SetPadding(FMargin(2.0f));
	}
}

void UBoxSelectionWidget::GatherSelectableCandidates_Implementation(TArray<FMarqueeCandidate>& OutCandidates)
{
	if (!OwningGrid.IsValid())
	{
		return;
	}

	UUniformGridPanel* GridPanel = OwningGrid->GridPanel;
	if (!GridPanel)
	{
		return;
	}

	for (int32 i = 0; i < GridPanel->GetChildrenCount(); ++i)
	{
		UInventorySlotWidget* SlotWidget = Cast<UInventorySlotWidget>(GridPanel->GetChildAt(i));
		if (!SlotWidget)
		{
			continue;
		}

		FGeometry SlotGeometry = SlotWidget->GetCachedGeometry();
		FVector2D AbsPos = SlotGeometry.GetAbsolutePosition();
		FVector2D AbsSize = SlotGeometry.GetAbsoluteSize();

		// Convert absolute to local space relative to the parent canvas
		// The marquee operates in local canvas space, so candidates need matching coords
		UCanvasPanel* ParentCanvas = nullptr;
		UWidget* Walker = OwningGrid.Get();
		while (Walker)
		{
			if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(Walker))
			{
				ParentCanvas = Canvas;
				break;
			}
			Walker = Walker->GetParent();
		}

		FVector2D LocalPos = AbsPos;
		if (ParentCanvas)
		{
			FGeometry CanvasGeometry = ParentCanvas->GetCachedGeometry();
			LocalPos = CanvasGeometry.AbsoluteToLocal(AbsPos);
		}

		FMarqueeCandidate Candidate;
		Candidate.CandidateObject = SlotWidget;
		Candidate.ScreenPosition = LocalPos;
		Candidate.ScreenSize = AbsSize;
		Candidate.SelectionPriority = 0;

		OutCandidates.Add(Candidate);
	}
}

void UBoxSelectionWidget::OnMarqueeEnded_Implementation(const FMarqueeSelectionResult& Result)
{
	if (!OwningGrid.IsValid())
	{
		return;
	}

	APlayerController* PC = OwningGrid->GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(PC);
	if (!WidgetMgr)
	{
		return;
	}

	const FGameplayTag& Mode = Result.ModifierMode;

	// Apply selection based on modifier mode
	if (Mode == FWWTagLibrary::UI_Selection_Mode_Replace())
	{
		WidgetMgr->ClearSelection();
		for (const TWeakObjectPtr<UObject>& Obj : Result.SelectedObjects)
		{
			if (UInventorySlotWidget* Slot = Cast<UInventorySlotWidget>(Obj.Get()))
			{
				if (Slot->HasItem())
				{
					WidgetMgr->SelectSlot(Slot->InventoryType, Slot->SlotIndex);
				}
			}
		}
	}
	else if (Mode == FWWTagLibrary::UI_Selection_Mode_Additive())
	{
		for (const TWeakObjectPtr<UObject>& Obj : Result.SelectedObjects)
		{
			if (UInventorySlotWidget* Slot = Cast<UInventorySlotWidget>(Obj.Get()))
			{
				if (Slot->HasItem())
				{
					WidgetMgr->SelectSlot(Slot->InventoryType, Slot->SlotIndex);
				}
			}
		}
	}
	else if (Mode == FWWTagLibrary::UI_Selection_Mode_Toggle())
	{
		for (const TWeakObjectPtr<UObject>& Obj : Result.SelectedObjects)
		{
			if (UInventorySlotWidget* Slot = Cast<UInventorySlotWidget>(Obj.Get()))
			{
				if (Slot->HasItem())
				{
					WidgetMgr->ToggleSlotSelection(Slot->InventoryType, Slot->SlotIndex);
				}
			}
		}
	}
	else if (Mode == FWWTagLibrary::UI_Selection_Mode_Subtractive())
	{
		for (const TWeakObjectPtr<UObject>& Obj : Result.SelectedObjects)
		{
			if (UInventorySlotWidget* Slot = Cast<UInventorySlotWidget>(Obj.Get()))
			{
				WidgetMgr->DeselectSlot(Slot->InventoryType, Slot->SlotIndex);
			}
		}
	}
}

void UBoxSelectionWidget::OnCandidatePreviewEnter_Implementation(UObject* Candidate)
{
	if (UInventorySlotWidget* Slot = Cast<UInventorySlotWidget>(Candidate))
	{
		Slot->SetHighlight(true, FLinearColor(1.0f, 0.7f, 0.0f, 0.5f));
	}
}

void UBoxSelectionWidget::OnCandidatePreviewExit_Implementation(UObject* Candidate)
{
	if (UInventorySlotWidget* Slot = Cast<UInventorySlotWidget>(Candidate))
	{
		Slot->SetHighlight(false);
	}
}
