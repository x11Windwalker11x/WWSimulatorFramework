// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InventorySlotWidget.h"
#include "UI/HelperUI/ContextMenu/MIIS_RightClickContextMenu.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "InstanceCulling/InstanceCullingContext.h"
#include "Windwalker_Productions_SharedDefaults/Public/WW_TagLibrary.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Subsystems/InventoryWidgetManager.h"

// ============================================================================
// WIDGET LIFECYCLE
// ============================================================================


void UInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Set up visual defaults (one-time setup)
    InitializeVisuals();
    
    // Bind delegate if component is set
    BindToInventoryComponent();

    // Bind to WidgetManager selection changes
    if (APlayerController* PlayerController = GetOwningPlayer())
    {
        if (UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(PlayerController))
        {
            WidgetMgr->OnSelectionChanged.AddDynamic(this, &UInventorySlotWidget::OnSelectionChanged);
            
            UE_LOG(LogInventoryInteractableSystem, Log, 
                TEXT("✅ Slot %d bound to WidgetManager selection changes"), SlotIndex);
        }
    }
    
    // Store original opacity for drag operations
    if (Border_SlotBackground)
    {
        OriginalOpacity = Border_SlotBackground->GetRenderOpacity();
    }
}

void UInventorySlotWidget::InitializeVisuals()
{
    // Set default states for UI elements
    if (Border_SlotBackground)
    {
        Border_SlotBackground->SetBrushColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
    }
}

void UInventorySlotWidget::BindToInventoryComponent()
{
    if (!InventoryComponent) return;
    
    // Ensure we're not double-binding
    InventoryComponent->OnInventorySlotChanged.RemoveDynamic(this, &UInventorySlotWidget::OnSlotChanged);
    InventoryComponent->OnInventorySlotChanged.AddDynamic(this, &UInventorySlotWidget::OnSlotChanged);
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("✅ Slot %d bound to InventoryComponent"), SlotIndex);
}

void UInventorySlotWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // Unbind delegate
    if (InventoryComponent)
    {
        InventoryComponent->OnInventorySlotChanged.RemoveDynamic(this, &UInventorySlotWidget::OnSlotChanged);
    }
    
    // Cancel async loading
    if (IconLoadHandle.IsValid())
    {
        IconLoadHandle->CancelHandle();
    }
    // *** NEW: Close any active context menu when slot widget is destroyed ***
    if (APlayerController* PlayerController = GetOwningPlayer())
    {
        if (UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(PlayerController))
        {
            WidgetMgr->CloseActiveContextMenu();
            
            UE_LOG(LogInventoryInteractableSystem, Log, 
                TEXT("Slot %d destroyed - closed active context menu"), SlotIndex);
        }
    }

}

// ============================================================================
// PUBLIC API
// ============================================================================

void UInventorySlotWidget::InitializeSlot(FGameplayTag InInventoryType, int32 InSlotIndex, UInventoryComponent* InInventoryComponent)
{
    InventoryType = InInventoryType;
    SlotIndex = InSlotIndex;
    InventoryComponent = InInventoryComponent;
    
    // Bind delegate (handles both pre and post NativeConstruct cases)
    BindToInventoryComponent();
    
    // Initial refresh
    RefreshSlot();
    
    UE_LOG(LogInventoryInteractableSystem, Verbose, 
        TEXT("SlotWidget initialized: InventoryType=%s, SlotIndex=%d"), 
        *InventoryType.ToString(), SlotIndex);
}

