// SplitQuantityWidget.cpp
#include "UI/HelperUI/ContextMenu/SplitQuantitiyWidget.h"
#include "Windwalker_Productions_SharedDefaults.h"
#include "Components/InventoryComponent.h"
#include "Subsystems/InventoryWidgetManager.h"

USplitQuantityWidget::USplitQuantityWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WidgetCategoryTag = FWWTagLibrary::UI_Widget_Category_ContextMenu();
    bAutoRegister = true;
}


void USplitQuantityWidget::NativeConstruct()
{
    Super::NativeConstruct();
    APlayerController* OwningPlayer = GetOwningPlayer();
    if (OwningPlayer && OwningPlayer->IsLocalPlayerController())
    {
        PC = OwningPlayer;
    }
}

void USplitQuantityWidget::InitializeSplitWidget(UInventoryComponent* InvComp, FGameplayTag InvType, int32 SlotIndex, int32 InMaxQuantity)
{
    TargetInventoryComp = InvComp;
    TargetInventoryType = InvType;
    TargetSlotIndex = SlotIndex;
    
    // CRITICAL: MaxQuantity must be at least 1 less than actual quantity (can't split all)
    // If slot has 2 items, can split 1. If slot has 10 items, can split up to 9.
    MaxQuantity = FMath::Max(1, InMaxQuantity - 1);
    SelectedQuantity = 1;
    bIsMultiSelection = false;
    
    if (Slider_Quantity)
    {
        Slider_Quantity->SetMinValue(1.0f);
        Slider_Quantity->SetMaxValue((float)MaxQuantity);
        Slider_Quantity->SetValue(1.0f);
    }
    
    if (EditableText_Quantity)
    {
        EditableText_Quantity->SetText(FText::AsNumber(SelectedQuantity));
        EditableText_Quantity->SetIsReadOnly(true);
    }
}

void USplitQuantityWidget::InitializeForMultiSelection(UInventoryComponent* InvComp, TArray<FInventorySlotReference> InSlots, int32 MinQuantity)
{
    TargetInventoryComp = InvComp;
    TargetSlots = InSlots;
    bIsMultiSelection = true;
    
    // CRITICAL: MaxQuantity = MinQuantity - 1 (can't split all from any slot)
    MaxQuantity = FMath::Max(1, MinQuantity - 1);
    SelectedQuantity = 1;
    
    if (Slider_Quantity)
    {
        Slider_Quantity->SetMinValue(1.0f);
        Slider_Quantity->SetMaxValue((float)MaxQuantity);
        Slider_Quantity->SetValue(1.0f);
    }
    
    if (EditableText_Quantity)
    {
        EditableText_Quantity->SetText(FText::AsNumber(SelectedQuantity));
        EditableText_Quantity->SetIsReadOnly(true);
    }
}

