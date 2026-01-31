// MIIS_RightClickContextMenu.cpp
// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HelperUI/ContextMenu/MIIS_RightClickContextMenu.h"
#include "UI/HelperUI/ContextMenu/DropQuantityWidget.h"
#include "UI/HelperUI/ContextMenu/SplitQuantityWidget.h"
#include "Windwalker_Productions_SharedDefaults/Public/WW_TagLibrary.h"
#include "Subsystems/WidgetManager.h"
#include "Components/InventoryComponent.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Actors/Interactables/Item/Data/ItemData.h"
#include "Logging/InteractableInventoryLogging.h"

UMIIS_RightClickContextMenu::UMIIS_RightClickContextMenu(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WidgetCategoryTag = FWWTagLibrary::UI_Widget_Category_ContextMenu();
    bAutoRegister = true;
}

void UMIIS_RightClickContextMenu::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Bind standard buttons
    if (Button_Use)
    {
        Button_Use->OnClicked.AddDynamic(this, &UMIIS_RightClickContextMenu::OnUseClicked);
    }
    
    if (Button_Unequip)
    {
        Button_Unequip->OnClicked.AddDynamic(this, &UMIIS_RightClickContextMenu::OnUnequipClicked);
    }
    
    if (Button_Drop)
    {
        Button_Drop->OnClicked.AddDynamic(this, &UMIIS_RightClickContextMenu::OnDropClicked);
    }
    
    if (Button_Split)
    {
        Button_Split->OnClicked.AddDynamic(this, &UMIIS_RightClickContextMenu::OnSplitClicked);
    }
    
    if (Button_Examine)
    {
        Button_Examine->OnClicked.AddDynamic(this, &UMIIS_RightClickContextMenu::OnExamineClicked);
    }
    
    if (Button_Favorite)
    {
        Button_Favorite->OnClicked.AddDynamic(this, &UMIIS_RightClickContextMenu::OnFavoriteClicked);
    }
    
    // Bind combine/attachment buttons
    if (Button_Combine)
    {
        Button_Combine->OnClicked.AddDynamic(this, &UMIIS_RightClickContextMenu::OnCombineClicked);
    }
    
    if (Button_CombineWith)
    {
        Button_CombineWith->OnClicked.AddDynamic(this, &UMIIS_RightClickContextMenu::OnCombineWithClicked);
    }
    
    if (Button_AttachTo)
    {
        Button_AttachTo->OnClicked.AddDynamic(this, &UMIIS_RightClickContextMenu::OnAttachToClicked);
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("📋 Context menu NativeConstruct complete"));
}

void UMIIS_RightClickContextMenu::NativeDestruct()
{
    // Unregister from WidgetManager
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UWidgetManager* WidgetMgr = UWidgetManager::Get(PC))
        {
            WidgetMgr->CloseActiveContextMenu();
        }
    }
    
    Super::NativeDestruct();
}

