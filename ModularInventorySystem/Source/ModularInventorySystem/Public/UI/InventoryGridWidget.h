// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/ScrollBox.h"
#include "GameplayTagContainer.h"
#include "UI/InventorySlotWidget.h"
#include "Components/CanvasPanel.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "Components/InventoryComponent.h"
#include "Subsystems/InventoryWidgetManager.h"

#include "InventoryGridWidget.generated.h"

// Forward declarations
class UBoxSelectionWidget;

/**
 * Inventory Grid Widget (C++ Manager)
 *
 * Industry-standard grid manager that creates and manages all inventory slot widgets.
 *
 * Features:
 * - Programmatic widget creation (no manual setup)
 * - Automatic initialization of all slots
 * - Performance-optimized (C++ iteration, widget pooling)
 * - Designer-friendly (Blueprint inheritance for visuals)
 * - Automatic horizontal scrolling when grid width exceeds available space
 * - Marquee box selection via UMarqueeSelectionWidget_Base (V2.13.6)
 *
 * Usage in Blueprint:
 * 1. Create Blueprint inheriting from this class (e.g., WBP_InventoryGrid)
 * 2. In Designer, add ScrollBox and UniformGridPanel (will auto-bind via BindWidget)
 * 3. In parent widget's Event Construct, call InitializeGrid(InventoryComponent)
 *
 * Scrolling Behavior:
 * - Vertical scrolling is always enabled
 * - Horizontal scrolling auto-enables when grid width exceeds ScrollBox width
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UInventoryGridWidget : public UManagedWidget_Master
{
    GENERATED_BODY()

public:
    // ============================================================================
    // WIDGET COMPONENTS (BindWidget - Must exist in Blueprint Designer)
    // ============================================================================

    UInventoryGridWidget(const FObjectInitializer& ObjectInitializer);

    /** Scroll box containing the grid (REQUIRED - Add in Blueprint Designer) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UScrollBox* ScrollBox;

    /** Grid panel holding all slot widgets (REQUIRED - Add in Blueprint Designer) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UUniformGridPanel* GridPanel;

    // ============================================================================
    // CONFIGURATION (Set in Blueprint Class Defaults)
    // ============================================================================

    /** Slot widget class to spawn (Set to WBP_InventorySlot in Blueprint) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory Grid")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    /** Number of columns in the grid */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory Grid", meta = (ClampMin = "1", ClampMax = "20"))
    int32 Columns = 10;

    /** Which inventory to display (Inventory.Type.PlayerInventory) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory Grid")
    FGameplayTag InventoryType;

    /** Size of each slot widget (square) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory Grid", meta = (ClampMin = "32", ClampMax = "128"))
    float SlotSize = 64.0f;

    /** Padding between slots */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory Grid", meta = (ClampMin = "0", ClampMax = "20"))
    float SlotPadding = 2.0f;

    /** Enable automatic horizontal scrolling when grid width exceeds available space */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory Grid|Scrolling")
    bool bEnableHorizontalScrolling = true;

    /** Minimum width threshold before enabling horizontal scrolling (pixels) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory Grid|Scrolling", meta = (ClampMin = "100", ClampMax = "2000"))
    float MinWidthForHorizontalScroll = 400.0f;

    // ============================================================================
    // BOX SELECTION CONFIGURATION
    // ============================================================================

    /** Class for box selection widget (must be UBoxSelectionWidget or subclass) */
    UPROPERTY(EditDefaultsOnly, Category = "Selection")
    TSubclassOf<UBoxSelectionWidget> BoxSelectionWidgetClass;

    // ============================================================================
    // PUBLIC API
    // ============================================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void InitializeGrid(UInventoryComponent* InInventoryComponent);

    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void RefreshAllSlots();

    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void ClearGrid();

    UFUNCTION(BlueprintPure, Category = "Inventory Grid")
    int32 GetSlotCount() const { return SlotWidgets.Num(); }

    /**
     * Update scroll box settings based on current grid dimensions
     * Called automatically after grid creation, but can be called manually if window resizes
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
    void UpdateScrollBoxSettings();

    /**
     * Calculate the total width required for the grid
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Grid")
    float CalculateGridWidth() const;

    /**
     * Get the current available width of the ScrollBox
     */
    UFUNCTION(BlueprintPure, Category = "Inventory Grid")
    float GetAvailableWidth() const;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ============================================================================
    // MOUSE INPUT (forwards to marquee widget)
    // ============================================================================

    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
    UPROPERTY()
    UInventoryComponent* InventoryComponent = nullptr;

    UPROPERTY()
    TArray<UInventorySlotWidget*> SlotWidgets;

    /** Active marquee selection widget instance */
    UPROPERTY()
    TObjectPtr<UBoxSelectionWidget> ActiveBoxSelection;

    /** Cached canvas panel for marquee overlay */
    TWeakObjectPtr<UCanvasPanel> CachedRootCanvas;

    /** Cached available width for comparison */
    float CachedAvailableWidth = 0.0f;

    /** Timer for checking width changes */
    float WidthCheckTimer = 0.0f;

    /** How often to check for width changes (seconds) */
    float WidthCheckInterval = 0.5f;

    void CreateSlotWidgets();
    bool ValidateConfiguration() const;
    void ConfigureScrollBox(float GridWidth, float AvailableWidth);

    /** Find the root canvas panel above the scroll box for marquee overlay */
    UCanvasPanel* FindRootCanvas();
};
