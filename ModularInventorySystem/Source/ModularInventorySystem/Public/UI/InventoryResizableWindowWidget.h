// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "HelperUI/SearchSortWidget_Master.h"
#include "UI/InventoryGridWidget.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "InventoryResizableWindowWidget.generated.h"

// Forward declare
class UInventoryComponent;

/**
 * Resizable Inventory Window
 * 
 * Features:
 * - Draggable title bar using DragDropOperation pattern
 * - Resizable via Canvas Panel Slot
 * - Min/Max size constraints
 * - Close button
 * - Contains the inventory grid
 * 
 * IMPORTANT: Widget must be in Canvas Panel for dragging to work!
 * Remove SizeBox - use Canvas Panel Slot for sizing instead
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UInventoryResizableWindowWidget : public UManagedWidget_Master
{
    GENERATED_BODY()

public:
    // ============================================================================
    // WIDGET COMPONENTS (BindWidget)
    // ============================================================================
    UInventoryResizableWindowWidget(const FObjectInitializer& ObjectInitializer);

    /** Main window border */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UBorder* Border_Window;
    
    /** Title bar (for dragging) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UBorder* Border_TitleBar;
    
    /** Title text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* Text_Title;
    
    /** Close button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* Button_Close;
    
    /** Inventory grid widget */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UInventoryGridWidget* InventoryGrid;

    /*SearchSortWidget (Optional)*/
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory | SearchSort")
    TSubclassOf<USearchSortWidget_Master> SearchSortWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<USearchSortWidget_Master> SearchSortWidget;
    
    // ============================================================================
    // CONFIGURATION
    // ============================================================================
    
    /** Minimum window size */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Window")
    FVector2D MinSize = FVector2D(400, 300);
    
    /** Maximum window size */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Window")
    FVector2D MaxSize = FVector2D(1200, 900);
    
    /** Default window size */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Window")
    FVector2D DefaultSize = FVector2D(800, 600);
    
    /** Window title text */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Window")
    FText WindowTitle = FText::FromString("INVENTORY");
    
    // ============================================================================
    // PUBLIC API
    // ============================================================================
    
    /**
     * Initialize the window
     * Call this from parent or InventoryComponent
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Window")
    void InitializeWindow(UInventoryComponent* InInventoryComponent);
    
    /**
     * Toggle window visibility
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Window")
    void ToggleWindow();
    
    /**
     * Show window
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Window")
    void ShowWindow();
    
    /**
     * Hide window
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Window")
    void HideWindow();
    
    /**
     * Set window size via Canvas Panel Slot
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Window")
    void SetWindowSize(FVector2D NewSize);

protected:
    // ============================================================================
    // WIDGET LIFECYCLE & DRAG AND DROP SYSTEM
    // ============================================================================
    
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    // Drag and Drop Override Functions
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    void NativeOnDragCancelled(
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation);
    //virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    
    // ============================================================================
    // BUTTON CALLBACKS
    // ============================================================================
    
    UFUNCTION()
    void OnCloseButtonClicked();

private:
    // ============================================================================
    // DRAGGING STATE
    // ============================================================================
    
    /** Drag offset from window position (stored during drag detect) */
    FVector2D DragOffset;
    
    /** Current window size */
    FVector2D CurrentSize;
    
    /** Reference to inventory component */
    UPROPERTY()
    UInventoryComponent* InventoryComponent = nullptr;
};