void UInventorySlotWidget::RefreshSlot()
{
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("🔍 RefreshSlot called for Slot %d"), SlotIndex);
    
    if (!InventoryComponent)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("❌ RefreshSlot Slot %d: InventoryComponent is NULL!"), SlotIndex);
        ClearSlot();
        return;
    }
    
    // Get slot data from inventory component
    FInventorySlot SlotData;
    bool bSlotValid = InventoryComponent->GetSlot(InventoryType, SlotIndex, SlotData);
    
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("🔍 Slot %d: GetSlot returned %s, IsEmpty=%s, ItemID=%s"), 
        SlotIndex,
        bSlotValid ? TEXT("VALID") : TEXT("INVALID"),
        SlotData.IsEmpty() ? TEXT("YES") : TEXT("NO"),
        *SlotData.ItemID.ToString());
    
    if (!bSlotValid || SlotData.IsEmpty())
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("⚠️ Slot %d is empty or invalid - clearing"), SlotIndex);
        ClearSlot();
        return;
    }
    
    // Get item data
    FItemData ItemData;
    bool bItemDataValid = InventoryComponent->GetItemData(SlotData.ItemID, ItemData);
    
    if (!bItemDataValid)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("❌ RefreshSlot Slot %d: ItemData not found for ItemID '%s'"), 
            SlotIndex, *SlotData.ItemID.ToString());
        ClearSlot();
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("✅ Slot %d: Updating visuals for %s (Qty=%d)"), 
        SlotIndex, *SlotData.ItemID.ToString(), SlotData.Quantity);
    
    // Update visuals
    UpdateStackCount(SlotData.Quantity, ItemData.MaxStackSize);
    UpdateProgressBar_Durability(SlotData.Durability, ItemData.bHasDurability);
    ApplyRarityColor(&ItemData);
    UpdateStatusIcons(SlotData.bIsFavorite, SlotData.bIsStolen);
    LoadIconAsync(ItemData.IconTexture);

    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("🔍 Slot %d: IconTexture path from ItemData: %s"), 
        SlotIndex, *ItemData.IconTexture.ToString());
    
    // Show all elements
    if (Image_ItemIcon)
    {
        Image_ItemIcon->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("✅ Slot %d: Image_ItemIcon set to Visible"), SlotIndex);
        
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("🔍 Slot %d: Image_ItemIcon visibility: %d, RenderOpacity: %f"), 
            SlotIndex, 
            (int32)Image_ItemIcon->GetVisibility(),
            Image_ItemIcon->GetRenderOpacity());
    }
    else
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ Slot %d: Image_ItemIcon is NULL!"), SlotIndex);
    }
    
    if (Text_StackCount) 
    {
        Text_StackCount->SetVisibility(ESlateVisibility::Visible);
    }
}

void UInventorySlotWidget::ClearSlot()
{
    // Hide all visual elements
    if (Image_ItemIcon) Image_ItemIcon->SetVisibility(ESlateVisibility::Hidden);
    if (Text_StackCount) Text_StackCount->SetVisibility(ESlateVisibility::Hidden);
    if (ProgressBar_Durability) ProgressBar_Durability->SetVisibility(ESlateVisibility::Hidden);
    if (Image_FavoriteIcon) Image_FavoriteIcon->SetVisibility(ESlateVisibility::Hidden);
    if (Image_StolenIcon) Image_StolenIcon->SetVisibility(ESlateVisibility::Hidden);
    
    // Reset slot background to default
    if (Border_SlotBackground)
    {
        Border_SlotBackground->SetBrushColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f)); // Dark gray
    }
}

void UInventorySlotWidget::SetHighlight(bool bHighlighted, FLinearColor HighlightColor)
{
    if (!Border_Highlight) return;
    
    if (bHighlighted)
    {
        Border_Highlight->SetVisibility(ESlateVisibility::HitTestInvisible);
        Border_Highlight->SetBrushColor(HighlightColor);
    }
    else
    {
        Border_Highlight->SetBrushColor(NormalColor);
    }
}

void UInventorySlotWidget::SetHotbarNumber(int32 HotbarNumber)
{
    if (!Text_HotbarNumber)
    {
        Text_HotbarNumber->SetVisibility(ESlateVisibility::Hidden);
        return;
    }
    
    if (HotbarNumber >= 0 && HotbarNumber <= 9)
    {
        // Display 1-10 (user-facing), but internally it's 0-9
        Text_HotbarNumber->SetText(FText::Format(FText::FromString("{0}"), FText::AsNumber(HotbarNumber + 1)));
        Text_HotbarNumber->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        Text_HotbarNumber->SetVisibility(ESlateVisibility::Hidden);
    }
}

bool UInventorySlotWidget::HasItem() const
{
    if (!InventoryComponent) return false;
    
    FInventorySlot SlotData;
    if (InventoryComponent->GetSlot(InventoryType, SlotIndex, SlotData))
    {
        return !SlotData.IsEmpty();
    }
    
    return false;
}

