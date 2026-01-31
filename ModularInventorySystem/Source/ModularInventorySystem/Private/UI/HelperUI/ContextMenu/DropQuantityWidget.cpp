// DropQuantityWidget.cpp
#include "UI/HelperUI/ContextMenu/DropQuantityWidget.h"
#include "Components/InventoryComponent.h"
#include "Subsystems/WidgetManager.h"
#include "Components/Slider.h"
#include "Components/EditableText.h"

UDropQuantityWidget::UDropQuantityWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetCategoryTag = FWWTagLibrary::UI_Widget_Category_ContextMenu();
	bAutoRegister = true;
}


void UDropQuantityWidget::InitializeQuantityWidget(int32 InMaxQuantity)
{
	MaxQuantity = FMath::Max(1, InMaxQuantity);
	SelectedQuantity = 1;
	bIsMultiSelection = false;
	
	// Set slider range
	if (Slider_Quantity)
	{
		Slider_Quantity->SetMinValue(1.0f);
		Slider_Quantity->SetMaxValue((float)MaxQuantity);
		Slider_Quantity->SetValue(1.0f);
	}
	
	// Update text
	if (EditableText_Quantity)
	{
		EditableText_Quantity->SetText(FText::AsNumber(SelectedQuantity));
	}
}

void UDropQuantityWidget::InitializeQuantityWidget(UInventoryComponent* InvComp, FGameplayTag InvType, int32 SlotIndex, int32 InMaxQuantity)
{
	TargetInventoryComp = InvComp;
	TargetInventoryType = InvType;
	TargetSlotIndex = SlotIndex;
	MaxQuantity = FMath::Max(1, InMaxQuantity);
	SelectedQuantity = 1;
	bIsMultiSelection = false;
	
	// Set slider range
	if (Slider_Quantity)
	{
		Slider_Quantity->SetMinValue(1.0f);
		Slider_Quantity->SetMaxValue((float)MaxQuantity);
		Slider_Quantity->SetValue(1.0f);
	}
	
	// Update text
	if (EditableText_Quantity)
	{
		EditableText_Quantity->SetText(FText::AsNumber(SelectedQuantity));
	}
}

void UDropQuantityWidget::InitializeForMultiSelection(UInventoryComponent* InvComp, TArray<FInventorySlotReference> InSlots, int32 MinQuantity)
{
	TargetInventoryComp = InvComp;
	TargetSlots = InSlots;
	bIsMultiSelection = true;
	MaxQuantity = FMath::Max(1, MinQuantity);
	SelectedQuantity = 1;
	
	// Set slider range (slider max = minimum quantity across all slots)
	if (Slider_Quantity)
	{
		Slider_Quantity->SetMinValue(1.0f);
		Slider_Quantity->SetMaxValue((float)MaxQuantity);
		Slider_Quantity->SetValue(1.0f);
	}
	
	// Update text
	if (EditableText_Quantity)
	{
		EditableText_Quantity->SetText(FText::AsNumber(SelectedQuantity));
	}
}

void UDropQuantityWidget::ConfirmDrop()
{
	if (!TargetInventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("DropQuantityWidget::ConfirmDrop - No TargetInventoryComp"));
		return;
	}
	
	if (bIsMultiSelection)
	{
		// Multi-selection: Drop SelectedQuantity from each slot
		UE_LOG(LogTemp, Log, TEXT("DropQuantityWidget::ConfirmDrop - Multi-selection, dropping %d items from %d slots"), SelectedQuantity, TargetSlots.Num());
		
		for (const FInventorySlotReference& SlotRef : TargetSlots)
		{
			TargetInventoryComp->DropItem(SlotRef.InventoryType, SlotRef.SlotIndex, SelectedQuantity);
		}
	}
	else
	{
		// Single slot: Drop SelectedQuantity
		UE_LOG(LogTemp, Log, TEXT("DropQuantityWidget::ConfirmDrop - Single slot, dropping %d items from slot %d"), SelectedQuantity, TargetSlotIndex);
		
		TargetInventoryComp->DropItem(TargetInventoryType, TargetSlotIndex, SelectedQuantity);
	}
	
	OnQuantityConfirmed.Broadcast(SelectedQuantity);
	RemoveFromParent();
}

void UDropQuantityWidget::ConfirmDropAll()
{
	if (!TargetInventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("DropQuantityWidget::ConfirmDropAll - No TargetInventoryComp"));
		return;
	}
	
	if (bIsMultiSelection)
	{
		// FIXED: Drop FULL quantity from each slot (not MinQuantity)
		// Each slot may have different quantities, so we need to get the actual quantity per slot
		UE_LOG(LogTemp, Log, TEXT("DropQuantityWidget::ConfirmDropAll - Multi-selection, dropping ALL items from %d slots"), TargetSlots.Num());
		
		for (const FInventorySlotReference& SlotRef : TargetSlots)
		{
			// Get the actual quantity in THIS specific slot
			FInventorySlot SlotData;
			if (TargetInventoryComp->GetSlot(SlotRef.InventoryType, SlotRef.SlotIndex, SlotData))
			{
				// Drop the FULL quantity of this particular slot
				int32 ActualQuantity = SlotData.Quantity;
				UE_LOG(LogTemp, Log, TEXT("  - Dropping %d items from slot %d (Type: %s)"), 
					ActualQuantity, SlotRef.SlotIndex, *SlotRef.InventoryType.ToString());
				
				TargetInventoryComp->DropItem(SlotRef.InventoryType, SlotRef.SlotIndex, ActualQuantity);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("  - Failed to get slot data for slot %d"), SlotRef.SlotIndex);
			}
		}
	}
	else
	{
		// Single slot: Drop MaxQuantity (which is the full quantity of the slot)
		UE_LOG(LogTemp, Log, TEXT("DropQuantityWidget::ConfirmDropAll - Single slot, dropping %d items from slot %d"), MaxQuantity, TargetSlotIndex);
		
		TargetInventoryComp->DropItem(TargetInventoryType, TargetSlotIndex, MaxQuantity);
	}
	
	// Broadcast MaxQuantity for consistency (even though we dropped different amounts per slot in multi-selection)
	OnQuantityConfirmed.Broadcast(MaxQuantity);
	RemoveFromParent();
}

void UDropQuantityWidget::Cancel()
{
	UE_LOG(LogTemp, Log, TEXT("DropQuantityWidget::Cancel - User cancelled drop"));
	
	OnQuantityCancelled.Broadcast();
	RemoveFromParent();
}

void UDropQuantityWidget::OnSliderValueChanged(float Value)
{
	SelectedQuantity = FMath::Clamp(FMath::RoundToInt(Value), 1, MaxQuantity);
	
	// Update editable text to reflect slider value
	if (EditableText_Quantity)
	{
		EditableText_Quantity->SetText(FText::AsNumber(SelectedQuantity));
	}
	
	// Optional: Log for debugging
	// UE_LOG(LogTemp, VeryVerbose, TEXT("DropQuantityWidget::OnSliderValueChanged - New quantity: %d"), SelectedQuantity);
}