// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "UI/HelperUI/ContextMenu/MIIS_RightClickContextMenu.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/ModularInventorySystem/InvnetoryData.h"
#include "Actors/Interactables/Item/Data/ItemData.h"
#include "Components/InventoryComponent.h"
#include "InventorySlotWidget.generated.h"

// Forward declarations
class UInventorySlotDragDropOperation;

/**
 * Single Inventory Slot Widget
 * 
 * Displays one inventory slot with:
 * - Item icon (async loaded)
 * - Stack count text
 * - Durability bar
 * - Rarity border
 * - Hotbar number (for equipment slots)
 * - Favorite/stolen indicators
 * - Full drag-and-drop support
 * 
 * Drag and Drop Features:
 * - Drag items between slots
 * - Visual drag preview
 * - Drop validation (can items stack, swap, etc.)
 * - Highlight on hover during drag
 * - Support for split/merge operations
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ============================================================================
    // WIDGET COMPONENTS (Bind in UMG Designer)
    // ============================================================================
    
    /** Background container (for sizing and click detection) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UBorder* Border_SlotBackground;
    
    /** Item icon image */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* Image_ItemIcon;
    
    /** Stack count text (e.g., "x47") */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* Text_StackCount;
    
    /** Durability progress bar (bottom of slot) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UProgressBar* ProgressBar_Durability;
    
    /** Rarity border (colored outline) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UOverlay* Overlay_Rarity;
    
    /** Hotbar number (for equipment slots, e.g., "1") */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* Text_HotbarNumber;
    
    /** Favorite star icon */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* Image_FavoriteIcon;
    
    /** Stolen indicator icon */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* Image_StolenIcon;
    
    /** Overlay for hover/selection highlight */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UBorder* Border_Highlight;
    
    // ============================================================================
    // SLOT DATA (Cached for performance)
    // ============================================================================
    
    /** Which inventory does this slot belong to? (Inventory.Type.PlayerInventory, etc.) */
    UPROPERTY(BlueprintReadOnly, Category = "Slot")
    FGameplayTag InventoryType;
    
    /** Slot index in that inventory */
    UPROPERTY(BlueprintReadOnly, Category = "Slot")
    int32 SlotIndex = -1;
    
    /** Cached inventory component reference */
    UPROPERTY(BlueprintReadOnly, Category = "Slot")
    UInventoryComponent* InventoryComponent = nullptr;
    
    // ============================================================================
    // DRAG AND DROP CONFIGURATION
    // ============================================================================
    
    /** Color for valid drop target highlight */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Drag and Drop")
    FLinearColor ValidDropColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f); // Green
    
    /** Color for invalid drop target highlight */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Drag and Drop")
    FLinearColor InvalidDropColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f); // Red
    
    /** Color for drag source highlight */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Drag and Drop")
    FLinearColor DragSourceColor = FLinearColor(1.0f, 1.0f, 0.0f, 0.3f); // Yellow
    
    /** Opacity of the slot during drag */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Drag and Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DragSourceOpacity = 0.5f;
    
    /** Enable drag and drop on this slot */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Drag and Drop")
    bool bEnableDragAndDrop = true;
    
    // ============================================================================
    // HOVER AND CLICK CONFIGURATION
    // ============================================================================
    
    /** Enable hover highlighting */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hover and Click")
    bool bEnableHoverHighlight = true;
    
    /** Color for hover highlight */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hover and Click")
    FLinearColor HoverColor = FLinearColor(1.0f, 0.516f, 1.0f, 0.3f); // White semi-transparent
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default State")
    FLinearColor NormalColor = FLinearColor(0.703f, 0.703f, 0.703f, 1.0f); // White semi-transparent
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default State")
    FLinearColor Slot_BackgroundColor = FLinearColor(0.0f, 0.062f, 0.056f, 1.0f); // White semi-transparent
    
    /** Enable click/selection highlighting */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hover and Click")
    bool bEnableClickHighlight = true;
    
    /** Color for click/selection highlight */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hover and Click")
    FLinearColor ClickColor = FLinearColor(0.0f, 0.5f, 1.0f, 0.6f); // Blue
    
    /** Duration of highlight fade animations (seconds) - RESERVED FOR FUTURE USE */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hover and Click", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HighlightFadeDuration = 0.15f;

    // ============================================================================
    // HIGHLIGHT COLORS (Priority: Hover > Active Selection > Multi-Selection)
    // ============================================================================
    
    /** Color for mouse hover (brightest - orange) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Highlight Colors")
    FLinearColor HoverHighlightColor = FLinearColor(1.0f, 0.7f, 0.0f, 0.9f); // Bright Orange
    
    /** Color for last/active selected slot (medium brightness - yellow) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Highlight Colors")
    FLinearColor ActiveSelectionColor = FLinearColor(1.0f, 0.7f, 0.0f, 0.7f); // Yellow
    
    /** Color for multi-selected slots (dimmest - cyan) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Highlight Colors")
    FLinearColor MultiSelectionColor = FLinearColor(1.0f, 0.7f, 0.0f, 0.6f); // Dim Cyan

    
    // ============================================================================
    // CONTEXT MENU
    // ============================================================================
    
    /** Right-click context menu widget class */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Context Menu")
    TSubclassOf<UMIIS_RightClickContextMenu> ContextMenuClass;
    
    // ============================================================================
    // PUBLIC API
    // ============================================================================
    
    /**
     * Initialize this slot widget (called once from pool)
     * @param InInventoryType - Which inventory (Inventory.Type.PlayerInventory, etc.)
     * @param InSlotIndex - Index in that inventory
     * @param InInventoryComponent - Reference to owner's inventory component
     */
    UFUNCTION(BlueprintCallable, Category = "Slot")
    void InitializeSlot(FGameplayTag InInventoryType, int32 InSlotIndex, UInventoryComponent* InInventoryComponent);
    
    /**
     * Refresh slot display (called when item changes)
     * CRITICAL: Only updates visuals, doesn't iterate or search
     */
    UFUNCTION(BlueprintCallable, Category = "Slot")
    void RefreshSlot();
    
    /**
     * Clear slot display (for empty slots)
     */
    UFUNCTION(BlueprintCallable, Category = "Slot")
    void ClearSlot();
    
    /**
     * Set highlight state (hover, drag-over, selected)
     * @param bHighlighted - Show highlight?
     * @param HighlightColor - Color of highlight
     */
    UFUNCTION(BlueprintCallable, Category = "Slot")
    void SetHighlight(bool bHighlighted, FLinearColor HighlightColor = FLinearColor(1.0f, 0.516f, 1.0f, 0.3f));
    
    /**
     * Set hotbar number (for equipment slots only)
     * @param HotbarNumber - Number to display (1-10, or -1 to hide)
     */
    UFUNCTION(BlueprintCallable, Category = "Slot")
    void SetHotbarNumber(int32 HotbarNumber);

    UFUNCTION(BlueprintCallable, Category = "Slot")
    void UpdateProgressBar_Durability(float DurabilityPercent, bool bHasDurability);
    
    /**
     * Check if this slot currently has an item
     */
    UFUNCTION(BlueprintPure, Category = "Slot")
    bool HasItem() const;
    
    /**
     * Get the current slot data
     */
    UFUNCTION(BlueprintPure, Category = "Slot")
    bool GetCurrentSlotData(FInventorySlot& OutSlotData) const;
    
    /**
     * Set slot as selected/clicked
     * @param bSelected - Is this slot selected?
     */
    UFUNCTION(BlueprintCallable, Category = "Slot")
    void SetSelected(bool bSelected);
    
    /**
     * Check if slot is currently selected
     */
    UFUNCTION(BlueprintPure, Category = "Slot")
    bool IsSelected() const { return bIsSelected; }