bool UInventorySlotWidget::GetCurrentSlotData(FInventorySlot& OutSlotData) const
{
    if (!InventoryComponent) return false;
    
    return InventoryComponent->GetSlot(InventoryType, SlotIndex, OutSlotData);
}

void UInventorySlotWidget::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;
    
    if (bIsSelected && bEnableClickHighlight)
    {
        // Show selection highlight
        SetHighlight(true, ClickColor);
        
        UE_LOG(LogInventoryInteractableSystem, Verbose, 
            TEXT("Slot %d selected"), SlotIndex);
    }
    else if (!bIsHovered)
    {
        // Only hide if not hovering
        SetHighlight(false);
    }
    else
    {
        // Show hover highlight instead
        SetHighlight(true, HoverColor);
    }
}

// ============================================================================
// MOUSE INTERACTION
// ============================================================================

void UInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    bIsHovered = true;

    // Notify WidgetManager of hover state
    if (UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(GetOwningPlayer()))
    {
        WidgetMgr->SetHoveredSlot(InventoryType, SlotIndex);
    }

    if (bEnableHoverHighlight)
    {
        // PRIORITY: Hover is ALWAYS brightest (overrides everything)
        SetHighlight(true, HoverHighlightColor);

        UE_LOG(LogInventoryInteractableSystem, Verbose,
            TEXT("🔶 Mouse entered Slot %d - Hover highlight"), SlotIndex);
    }
}

void UInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    bIsHovered = false;

    // Notify WidgetManager hover cleared
    if (UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(GetOwningPlayer()))
    {
        WidgetMgr->ClearHoveredSlot();
    }

    // When mouse leaves, restore the appropriate state
    PC = GetOwningPlayer();
    if (!PC)
    {
        SetHighlight(false);
        return;
    }
    
    WidgetManager = UInventoryWidgetManager::Get(PC);
    if (!WidgetManager)
    {
        SetHighlight(false);
        return;
    }
    
    // Check if slot is selected in WidgetManager
    bIsSelected = WidgetManager->IsSlotSelected(InventoryType, SlotIndex);
    
    if (bIsSelected)
    {
        // Restore selection highlight (active or multi)
        if (bIsActiveSelection)
        {
            SetHighlight(true, ActiveSelectionColor); // Medium brightness
            UE_LOG(LogInventoryInteractableSystem, Verbose, 
                TEXT("🟡 Mouse left Slot %d - Active selection highlight"), SlotIndex);
        }
        else
        {
            SetHighlight(true, MultiSelectionColor); // Dim
            UE_LOG(LogInventoryInteractableSystem, Verbose, 
                TEXT("🔵 Mouse left Slot %d - Multi-selection highlight"), SlotIndex);
        }
    }
    else if (bIsBeingDragged)
    {
        // Keep drag highlight
        SetHighlight(true, DragSourceColor);
        UE_LOG(LogInventoryInteractableSystem, Verbose, 
            TEXT("🟨 Mouse left Slot %d - Drag highlight"), SlotIndex);
    }
    else
    {
        // Not selected, not dragging - clear highlight
        SetHighlight(false);
        UE_LOG(LogInventoryInteractableSystem, Verbose, 
            TEXT("Mouse left Slot %d - No highlight"), SlotIndex);
    }
}

FReply UInventorySlotWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // For now, just pass through
    // We might use this later for drag preview updates
    return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UInventorySlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // For now, just pass through
    // Box selection will be handled by InventoryGridWidget
    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}