void USplitQuantityWidget::ConfirmSplit()
{
    if (!TargetInventoryComp)
    {
        UE_LOG(LogTemp, Error, TEXT("SplitQuantityWidget::ConfirmSplit - No TargetInventoryComp!"));
        RemoveFromParent();
        return;
    }
    
    if (bIsMultiSelection)
    {
        for (int32 i = 0; i < TargetSlots.Num(); i++)
        {
            const FInventorySlotReference& SlotRef = TargetSlots[i];
            
            FInventorySlot SourceSlot;
            if (!TargetInventoryComp->GetSlot(SlotRef.InventoryType, SlotRef.SlotIndex, SourceSlot))
                continue;
            
            if (SourceSlot.Quantity <= 1)
                continue;
            
            int32 ActualSplitAmount = (SelectedQuantity == 1) ? 1 : FMath::Clamp(SelectedQuantity, 1, SourceSlot.Quantity - 1);
            
            TArray<FInventorySlot>* InventoryArray = TargetInventoryComp->GetInventoryArray(SlotRef.InventoryType);
            if (!InventoryArray) continue;
            
            int32 TargetSlot = -1;
            
            // Find empty forward
            for (int32 j = SlotRef.SlotIndex + 1; j < InventoryArray->Num(); j++)
            {
                if ((*InventoryArray)[j].IsEmpty())
                {
                    TargetSlot = j;
                    break;
                }
            }
            
            // Find empty backward
            if (TargetSlot == -1)
            {
                for (int32 j = SlotRef.SlotIndex - 1; j >= 0; j--)
                {
                    if ((*InventoryArray)[j].IsEmpty())
                    {
                        TargetSlot = j;
                        break;
                    }
                }
            }
            
            // Find same ItemID
            if (TargetSlot == -1)
            {
                for (int32 j = 0; j < InventoryArray->Num(); j++)
                {
                    if (j == SlotRef.SlotIndex) continue;
                    if ((*InventoryArray)[j].ItemID == SourceSlot.ItemID && !(*InventoryArray)[j].IsEmpty())
                    {
                        TargetSlot = j;
                        break;
                    }
                }
            }
            
            if (TargetSlot == -1) continue;
            
            TargetInventoryComp->SplitStack(SlotRef.InventoryType, SlotRef.SlotIndex, ActualSplitAmount, SlotRef.InventoryType, TargetSlot);
        }
    }
    else
    {
        FInventorySlot SourceSlot;
        if (!TargetInventoryComp->GetSlot(TargetInventoryType, TargetSlotIndex, SourceSlot))
        {
            RemoveFromParent();
            return;
        }
        
        if (SourceSlot.Quantity <= 1)
        {
            RemoveFromParent();
            return;
        }
        
        int32 ActualSplitAmount = (SelectedQuantity == 1) ? 1 : FMath::Clamp(SelectedQuantity, 1, SourceSlot.Quantity - 1);
        
        TArray<FInventorySlot>* InventoryArray = TargetInventoryComp->GetInventoryArray(TargetInventoryType);
        if (!InventoryArray)
        {
            RemoveFromParent();
            return;
        }
        
        int32 TargetSlot = -1;
        
        // Find empty forward
        for (int32 i = TargetSlotIndex + 1; i < InventoryArray->Num(); i++)
        {
            if ((*InventoryArray)[i].IsEmpty())
            {
                TargetSlot = i;
                break;
            }
        }
        
        // Find empty backward
        if (TargetSlot == -1)
        {
            for (int32 i = TargetSlotIndex - 1; i >= 0; i--)
            {
                if ((*InventoryArray)[i].IsEmpty())
                {
                    TargetSlot = i;
                    break;
                }
            }
        }
        
        // Find same ItemID
        if (TargetSlot == -1)
        {
            for (int32 i = 0; i < InventoryArray->Num(); i++)
            {
                if (i == TargetSlotIndex) continue;
                if ((*InventoryArray)[i].ItemID == SourceSlot.ItemID && !(*InventoryArray)[i].IsEmpty())
                {
                    TargetSlot = i;
                    break;
                }
            }
        }
        
        if (TargetSlot == -1)
        {
            RemoveFromParent();
            return;
        }
        
        TargetInventoryComp->SplitStack(TargetInventoryType, TargetSlotIndex, ActualSplitAmount, TargetInventoryType, TargetSlot);
    }
    
    RemoveFromParent();
}

void USplitQuantityWidget::Cancel()
{
    UE_LOG(LogTemp, Warning, TEXT("SplitQuantityWidget::Cancel"));
    
    if (!PC)
    {
        UDebugSubsystem::PrintDebug(this, WWDebugTags::Debug_Widget(), 
            TEXT("Can not find local player for widget: SplitQuantity"), false, EDebugVerbosity::Critical);
    }
    
    if (PC)
    {
        FInputModeGameAndUI InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
    
    RemoveFromParent();
}

void USplitQuantityWidget::OnSliderValueChanged(float Value)
{
    SelectedQuantity = FMath::Clamp(FMath::RoundToInt(Value), 1, MaxQuantity);
    
    if (EditableText_Quantity)
    {
        EditableText_Quantity->SetText(FText::AsNumber(SelectedQuantity));
    }
}

FReply USplitQuantityWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // Close widget if clicked outside bounds
    if (!InGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
    {
        RemoveFromParent();
        return FReply::Handled();
    }
    
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}