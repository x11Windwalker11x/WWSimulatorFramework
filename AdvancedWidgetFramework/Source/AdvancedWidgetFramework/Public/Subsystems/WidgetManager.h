// WidgetManager.h
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/ModularInventorySystem/InvnetoryData.h"
#include "WidgetManager.generated.h"

// Forward declarations
class UUserWidget;
class UInventoryComponent;
class UMIIS_RightClickContextMenu;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectionModeChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectionChanged);

/**
 * Widget Manager Subsystem
 * Handles widget lifecycle, selection modes, and inventory UI interactions
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UWidgetManager : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    // ============================================================================
    // LIFECYCLE
    // ============================================================================
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    /** Get WidgetManager from PlayerController */
    UFUNCTION(BlueprintPure, Category = "Widget Manager", meta = (DefaultToSelf = "PlayerController"))
    static UWidgetManager* Get(APlayerController* PlayerController);
    
    // ============================================================================
    // WIDGET MANAGEMENT
    // ============================================================================
    
    /** Show a widget by class */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    UUserWidget* ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 0);
    
    /** Hide a widget */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    void HideWidget(UUserWidget* Widget);
    
    /** Close active context menu */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    void CloseActiveContextMenu();
    
    // ============================================================================
    // SLOT SELECTION
    // ============================================================================
    
    /** Check if a slot is selected */
    UFUNCTION(BlueprintPure, Category = "Widget Manager|Selection")
    bool IsSlotSelected(FGameplayTag InventoryType, int32 SlotIndex) const;
    
    /** Select a slot */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Selection")
    void SelectSlot(FGameplayTag InventoryType, int32 SlotIndex);
    
    /** Deselect a slot */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Selection")
    void DeselectSlot(FGameplayTag InventoryType, int32 SlotIndex);
    
    /** Toggle slot selection */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Selection")
    void ToggleSlotSelection(FGameplayTag InventoryType, int32 SlotIndex);
    
    /** Clear all selections */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Selection")
    void ClearSelection();
    
    /** Get selected slot count */
    UFUNCTION(BlueprintPure, Category = "Widget Manager|Selection")
    int32 GetSelectedSlotCount() const;
    
    /** Get all selected slots */
    UFUNCTION(BlueprintPure, Category = "Widget Manager|Selection")
    TArray<FInventorySlotReference> GetSelectedSlots() const;
    
    // ============================================================================
    // HELPER WIDGETS
    // ============================================================================
    
    /** Show split quantity widget */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Inventory")
    void ShowSplitQuantityWidget(FGameplayTag InventoryType, int32 SlotIndex);
    
    /** Show examine widget */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Inventory")
    void ShowExamineWidget(FGameplayTag InventoryType, int32 SlotIndex);
    
    // ============================================================================
    // QUEST COMBINE MODE
    // ============================================================================
    
    /**
     * Enter quest combine mode - allows user to select another item to combine with
     * @param InventoryType - The inventory containing the source item
     * @param SourceSlotIndex - The slot initiating the quest combine
     */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Inventory")
    void EnterQuestCombineMode(FGameplayTag InventoryType, int32 SourceSlotIndex);
    
    /**
     * Exit quest combine mode
     */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Inventory")
    void ExitQuestCombineMode();
    
    /**
     * Check if currently in quest combine mode
     */
    UFUNCTION(BlueprintPure, Category = "Widget Manager|Inventory")
    bool IsInQuestCombineMode() const { return bIsInQuestCombineMode; }
    
    /**
     * Get the source slot for quest combine
     */
    UFUNCTION(BlueprintPure, Category = "Widget Manager|Inventory")
    void GetQuestCombineSource(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const;
    
    // ============================================================================
    // ATTACHMENT MODE
    // ============================================================================
    
    /**
     * Enter attachment mode - dims incompatible slots for attachment
     * @param InventoryType - The inventory containing the attachment
     * @param AttachmentSlotIndex - The slot containing the attachment
     */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Inventory")
    void EnterAttachmentMode(FGameplayTag InventoryType, int32 AttachmentSlotIndex);
    
    /**
     * Exit attachment mode
     */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager|Inventory")
    void ExitAttachmentMode();
    
    /**
     * Check if currently in attachment mode
     */
    UFUNCTION(BlueprintPure, Category = "Widget Manager|Inventory")
    bool IsInAttachmentMode() const { return bIsInAttachmentMode; }
    
    /**
     * Get the source slot for attachment
     */
    UFUNCTION(BlueprintPure, Category = "Widget Manager|Inventory")
    void GetAttachmentSource(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const;
    
    /**
     * Check if a slot can accept the current attachment
     * Delegates to InventoryComponent->CanAttachToSlot()
     * @param InventoryType - The inventory to check
     * @param SlotIndex - The slot to check
     * @return True if this slot can accept the attachment
     */
    UFUNCTION(BlueprintPure, Category = "Widget Manager|Inventory")
    bool CanSlotAcceptAttachment(FGameplayTag InventoryType, int32 SlotIndex) const;
    
    // ============================================================================
    // DELEGATES
    // ============================================================================
    
    /** Broadcast when selection mode changes (combine/attachment mode entered/exited) */
    UPROPERTY(BlueprintAssignable, Category = "Widget Manager|Events")
    FOnSelectionModeChanged OnSelectionModeChanged;
    
    /** Broadcast when slot selection changes */
    UPROPERTY(BlueprintAssignable, Category = "Widget Manager|Events")
    FOnSelectionChanged OnSelectionChanged;

protected:
    // ============================================================================
    // HELPERS
    // ============================================================================
    
    /**
     * Helper function to get player inventory component
     * @return Pointer to player's inventory component, or nullptr if not found
     */
    UInventoryComponent* GetPlayerInventoryComponent() const;
    
    /** Get owning player controller */
    APlayerController* GetOwningPlayer() const;

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
};