// ============================================================================
// DRAG AND DROP SYSTEM - MultiSelectionSystem
// ============================================================================

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // ============================================================================
    // RIGHT-CLICK: Context Menu
    // ============================================================================
    UE_LOG(LogTemp, Error, TEXT("🔥 MOUSE BUTTON DOWN - Slot %d"), SlotIndex);
    
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // Close context menu if open
        if (UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(GetOwningPlayer()))
        {
            WidgetMgr->CloseActiveContextMenu();
        }
    }


    if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
    {
        if (HasItem() && ContextMenuClass)
        {
            PC = GetOwningPlayer();
            if (!PC) return FReply::Handled();
            
            WidgetManager = UInventoryWidgetManager::Get(PC);
            if (!WidgetManager) return FReply::Handled();
            
            // Only show context menu in inventory mode
            if (WidgetManager->CanShowContextMenu(InventoryType))
            {
                OpenContextMenu(InMouseEvent);
                return FReply::Handled();
            }
        }
    }
    
    // ============================================================================
    // LEFT-CLICK + SHIFT: Multi-Selection
    // ============================================================================
    
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && InMouseEvent.IsShiftDown())
    {
        PC = GetOwningPlayer();
        WidgetManager = UInventoryWidgetManager::Get(PC);
        
        if (HasItem() && WidgetManager && WidgetManager->CanMultiSelect(InventoryType))
        {
            WidgetManager->ToggleSelection(InventoryType, SlotIndex);
            return FReply::Handled();
        }
        
        // Empty slot with Shift - let grid handle box selection
        return FReply::Unhandled();
    }
    
    // ============================================================================
    // LEFT-CLICK (NO SHIFT): Clear Selection OR Start Drag
    // ============================================================================
    
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        PC = GetOwningPlayer();
        WidgetManager = UInventoryWidgetManager::Get(PC);
        
        // *** NEW: Close any open context menu on left-click ***
        if (WidgetManager)
        {
            WidgetManager->CloseActiveContextMenu();
            WidgetManager->ClearSelection();
        }

        
        // *** KEY FIX: If slot is EMPTY, let the grid handle box selection ***
        if (!HasItem())
        {
            // Don't handle - let it bubble up to grid
            return FReply::Unhandled();
        }
        
        // Has item - start drag if enabled
        if (bEnableDragAndDrop)
        {
            return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
        }
        
        return FReply::Handled();
    }
    
    return FReply::Unhandled();
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("🎯 Drag detected on Slot %d"), SlotIndex);
    
    if (!InventoryComponent)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ Cannot drag - InventoryComponent is NULL"));
        return;
    }
    
    // Get current slot data
    FInventorySlot SlotData;
    if (!GetCurrentSlotData(SlotData) || SlotData.IsEmpty())
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ Cannot drag - slot is empty"));
        return;
    }
    
    // Create drag drop operation
    UInventorySlotDragDropOperation* DragDropOp = NewObject<UInventorySlotDragDropOperation>();
    if (!DragDropOp)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ Failed to create DragDropOperation"));
        return;
    }
    
    // Set operation data
    DragDropOp->SourceInventoryType = InventoryType;
    DragDropOp->SourceSlotIndex = SlotIndex;
    DragDropOp->SourceInventoryComponent = InventoryComponent;
    DragDropOp->DraggedSlotData = SlotData;
    DragDropOp->SourceSlotWidget = this;
    DragDropOp->DragOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
    
    // Create visual for dragging (clone of this widget)
    UInventorySlotWidget* DragVisual = CreateWidget<UInventorySlotWidget>(GetWorld(), GetClass());
    if (DragVisual)
    {
        // Initialize with same data
        DragVisual->InventoryType = InventoryType;
        DragVisual->SlotIndex = SlotIndex;
        DragVisual->InventoryComponent = InventoryComponent;
        DragVisual->RefreshSlot();
        
        // Make it non-interactive and slightly transparent
        DragVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
        if (DragVisual->Border_SlotBackground)
        {
            DragVisual->Border_SlotBackground->SetRenderOpacity(0.7f);
        }
        
        DragDropOp->DefaultDragVisual = DragVisual;
        DragDropOp->Pivot = EDragPivot::MouseDown;
    }
    
    // Update source visuals (make semi-transparent)
    SetDragVisuals(true);
    bIsBeingDragged = true;
    
    OutOperation = DragDropOp;
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("✅ Drag operation created for Slot %d (ItemID: %s)"), 
        SlotIndex, *SlotData.ItemID.ToString());
}

void UInventorySlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
    
    UInventorySlotDragDropOperation* DragOp = Cast<UInventorySlotDragDropOperation>(InOperation);
    if (!DragOp) return;
    
    // Don't highlight the source slot
    if (DragOp->SourceSlotWidget == this) return;
    
    // Check if drop is valid and update highlight
    bool bIsValidDrop = CanAcceptDrop(DragOp);
    UpdateDropHighlight(bIsValidDrop);
    
    UE_LOG(LogInventoryInteractableSystem, Verbose, 
        TEXT("👉 Drag entered Slot %d - Valid: %s"), 
        SlotIndex, bIsValidDrop ? TEXT("YES") : TEXT("NO"));
}

void UInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragLeave(InDragDropEvent, InOperation);
    
    // Remove highlight
    SetHighlight(false, NormalColor);
    
    UE_LOG(LogInventoryInteractableSystem, Verbose, 
        TEXT("👈 Drag left Slot %d"), SlotIndex);
}

bool UInventorySlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
    
    UInventorySlotDragDropOperation* DragOp = Cast<UInventorySlotDragDropOperation>(InOperation);
    if (!DragOp) return false;
    
    // Return true if we can accept the drop
    return CanAcceptDrop(DragOp);
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
    
    UInventorySlotDragDropOperation* DragOp = Cast<UInventorySlotDragDropOperation>(InOperation);
    if (!DragOp)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ Invalid drag operation"));
        return false;
    }
    
    // Remove highlight
    SetHighlight(false, NormalColor);
    
    // Don't drop on self
    if (DragOp->SourceSlotWidget == this)
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("⚠️ Attempted to drop on source slot"));
        return false;
    }
    
    // Validate drop
    if (!CanAcceptDrop(DragOp))
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("⚠️ Cannot accept drop on Slot %d"), SlotIndex);
        return false;
    }
    
    // Execute the drop
    ExecuteDrop(DragOp);
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("✅ Drop completed on Slot %d"), SlotIndex);
    
    return true;
}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
    
    UInventorySlotDragDropOperation* DragOp = Cast<UInventorySlotDragDropOperation>(InOperation);
    if (DragOp && DragOp->SourceSlotWidget)
    {
        // Restore source visuals
        DragOp->SourceSlotWidget->SetDragVisuals(false);
        DragOp->SourceSlotWidget->bIsBeingDragged = false;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("❌ Drag cancelled"));
}

// ============================================================================
// DRAG AND DROP HELPERS
// ============================================================================

bool UInventorySlotWidget::CanAcceptDrop(UInventorySlotDragDropOperation* DragOp) const
{
    if (!DragOp || !DragOp->SourceInventoryComponent || !InventoryComponent)
    {
        return false;
    }
    
    // Get target slot data
    FInventorySlot TargetSlotData;
    bool bTargetHasItem = GetCurrentSlotData(TargetSlotData) && !TargetSlotData.IsEmpty();
    
    // Empty slot - always accept
    if (!bTargetHasItem)
    {
        return true;
    }
    
    // Same item - check if can stack
    if (TargetSlotData.ItemID == DragOp->DraggedSlotData.ItemID)
    {
        // Get item data to check max stack size
        FItemData ItemData;
        if (InventoryComponent->GetItemData(TargetSlotData.ItemID, ItemData))
        {
            // Check if target has space for stacking
            if (TargetSlotData.CanStackWith(DragOp->DraggedSlotData, ItemData.MaxStackSize))
            {
                return true;
            }
        }
    }
    
    // Different items - can swap (always allowed)
    return true;
}

