// InventoryWidgetManager.h
// Inventory-specific widget manager extending UWidgetManagerBase

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/AdvancedWidgetFramework/WidgetManagerBase.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/ModularInventorySystem/InvnetoryData.h"
#include "InventoryWidgetManager.generated.h"

// Forward declarations
class UUserWidget;
class UInventoryComponent;
class UMIIS_RightClickContextMenu;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectionModeChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectionChanged);

/**
 * Inventory Widget Manager Subsystem
 * Extends UWidgetManagerBase with inventory-specific functionality:
 * - Slot selection modes (quest combine, attachment, compare)
 * - Hover tracking for inventory slots
 * - Context menu management
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UInventoryWidgetManager : public UWidgetManagerBase
{
    GENERATED_BODY()

public:
    // ============================================================================
    // LIFECYCLE
    // ============================================================================

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Get InventoryWidgetManager from PlayerController */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager", meta = (DefaultToSelf = "PlayerController"))
    static UInventoryWidgetManager* Get(APlayerController* PlayerController);

    // ============================================================================
    // WIDGET MANAGEMENT (extends UWidgetManagerBase)
    // ============================================================================
    // Inherited: ShowWidget(), HideWidget(), HideAllWidgets(), IsWidgetShown(), GetActiveWidget()

    /**
     * Show a widget at the current mouse cursor position
     * @param WidgetClass - The widget class to create and show
     * @param Offset - Optional offset from cursor position (default: zero)
     * @return The created widget, or nullptr if failed
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager")
    UUserWidget* ShowWidgetAtCursor(TSubclassOf<UUserWidget> WidgetClass, FVector2D Offset = FVector2D::ZeroVector);

    /** Close active context menu */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager")
    void CloseActiveContextMenu();

    // ============================================================================
    // SLOT SELECTION
    // ============================================================================

    /** Check if a slot is selected */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Selection")
    bool IsSlotSelected(FGameplayTag InventoryType, int32 SlotIndex) const;

    /** Select a slot */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Selection")
    void SelectSlot(FGameplayTag InventoryType, int32 SlotIndex);

    /** Deselect a slot */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Selection")
    void DeselectSlot(FGameplayTag InventoryType, int32 SlotIndex);

    /** Toggle slot selection */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Selection")
    void ToggleSlotSelection(FGameplayTag InventoryType, int32 SlotIndex);

    /** Clear all selections */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Selection")
    void ClearSelection();

    /** Get selected slot count */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Selection")
    int32 GetSelectedSlotCount() const;

    /** Get all selected slots */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Selection")
    TArray<FInventorySlotReference> GetSelectedSlots() const;

    // ============================================================================
    // HELPER WIDGETS
    // ============================================================================

    /** Show split quantity widget */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Inventory")
    void ShowSplitQuantityWidget(FGameplayTag InventoryType, int32 SlotIndex);

    /** Show examine widget */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Inventory")
    void ShowExamineWidget(FGameplayTag InventoryType, int32 SlotIndex);

    // ============================================================================
    // QUEST COMBINE MODE
    // ============================================================================

    /**
     * Enter quest combine mode - allows user to select another item to combine with
     * @param InventoryType - The inventory containing the source item
     * @param SourceSlotIndex - The slot initiating the quest combine
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Inventory")
    void EnterQuestCombineMode(FGameplayTag InventoryType, int32 SourceSlotIndex);

    /**
     * Exit quest combine mode
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Inventory")
    void ExitQuestCombineMode();

    /**
     * Check if currently in quest combine mode
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Inventory")
    bool IsInQuestCombineMode() const { return bIsInQuestCombineMode; }

    /**
     * Get the source slot for quest combine
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Inventory")
    void GetQuestCombineSource(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const;

    // ============================================================================
    // ATTACHMENT MODE
    // ============================================================================

    /**
     * Enter attachment mode - dims incompatible slots for attachment
     * @param InventoryType - The inventory containing the attachment
     * @param AttachmentSlotIndex - The slot containing the attachment
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Inventory")
    void EnterAttachmentMode(FGameplayTag InventoryType, int32 AttachmentSlotIndex);

    /**
     * Exit attachment mode
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Inventory")
    void ExitAttachmentMode();

    /**
     * Check if currently in attachment mode
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Inventory")
    bool IsInAttachmentMode() const { return bIsInAttachmentMode; }

    /**
     * Get the source slot for attachment
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Inventory")
    void GetAttachmentSource(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const;

    /**
     * Check if a slot can accept the current attachment
     * Delegates to InventoryComponent->CanAttachToSlot()
     * @param InventoryType - The inventory to check
     * @param SlotIndex - The slot to check
     * @return True if this slot can accept the attachment
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Inventory")
    bool CanSlotAcceptAttachment(FGameplayTag InventoryType, int32 SlotIndex) const;

    // ============================================================================
    // COMPARE MODE
    // ============================================================================

    /**
     * Enter compare mode - compare items against a source item
     * @param InventoryType - The inventory containing the source item
     * @param SlotIndex - The slot of the item to compare against
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Inventory")
    void EnterCompareMode(FGameplayTag InventoryType, int32 SlotIndex);

    /**
     * Exit compare mode
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Widget Manager|Inventory")
    void ExitCompareMode();

    /**
     * Check if currently in compare mode
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Inventory")
    bool IsInCompareMode() const { return bIsInCompareMode; }

    /**
     * Get the source slot for comparison
     * @param OutInventoryType - The inventory type of the compare source
     * @param OutSlotIndex - The slot index of the compare source
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Inventory")
    void GetCompareSource(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const;

    // ============================================================================
    // HOVER TRACKING
    // ============================================================================

    /**
     * Get the currently hovered inventory slot
     * @param OutInventoryType - The inventory type of the hovered slot
     * @param OutSlotIndex - The slot index of the hovered slot
     * @return True if a slot is currently hovered
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Widget Manager|Hover")
    bool GetHoveredInventorySlot(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const;

    /**
     * Set the currently hovered slot (called by InventorySlotWidget)
     * @param InventoryType - The inventory type being hovered
     * @param SlotIndex - The slot index being hovered
     */
    void SetHoveredSlot(FGameplayTag InventoryType, int32 SlotIndex);

    /**
     * Clear the hovered slot (called by InventorySlotWidget on mouse leave)
     */
    void ClearHoveredSlot();

    // ============================================================================
    // DELEGATES
    // ============================================================================

    /** Broadcast when selection mode changes (combine/attachment mode entered/exited) */
    UPROPERTY(BlueprintAssignable, Category = "Inventory Widget Manager|Events")
    FOnSelectionModeChanged OnSelectionModeChanged;

    /** Broadcast when slot selection changes */
    UPROPERTY(BlueprintAssignable, Category = "Inventory Widget Manager|Events")
    FOnSelectionChanged OnSelectionChanged;