FReply UMIIS_RightClickContextMenu::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // Close menu on click outside
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) || 
        InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
    {
        FVector2D LocalMousePos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        FVector2D LocalSize = InGeometry.GetLocalSize();
        
        if (LocalMousePos.X < 0 || LocalMousePos.Y < 0 ||
            LocalMousePos.X > LocalSize.X || LocalMousePos.Y > LocalSize.Y)
        {
            CloseMenu();
            return FReply::Handled();
        }
    }
    
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UMIIS_RightClickContextMenu::InitializeMenu(FGameplayTag InInventoryType, int32 InSlotIndex, UInventoryComponent* InInventoryComponent)
{
    ClearMenuItems();
    
    if (!InInventoryComponent)
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("InitializeMenu failed - Invalid InventoryComponent"));
        return;
    }
    
    TargetInventoryType = InInventoryType;
    TargetSlotIndex = InSlotIndex;
    TargetInventoryComp = InInventoryComponent;
    
    // Get slot data using GetSlot
    FInventorySlot SlotData;
    if (!InInventoryComponent->GetSlot(InInventoryType, InSlotIndex, SlotData) || SlotData.IsEmpty())
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("InitializeMenu failed - Empty slot or invalid index"));
        return;
    }
    
    // Get item data
    FItemData ItemData;
    if (!InInventoryComponent->GetItemData(SlotData.ItemID, ItemData))
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("InitializeMenu failed - Could not get ItemData for %s"), *SlotData.ItemID.ToString());
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("📋 Context menu initialized for slot %d - Item: %s, Qty: %d"),
        InSlotIndex, *SlotData.ItemID.ToString(), SlotData.Quantity);
    
    // ============================================================================
    // Configure button visibility using InventoryComponent query helpers
    // (AAA professional standard - all game logic in InventoryComponent)
    // ============================================================================
    
    // Use/Equip button - show if item is usable or equippable
    if (Button_Use)
    {
        bool bShowUse = InInventoryComponent->IsSlotUsable(InInventoryType, InSlotIndex) ||
                        InInventoryComponent->IsSlotEquippable(InInventoryType, InSlotIndex);
        Button_Use->SetVisibility(bShowUse ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    
    // Unequip button - show only if item is currently equipped
    if (Button_Unequip)
    {
        bool bShowUnequip = InInventoryComponent->IsSlotEquipped(InInventoryType, InSlotIndex);
        Button_Unequip->SetVisibility(bShowUnequip ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    
    // Drop button - show if item is droppable
    if (Button_Drop)
    {
        bool bShowDrop = InInventoryComponent->IsSlotDroppable(InInventoryType, InSlotIndex);
        Button_Drop->SetVisibility(bShowDrop ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    
    // Split button - show if stack size > 1
    if (Button_Split)
    {
        bool bShowSplit = SlotData.Quantity > 1;
        Button_Split->SetVisibility(bShowSplit ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    
    // Examine button - always visible
    if (Button_Examine)
    {
        Button_Examine->SetVisibility(ESlateVisibility::Visible);
    }
    
    // Favorite button - always visible
    if (Button_Favorite)
    {
        Button_Favorite->SetVisibility(ESlateVisibility::Visible);
    }
    
    // ============================================================================
    // COMBINE SYSTEM BUTTONS
    // ============================================================================
    
    // Combine button - show if there are similar items to combine with
    if (Button_Combine)
    {
        bool bShowCombine = InInventoryComponent->IsSlotCombinable(InInventoryType, InSlotIndex);
        Button_Combine->SetVisibility(bShowCombine ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("  Combine button: %s"), bShowCombine ? TEXT("Visible") : TEXT("Hidden"));
    }
    
    // CombineWith vs AttachTo - mutually exclusive based on attachment tag
    if (Button_CombineWith && Button_AttachTo)
    {
        bool bIsAttachment = InInventoryComponent->IsSlotAttachment(InInventoryType, InSlotIndex);
        
        // Show AttachTo if item is an attachment, otherwise show CombineWith
        Button_CombineWith->SetVisibility(bIsAttachment ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
        Button_AttachTo->SetVisibility(bIsAttachment ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("  IsAttachment: %s - CombineWith: %s, AttachTo: %s"),
            bIsAttachment ? TEXT("Yes") : TEXT("No"),
            bIsAttachment ? TEXT("Hidden") : TEXT("Visible"),
            bIsAttachment ? TEXT("Visible") : TEXT("Hidden"));
    }
}

void UMIIS_RightClickContextMenu::ClearMenuItems()
{
    // Hide all optional buttons
    if (Button_Use) Button_Use->SetVisibility(ESlateVisibility::Collapsed);
    if (Button_Unequip) Button_Unequip->SetVisibility(ESlateVisibility::Collapsed);
    if (Button_Drop) Button_Drop->SetVisibility(ESlateVisibility::Collapsed);
    if (Button_Split) Button_Split->SetVisibility(ESlateVisibility::Collapsed);
    if (Button_Combine) Button_Combine->SetVisibility(ESlateVisibility::Collapsed);
    if (Button_CombineWith) Button_CombineWith->SetVisibility(ESlateVisibility::Collapsed);
    if (Button_AttachTo) Button_AttachTo->SetVisibility(ESlateVisibility::Collapsed);
}

void UMIIS_RightClickContextMenu::CloseMenu()
{
    RemoveFromParent();
}

// ============================================================================
// BUTTON CLICK HANDLERS
// ============================================================================

void UMIIS_RightClickContextMenu::OnUseClicked()
{
    if (!TargetInventoryComp)
    {
        CloseMenu();
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("🎯 Use clicked for slot %d"), TargetSlotIndex);
    
    TargetInventoryComp->UseItem(TargetInventoryType, TargetSlotIndex);
    CloseMenu();
}

void UMIIS_RightClickContextMenu::OnUnequipClicked()
{
    if (!TargetInventoryComp)
    {
        CloseMenu();
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("🎯 Unequip clicked for slot %d"), TargetSlotIndex);
    
    TargetInventoryComp->UnequipItem(TargetInventoryType, TargetSlotIndex);
    CloseMenu();
}

void UMIIS_RightClickContextMenu::OnDropClicked()
{
    if (!TargetInventoryComp)
    {
        CloseMenu();
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("🎯 Drop clicked for slot %d"), TargetSlotIndex);
    
    TargetInventoryComp->DropItem(TargetInventoryType, TargetSlotIndex);
    CloseMenu();
}

void UMIIS_RightClickContextMenu::OnSplitClicked()
{
    if (!TargetInventoryComp)
    {
        CloseMenu();
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("🎯 Split clicked for slot %d"), TargetSlotIndex);
    
    // Show split quantity widget
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UWidgetManager* WidgetMgr = UWidgetManager::Get(PC))
        {
            WidgetMgr->ShowSplitQuantityWidget(TargetInventoryType, TargetSlotIndex);
        }
    }
    
    CloseMenu();
}

void UMIIS_RightClickContextMenu::OnExamineClicked()
{
    if (!TargetInventoryComp)
    {
        CloseMenu();
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("🎯 Examine clicked for slot %d"), TargetSlotIndex);
    
    // Show examine UI
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UWidgetManager* WidgetMgr = UWidgetManager::Get(PC))
        {
            WidgetMgr->ShowExamineWidget(TargetInventoryType, TargetSlotIndex);
        }
    }
    
    CloseMenu();
}

void UMIIS_RightClickContextMenu::OnFavoriteClicked()
{
    if (!TargetInventoryComp)
    {
        CloseMenu();
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("🎯 Favorite toggled for slot %d"), TargetSlotIndex);
    
    TargetInventoryComp->ToggleFavorite(TargetInventoryType, TargetSlotIndex);
    CloseMenu();
}

void UMIIS_RightClickContextMenu::OnCombineClicked()
{
    if (!TargetInventoryComp)
    {
        CloseMenu();
        return;
    }
    
    // Get slot data for logging
    FInventorySlot SlotData;
    if (TargetInventoryComp->GetSlot(TargetInventoryType, TargetSlotIndex, SlotData))
    {
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("🔀 Combine clicked for slot %d - Item: %s, Rarity: %d"),
            TargetSlotIndex, *SlotData.ItemID.ToString(), SlotData.Rarity);
    }
    
    // Call combine function on inventory component
    TargetInventoryComp->CombineSimilarItems(TargetInventoryType, TargetSlotIndex);
    
    CloseMenu();
}

void UMIIS_RightClickContextMenu::OnCombineWithClicked()
{
    if (!TargetInventoryComp)
    {
        CloseMenu();
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("🧪 Combine With (Quest) clicked for slot %d"), TargetSlotIndex);
    
    // Enter quest combine mode - highlight valid slots
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UWidgetManager* WidgetMgr = UWidgetManager::Get(PC))
        {
            WidgetMgr->EnterQuestCombineMode(TargetInventoryType, TargetSlotIndex);
        }
    }
    
    CloseMenu();
}

void UMIIS_RightClickContextMenu::OnAttachToClicked()
{
    if (!TargetInventoryComp)
    {
        CloseMenu();
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("🔧 Attach To clicked for slot %d"), TargetSlotIndex);
    
    // Enter attachment mode - dim inappropriate slots
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UWidgetManager* WidgetMgr = UWidgetManager::Get(PC))
        {
            WidgetMgr->EnterAttachmentMode(TargetInventoryType, TargetSlotIndex);
        }
    }
    
    CloseMenu();
}