void UInventorySlotWidget::ExecuteDrop(UInventorySlotDragDropOperation* DragOp)
{
    if (!DragOp || !DragOp->SourceInventoryComponent || !InventoryComponent)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ Invalid drop operation data"));
        return;
    }
    
    // Get current target slot data
    FInventorySlot TargetSlotData;
    bool bTargetHasItem = GetCurrentSlotData(TargetSlotData) && !TargetSlotData.IsEmpty();
    
    // Case 1: Empty target slot - simple move
    if (!bTargetHasItem)
    {
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("📦 Moving item from Slot %d to Slot %d"), 
            DragOp->SourceSlotIndex, SlotIndex);
        
        InventoryComponent->SetSlot(InventoryType, SlotIndex, DragOp->DraggedSlotData);

        // Clear source slot
        DragOp->SourceInventoryComponent->ClearSlot(
            DragOp->SourceInventoryType, 
            DragOp->SourceSlotIndex
        );
        
        // Restore source visuals
        if (DragOp->SourceSlotWidget)
        {
            DragOp->SourceSlotWidget->SetDragVisuals(false);
            DragOp->SourceSlotWidget->bIsBeingDragged = false;
            DragOp->SourceSlotWidget->SetHighlight(false, NormalColor);
        }
        
        return;
    }
    
    // Case 2: Same item - try to stack
    if (TargetSlotData.ItemID == DragOp->DraggedSlotData.ItemID)
    {
        FItemData ItemData;
        if (InventoryComponent->GetItemData(TargetSlotData.ItemID, ItemData))
        {
            int32 SpaceAvailable = ItemData.MaxStackSize - TargetSlotData.Quantity;
            int32 AmountToTransfer = FMath::Min(SpaceAvailable, DragOp->DraggedSlotData.Quantity);
            
            if (AmountToTransfer > 0)
            {
                UE_LOG(LogInventoryInteractableSystem, Log, 
                    TEXT("📚 Stacking %d items from Slot %d to Slot %d"), 
                    AmountToTransfer, DragOp->SourceSlotIndex, SlotIndex);
                
                // Transfer quantity
                TargetSlotData.Quantity += AmountToTransfer;
                InventoryComponent->SetSlot(InventoryType, SlotIndex, TargetSlotData);
                
                // Update or clear source
                if (DragOp->DraggedSlotData.Quantity > AmountToTransfer)
                {
                    // Partial transfer - reduce source quantity
                    FInventorySlot UpdatedSourceSlot = DragOp->DraggedSlotData;
                    UpdatedSourceSlot.Quantity -= AmountToTransfer;
                    DragOp->SourceInventoryComponent->SetSlot(
                        DragOp->SourceInventoryType, 
                        DragOp->SourceSlotIndex, 
                        UpdatedSourceSlot
                    );
                }
                else
                {
                    // Full transfer - clear source slot
                    DragOp->SourceInventoryComponent->ClearSlot(
                        DragOp->SourceInventoryType, 
                        DragOp->SourceSlotIndex
                    );
                }
                
                // Restore source visuals
                if (DragOp->SourceSlotWidget)
                {
                    DragOp->SourceSlotWidget->SetDragVisuals(false);
                    DragOp->SourceSlotWidget->bIsBeingDragged = false;
                    DragOp->SourceSlotWidget->SetHighlight(false, NormalColor);
                }
                
                return;
            }
        }
    }
    
    // Case 3: Different items - swap
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("🔄 Swapping items between Slot %d and Slot %d"), 
        DragOp->SourceSlotIndex, SlotIndex);
    
    InventoryComponent->SwapSlots(
        DragOp->SourceInventoryType, 
        DragOp->SourceSlotIndex,
        InventoryType, 
        SlotIndex
    );
    
    // Restore source visuals
    if (DragOp->SourceSlotWidget)
    {
        DragOp->SourceSlotWidget->SetDragVisuals(false);
        DragOp->SourceSlotWidget->bIsBeingDragged = false;
        DragOp->SourceSlotWidget->SetHighlight(false, NormalColor);
    }
}