protected:

    /**
     * Called when selection changes in WidgetManager
     * Updates this slot's highlight based on selection state
     */
    UFUNCTION()
    void OnSelectionChanged();
    
    /** Track if this is the last/active selected slot */
    bool bIsActiveSelection = false;

    // ============================================================================
    // CONTEXT MENU
    // ============================================================================
    
    /**
     * Opens the right-click context menu at the mouse position
     * @param MouseEvent - The mouse event containing cursor position
     */
    void OpenContextMenu(const FPointerEvent& MouseEvent);
    
    // ============================================================================
    // WIDGET LIFECYCLE
    // ============================================================================
    
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    void InitializeVisuals();
    void BindToInventoryComponent();

    // ============================================================================
    // MOUSE INPUT
    // ============================================================================
    
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    

    // ============================================================================
    // MOUSE INTERACTION OVERRIDES
    // ============================================================================
    
    /**
     * Called when mouse enters the widget
     */
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    /**
     * Called when mouse leaves the widget
     */
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

    // ============================================================================
    // DRAG AND DROP OVERRIDES
    // ============================================================================
    
   
    /**
     * Called when drag is detected - create drag operation
     */
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    
    /**
     * Called when dragged item enters this widget
     */
    virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    
    /**
     * Called when dragged item leaves this widget
     */
    virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    
    /**
     * Called continuously while hovering during drag
     */
    virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    
    /**
     * Called when item is dropped on this widget
     */
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    
    /**
     * Called when drag is cancelled (ESC key or invalid drop)
     */
    virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    // ============================================================================
    // DELEGATE BINDINGS
    // ============================================================================
    
    /** Called when this specific slot changes (from InventoryComponent) */
    UFUNCTION()
    void OnSlotChanged(FGameplayTag ChangedInventoryType, int32 ChangedSlotIndex);

    // ============================================================================
    // DRAG AND DROP HELPERS
    // ============================================================================
    
    /**
     * Check if we can drop the dragged item on this slot
     * @return True if drop is valid
     */
    bool CanAcceptDrop(UInventorySlotDragDropOperation* DragOp) const;
    
    /**
     * Execute the drop operation (move/swap/stack)
     */
    void ExecuteDrop(UInventorySlotDragDropOperation* DragOp);
    
    /**
     * Create visual feedback for drag state
     */
    void SetDragVisuals(bool bIsDragging);
    
    /**
     * Update highlight based on drop validity
     */
    void UpdateDropHighlight(bool bIsValidDrop);
       
    // ============================================================================
    // INTERNAL HELPERS
    // ============================================================================
    
    /** Load icon asynchronously */
    void LoadIconAsync(TSoftObjectPtr<UTexture2D> IconPath);
    
    /** Apply rarity border color */
    void ApplyRarityColor(const FItemData* ItemData);
    
    /** Update durability bar */
    void UpdateDurabilityBar(float DurabilityPercent, bool bHasDurability);
    
    /** Update stack count text */
    void UpdateStackCount(int32 Quantity, int32 MaxStackSize);
    
    /** Update favorite/stolen icons */
    void UpdateStatusIcons(bool bIsFavorite, bool bIsStolen);
    
    /** Async icon loading handle */
    TSharedPtr<FStreamableHandle> IconLoadHandle;
    
    /** Store original opacity for restoring after drag */
    float OriginalOpacity = 1.0f;
    
    /** Track if currently being dragged */
    bool bIsBeingDragged = false;
    
    /** Track if mouse is hovering over this slot */
    bool bIsHovered = false;
    
    /** Track if this slot is selected/clicked */
    bool bIsSelected = false;

    UPROPERTY(Transient)
    UWidgetManager* WidgetManager;
    UPROPERTY(Transient)
    APlayerController* PC;
    
};

/**
 * Drag Drop Operation for Inventory Slots
 * 
 * Contains all necessary data for drag-and-drop operations:
 * - Source slot information
 * - Item data being dragged
 * - Visual widget for drag preview
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UInventorySlotDragDropOperation : public UDragDropOperation
{
    GENERATED_BODY()

public:
    /** Source inventory type */
    UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
    FGameplayTag SourceInventoryType;
    
    /** Source slot index */
    UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
    int32 SourceSlotIndex = -1;
    
    /** Reference to source inventory component */
    UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
    UInventoryComponent* SourceInventoryComponent = nullptr;
    
    /** Cached slot data being dragged */
    UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
    FInventorySlot DraggedSlotData;
    
    /** Reference to the source slot widget */
    UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
    UInventorySlotWidget* SourceSlotWidget = nullptr;
    
    /** Offset from widget origin where drag started */
    UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
    FVector2D DragOffset;
};