protected:
    // ============================================================================
    // HELPERS
    // ============================================================================
    // Inherited: GetOwningPlayer()

    /**
     * Helper function to get player inventory component
     * @return Pointer to player's inventory component, or nullptr if not found
     */
    UInventoryComponent* GetPlayerInventoryComponent() const;

private:
    // ============================================================================
    // STATE
    // ============================================================================

    /** Currently selected slots */
    UPROPERTY()
    TArray<FInventorySlotReference> SelectedSlots;

    /** Active context menu */
    UPROPERTY()
    TObjectPtr<UMIIS_RightClickContextMenu> ActiveContextMenu;

    /** Are we in quest combine mode? */
    bool bIsInQuestCombineMode = false;

    /** Source inventory for quest combine */
    FGameplayTag QuestCombineSourceInventory;

    /** Source slot for quest combine */
    int32 QuestCombineSourceSlot = -1;

    /** Are we in attachment mode? */
    bool bIsInAttachmentMode = false;

    /** Source inventory for attachment */
    FGameplayTag AttachmentSourceInventory;

    /** Source slot for attachment */
    int32 AttachmentSourceSlot = -1;

    // ----------------------------------------------------------------------------
    // COMPARE MODE STATE
    // ----------------------------------------------------------------------------

    /** Are we in compare mode? */
    bool bIsInCompareMode = false;

    /** Source inventory for compare */
    FGameplayTag CompareSourceInventory;

    /** Source slot for compare */
    int32 CompareSourceSlot = INDEX_NONE;

    // ----------------------------------------------------------------------------
    // HOVER TRACKING STATE
    // ----------------------------------------------------------------------------

    /** Currently hovered slot */
    FInventorySlotReference HoveredSlot;

    /** Is there a currently hovered slot? */
    bool bHasHoveredSlot = false;
};