void UInventorySlotWidget::SetDragVisuals(bool bIsDragging)
{
    if (!Border_SlotBackground) return;
    
    if (bIsDragging)
    {
        // Make semi-transparent and highlight
        Border_SlotBackground->SetRenderOpacity(DragSourceOpacity);
        SetHighlight(true, DragSourceColor);
    }
    else
    {
        // Restore original state
        Border_SlotBackground->SetRenderOpacity(OriginalOpacity);
        SetHighlight(false);
        
        // CRITICAL: Restore background color to correct state
        if (InventoryComponent && HasItem())
        {
            // Has item - restore rarity color
            FInventorySlot SlotData;
            if (GetCurrentSlotData(SlotData) && !SlotData.IsEmpty())
            {
                FItemData ItemData;
                if (InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
                {
                    // Restore the rarity color
                    ApplyRarityColor(&ItemData);
                }
            }
        }
        else
        {
            // Empty slot - restore default gray
            Border_SlotBackground->SetBrushColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
        }
    }
}

void UInventorySlotWidget::UpdateDropHighlight(bool bIsValidDrop)
{
    if (bIsValidDrop)
    {
        SetHighlight(true, ValidDropColor);
    }
    else
    {
        SetHighlight(true, InvalidDropColor);
    }
}

// ============================================================================
// DELEGATE BINDINGS
// ============================================================================

void UInventorySlotWidget::OnSlotChanged(FGameplayTag ChangedInventoryType, int32 ChangedSlotIndex)
{
    // Only refresh if THIS specific slot changed
    if (ChangedInventoryType == InventoryType && ChangedSlotIndex == SlotIndex)
    {
        RefreshSlot();
        
        UE_LOG(LogInventoryInteractableSystem, Verbose, 
            TEXT("SlotWidget refreshed: InventoryType=%s, SlotIndex=%d"), 
            *InventoryType.ToString(), SlotIndex);
    }
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

void UInventorySlotWidget::LoadIconAsync(TSoftObjectPtr<UTexture2D> IconPath)
{
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("🖼️ LoadIconAsync called for Slot %d"), SlotIndex);
    
    if (!Image_ItemIcon)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("❌ Slot %d: Image_ItemIcon is NULL!"), SlotIndex);
        return;
    }
    
    if (IconPath.IsNull())
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("⚠️ Slot %d: IconPath is NULL (not set in DataTable)"), SlotIndex);
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("🖼️ Slot %d: IconPath: %s"), 
        SlotIndex, *IconPath.ToString());
    
    // Cancel previous load if still pending
    if (IconLoadHandle.IsValid() && IconLoadHandle->IsActive())
    {
        IconLoadHandle->CancelHandle();
    }
    
    // Check if already loaded in memory
    if (UTexture2D* LoadedTexture = IconPath.Get())
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("✅ Slot %d: Texture already loaded! Setting immediately."), SlotIndex);
        Image_ItemIcon->SetBrushFromTexture(LoadedTexture);
        return;
    }
    
    // Async load
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("⏳ Slot %d: Starting async load..."), SlotIndex);
    
    IconLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        IconPath.ToSoftObjectPath(),
        [this, IconPath, CapturedSlotIndex = SlotIndex]()
        {
            UE_LOG(LogInventoryInteractableSystem, Warning, 
                TEXT("🔥 Slot %d: Async load callback!"), CapturedSlotIndex);
            
            if (UTexture2D* LoadedIcon = IconPath.Get())
            {
                if (Image_ItemIcon)
                {
                    Image_ItemIcon->SetBrushFromTexture(LoadedIcon);
                    UE_LOG(LogInventoryInteractableSystem, Warning, 
                        TEXT("✅ Slot %d: Icon texture applied!"), CapturedSlotIndex);
                }
                else
                {
                    UE_LOG(LogInventoryInteractableSystem, Error, 
                        TEXT("❌ Slot %d: Image_ItemIcon destroyed!"), CapturedSlotIndex);
                }
            }
            else
            {
                UE_LOG(LogInventoryInteractableSystem, Error, 
                    TEXT("❌ Slot %d: Failed to load texture from path: %s"), 
                    CapturedSlotIndex, *IconPath.ToString());
            }
        }
    );
}

void UInventorySlotWidget::ApplyRarityColor(const FItemData* ItemData)
{
    if (!Border_SlotBackground || !ItemData) return;
    
    FLinearColor RarityColor = ItemData->GetRarityColor();
    Border_SlotBackground->SetBrushColor(RarityColor);
}

void UInventorySlotWidget::UpdateDurabilityBar(float DurabilityPercent, bool bHasDurability)
{
}

