// MIIS_RightClickContextMenu.h
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/ModularInventorySystem/InvnetoryData.h"
#include "MIIS_RightClickContextMenu.generated.h"

// Forward declarations
class UButton;
class UVerticalBox;
class UInventoryComponent;
class UWidgetManager;

/**
 * Right-Click Context Menu for Inventory Slots
 * Provides options like Use, Drop, Combine, Attach, etc.
 * 
 * Button visibility is controlled by InventoryComponent query helpers
 * to maintain separation of concerns (AAA professional standard).
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UMIIS_RightClickContextMenu : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UMIIS_RightClickContextMenu(const FObjectInitializer& ObjectInitializer);
    
    // ============================================================================
    // WIDGET BINDINGS
    // ============================================================================
    
    /** Container for menu buttons */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UVerticalBox> VBox_MenuItems;
    
    /** Use/Equip button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Use;
    
    /** Unequip button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Unequip;
    
    /** Drop button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Drop;
    
    /** Split stack button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Split;
    
    /** Examine button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Examine;
    
    /** Favorite toggle button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Favorite;
    
    /** Combine similar items button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Combine;
    
    /** Combine with quest item button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_CombineWith;
    
    /** Attach to weapon button (replaces CombineWith for attachments) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_AttachTo;
    
    // ============================================================================
    // INITIALIZATION
    // ============================================================================
    
    /**
     * Initialize the context menu for a specific inventory slot
     * @param InInventoryType - The inventory type tag
     * @param InSlotIndex - The slot index
     * @param InInventoryComponent - The inventory component to operate on
     */
    UFUNCTION(BlueprintCallable, Category = "Context Menu")
    void InitializeMenu(FGameplayTag InInventoryType, int32 InSlotIndex, UInventoryComponent* InInventoryComponent);
    
    /** Clear all menu items */
    UFUNCTION(BlueprintCallable, Category = "Context Menu")
    void ClearMenuItems();
    
    /** Close this menu */
    UFUNCTION(BlueprintCallable, Category = "Context Menu")
    void CloseMenu();
    
protected:
    // ============================================================================
    // NATIVE WIDGET OVERRIDES
    // ============================================================================
    
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    // ============================================================================
    // BUTTON CALLBACKS
    // ============================================================================
    
    UFUNCTION()
    void OnUseClicked();
    
    UFUNCTION()
    void OnUnequipClicked();
    
    UFUNCTION()
    void OnDropClicked();
    
    UFUNCTION()
    void OnSplitClicked();
    
    UFUNCTION()
    void OnExamineClicked();
    
    UFUNCTION()
    void OnFavoriteClicked();
    
    /** Combine all similar items (same ID, same rarity, not favorited) */
    UFUNCTION()
    void OnCombineClicked();
    
    /** Enter quest combine mode - select another item to combine with */
    UFUNCTION()
    void OnCombineWithClicked();
    
    /** Enter attachment mode - select a weapon to attach to */
    UFUNCTION()
    void OnAttachToClicked();
    
    // ============================================================================
    // CACHED DATA
    // ============================================================================
    
    /** Target inventory type */
    UPROPERTY()
    FGameplayTag TargetInventoryType;
    
    /** Target slot index */
    UPROPERTY()
    int32 TargetSlotIndex = -1;
    
    /** Reference to inventory component */
    UPROPERTY()
    TObjectPtr<UInventoryComponent> TargetInventoryComp;
    
    /** Widget category tag for WidgetManager */
    UPROPERTY()
    FGameplayTag WidgetCategoryTag;
    
    /** Auto-register with WidgetManager */
    UPROPERTY()
    bool bAutoRegister = true;
};