void UInventorySlotWidget::UpdateProgressBar_Durability(float DurabilityPercent, bool bHasDurability)
{
    if (!ProgressBar_Durability) return;
    
    if (bHasDurability)
    {
        ProgressBar_Durability->SetVisibility(ESlateVisibility::HitTestInvisible);
        ProgressBar_Durability->SetPercent(DurabilityPercent);
        
        // Color code: Green > Yellow > Red
        FLinearColor BarColor;
        if (DurabilityPercent > 0.6f)
        {
            BarColor = FLinearColor::Green;
        }
        else if (DurabilityPercent > 0.3f)
        {
            BarColor = FLinearColor::Yellow;
        }
        else
        {
            BarColor = FLinearColor::Red;
        }
        
        ProgressBar_Durability->SetFillColorAndOpacity(BarColor);
    }
    else
    {
        ProgressBar_Durability->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UInventorySlotWidget::UpdateStackCount(int32 Quantity, int32 MaxStackSize)
{
    if (!Text_StackCount) return;
    
    if (Quantity > 1)
    {
        Text_StackCount->SetText(FText::Format(FText::FromString("x{0}"), FText::AsNumber(Quantity)));
        Text_StackCount->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        Text_StackCount->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UInventorySlotWidget::UpdateStatusIcons(bool bIsFavorite, bool bIsStolen)
{
    if (Image_FavoriteIcon)
    {
        Image_FavoriteIcon->SetVisibility(bIsFavorite ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
    }
    
    if (Image_StolenIcon)
    {
        Image_StolenIcon->SetVisibility(bIsStolen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
    }
}

void UInventorySlotWidget::OnSelectionChanged()
{
    if (!InventoryComponent)
    {
        return;
    }
    
    PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }
    
    WidgetManager = UInventoryWidgetManager::Get(PC);
    if (!WidgetManager)
    {
        return;
    }
    
    // Check if this slot is selected
    bIsSelected = WidgetManager->IsSlotSelected(InventoryType, SlotIndex);
    
    if (bIsSelected)
    {
        // Check if this is the LAST selected slot (active selection)
        TArray<FInventorySlotReference> SelectedSlots = WidgetManager->GetSelectedSlots();
        if (SelectedSlots.Num() > 0)
        {
            FInventorySlotReference LastSelected = SelectedSlots.Last();
            bIsActiveSelection = (LastSelected.InventoryType == InventoryType && 
                                  LastSelected.SlotIndex == SlotIndex);
        }
        
        // Update highlight based on whether it's active or just selected
        // BUT: Don't override hover highlight
        if (bIsHovered)
        {
            // Keep hover highlight (brightest)
            SetHighlight(true, HoverHighlightColor);
        }
        else if (bIsActiveSelection)
        {
            SetHighlight(true, ActiveSelectionColor); // Brighter
        }
        else
        {
            SetHighlight(true, MultiSelectionColor); // Dimmer
        }
        
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("✅ Slot %d selected (Active: %s)"), SlotIndex, bIsActiveSelection ? TEXT("YES") : TEXT("NO"));
    }
    else
    {
        bIsActiveSelection = false;
        
        // Only clear highlight if not hovering
        if (!bIsHovered)
        {
            SetHighlight(false);
            UE_LOG(LogInventoryInteractableSystem, Log, 
                TEXT("❌ Slot %d deselected"), SlotIndex);
        }
        else
        {
            // Keep hover highlight
            SetHighlight(true, HoverHighlightColor);
        }
    }
}

void UInventorySlotWidget::OpenContextMenu(const FPointerEvent& MouseEvent)
{
    if (!ContextMenuClass)
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("Cannot open context menu - ContextMenuClass is NULL"));
        return;
    }
    
    if (!InventoryComponent)
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("Cannot open context menu - InventoryComponent is NULL"));
        return;
    }
    
    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("Cannot open context menu - No PlayerController"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("Cannot open context menu - No World"));
        return;
    }
    
    // Get WidgetManager to manage single menu instance
    UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(PlayerController);
    if (!WidgetMgr)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("Cannot open context menu - No WidgetManager"));
        return;
    }
    
    // Close any existing context menu first (only one menu at a time)
    WidgetMgr->CloseActiveContextMenu();
    
    // Create context menu widget
    UMIIS_RightClickContextMenu* ContextMenu = CreateWidget<UMIIS_RightClickContextMenu>(
        PlayerController, 
        ContextMenuClass
    );
    
    if (!ContextMenu)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("Failed to create context menu widget"));
        return;
    }
    
    // Initialize with slot data
    ContextMenu->InitializeMenu(InventoryType, SlotIndex, InventoryComponent);
    
    // Register with WidgetManager (so it can close it later)
    WidgetMgr->SetActiveContextMenu(ContextMenu);
    
    // Add to viewport with high z-order
    ContextMenu->AddToViewport(999);
    
    // *** CORRECT WAY: Use GetMousePositionOnViewport ***
    FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(World);
    ContextMenu->SetPositionInViewport(MousePosition, false);
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("✅ Context menu opened for slot %d at viewport position %s"), 
        SlotIndex, *MousePosition.ToString());
}


