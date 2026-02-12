// Copyright Windwalker Productions. All Rights Reserved.

#include "UI/InventoryGridWidget.h"
#include "UI/BoxSelectionWidget.h"
#include "Components/UniformGridSlot.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Subsystems/InventoryWidgetManager.h"

// ============================================================================
// WIDGET LIFECYCLE
// ============================================================================
UInventoryGridWidget::UInventoryGridWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WidgetCategoryTag = FWWTagLibrary::UI_Widget_Category_Window();
    bAutoRegister = true;
}

void UInventoryGridWidget::NativeConstruct()
{
    Super::NativeConstruct();
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("InventoryGridWidget constructed"));
}

void UInventoryGridWidget::NativeDestruct()
{
    Super::NativeDestruct();

    // Clean up marquee widget
    if (ActiveBoxSelection)
    {
        ActiveBoxSelection->DeactivateMarquee();
        ActiveBoxSelection->RemoveFromParent();
        ActiveBoxSelection = nullptr;
    }

    // Clean up all widgets
    ClearGrid();

    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("InventoryGridWidget destructed"));
}

void UInventoryGridWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bEnableHorizontalScrolling || !ScrollBox)
    {
        return;
    }

    // Periodically check if available width has changed (window resize)
    WidthCheckTimer += InDeltaTime;
    if (WidthCheckTimer >= WidthCheckInterval)
    {
        WidthCheckTimer = 0.0f;

        float CurrentAvailableWidth = GetAvailableWidth();
        if (FMath::Abs(CurrentAvailableWidth - CachedAvailableWidth) > 10.0f)
        {
            CachedAvailableWidth = CurrentAvailableWidth;
            UpdateScrollBoxSettings();
        }
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UInventoryGridWidget::InitializeGrid(UInventoryComponent* InInventoryComponent)
{
    UE_LOG(LogInventoryInteractableSystem, Warning, TEXT(">>> InitializeGrid called"));

    if (!InInventoryComponent)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("InitializeGrid: InventoryComponent is NULL!"));
        return;
    }

    if (!ValidateConfiguration())
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("InitializeGrid: Configuration validation failed!"));
        return;
    }

    InventoryComponent = InInventoryComponent;

    // Set default inventory type if not set
    if (!InventoryType.IsValid())
    {
        InventoryType = FWWTagLibrary::Inventory_Type_PlayerInventory();
        UE_LOG(LogInventoryInteractableSystem, Warning,
            TEXT("InventoryType not set, defaulting to PlayerInventory"));
    }

    // Clear existing slots
    ClearGrid();

    // Create new slots
    CreateSlotWidgets();

    // Update scrolling settings after creation
    UpdateScrollBoxSettings();

    UE_LOG(LogInventoryInteractableSystem, Warning,
        TEXT("InventoryGridWidget initialized: %d slots in %d columns"),
        SlotWidgets.Num(), Columns);
}

void UInventoryGridWidget::RefreshAllSlots()
{
    int32 RefreshCount = 0;

    for (UInventorySlotWidget* SlotWidget : SlotWidgets)
    {
        if (SlotWidget)
        {
            SlotWidget->RefreshSlot();
            RefreshCount++;
        }
    }

    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("Refreshed %d slots"), RefreshCount);
}

void UInventoryGridWidget::ClearGrid()
{
    if (GridPanel)
    {
        GridPanel->ClearChildren();
    }

    SlotWidgets.Empty();

    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("Grid cleared"));
}

void UInventoryGridWidget::UpdateScrollBoxSettings()
{
    if (!ScrollBox || !bEnableHorizontalScrolling)
    {
        return;
    }

    float GridWidth = CalculateGridWidth();
    float AvailableWidth = GetAvailableWidth();

    ConfigureScrollBox(GridWidth, AvailableWidth);
}

float UInventoryGridWidget::CalculateGridWidth() const
{
    float TotalWidth = (SlotSize * Columns) + (SlotPadding * (Columns - 1));
    TotalWidth += 20.0f;
    return TotalWidth;
}

float UInventoryGridWidget::GetAvailableWidth() const
{
    if (!ScrollBox)
    {
        return 0.0f;
    }

    FGeometry ScrollBoxGeometry = ScrollBox->GetCachedGeometry();
    FVector2D LocalSize = ScrollBoxGeometry.GetLocalSize();

    if (LocalSize.X <= 0.0f)
    {
        if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ScrollBox))
        {
            LocalSize = CanvasSlot->GetSize();
        }
    }

    return LocalSize.X;
}

// ============================================================================
// MOUSE INPUT (forwards to marquee widget)
// ============================================================================

FReply UInventoryGridWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // Close context menu on left click
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(GetOwningPlayer()))
        {
            WidgetMgr->CloseActiveContextMenu();
        }
    }

    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        // Find or cache root canvas
        UCanvasPanel* RootCanvas = FindRootCanvas();
        if (!RootCanvas)
        {
            return FReply::Unhandled();
        }

        // Create marquee widget if needed
        if (!ActiveBoxSelection && BoxSelectionWidgetClass)
        {
            ActiveBoxSelection = CreateWidget<UBoxSelectionWidget>(GetWorld(), BoxSelectionWidgetClass);
            if (ActiveBoxSelection)
            {
                ActiveBoxSelection->SetOwningGrid(this);

                UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(ActiveBoxSelection);
                if (CanvasSlot)
                {
                    CanvasSlot->SetZOrder(9999);
                    CanvasSlot->SetAnchors(FAnchors(0, 0, 0, 0));
                    CanvasSlot->SetPosition(FVector2D::ZeroVector);
                    CanvasSlot->SetSize(FVector2D(1, 1));
                }

                ActiveBoxSelection->ActivateMarquee(RootCanvas);
            }
        }

        if (ActiveBoxSelection && ActiveBoxSelection->IsMarqueeActive())
        {
            // Convert screen position to local canvas space
            FGeometry CanvasGeometry = RootCanvas->GetCachedGeometry();
            FVector2D LocalPos = CanvasGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

            if (ActiveBoxSelection->HandleMouseDown(LocalPos, InMouseEvent))
            {
                return FReply::Handled().CaptureMouse(TakeWidget());
            }
        }
    }

    return FReply::Unhandled();
}

FReply UInventoryGridWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (ActiveBoxSelection && ActiveBoxSelection->IsMarqueeActive())
    {
        UCanvasPanel* RootCanvas = FindRootCanvas();
        if (RootCanvas)
        {
            FGeometry CanvasGeometry = RootCanvas->GetCachedGeometry();
            FVector2D LocalPos = CanvasGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

            if (ActiveBoxSelection->HandleMouseMove(LocalPos, InMouseEvent))
            {
                return FReply::Handled();
            }
        }
    }

    return FReply::Unhandled();
}

FReply UInventoryGridWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && ActiveBoxSelection)
    {
        UCanvasPanel* RootCanvas = FindRootCanvas();
        if (RootCanvas)
        {
            FGeometry CanvasGeometry = RootCanvas->GetCachedGeometry();
            FVector2D LocalPos = CanvasGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

            if (ActiveBoxSelection->HandleMouseUp(LocalPos, InMouseEvent))
            {
                // Clean up the widget after selection completes
                ActiveBoxSelection->DeactivateMarquee();
                ActiveBoxSelection->RemoveFromParent();
                ActiveBoxSelection = nullptr;

                return FReply::Handled().ReleaseMouseCapture();
            }
        }
    }

    return FReply::Unhandled();
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

bool UInventoryGridWidget::ValidateConfiguration() const
{
    bool bIsValid = true;

    if (!GridPanel)
    {
        UE_LOG(LogInventoryInteractableSystem, Error,
            TEXT("GridPanel is NULL! Add a UniformGridPanel to your Blueprint Designer with variable name 'GridPanel'"));
        bIsValid = false;
    }

    if (!ScrollBox)
    {
        UE_LOG(LogInventoryInteractableSystem, Error,
            TEXT("ScrollBox is NULL! Add a ScrollBox to your Blueprint Designer with variable name 'ScrollBox'"));
        bIsValid = false;
    }

    if (!SlotWidgetClass)
    {
        UE_LOG(LogInventoryInteractableSystem, Error,
            TEXT("SlotWidgetClass is not set! Set it to WBP_InventorySlot in Blueprint Class Defaults"));
        bIsValid = false;
    }

    if (Columns <= 0 || Columns > 20)
    {
        UE_LOG(LogInventoryInteractableSystem, Error,
            TEXT("Invalid Columns value: %d (must be 1-20)"), Columns);
        bIsValid = false;
    }

    return bIsValid;
}

void UInventoryGridWidget::ConfigureScrollBox(float GridWidth, float AvailableWidth)
{
    if (!ScrollBox)
    {
        return;
    }

    bool bNeedsHorizontalScroll = GridWidth > AvailableWidth && AvailableWidth >= MinWidthForHorizontalScroll;

    if (bNeedsHorizontalScroll)
    {
        ScrollBox->SetOrientation(EOrientation::Orient_Horizontal);
        ScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);

        UE_LOG(LogInventoryInteractableSystem, Log,
            TEXT("Horizontal scrolling ENABLED - Grid: %.0fpx, Available: %.0fpx"),
            GridWidth, AvailableWidth);
    }
    else
    {
        ScrollBox->SetOrientation(EOrientation::Orient_Vertical);
        ScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);

        UE_LOG(LogInventoryInteractableSystem, Log,
            TEXT("Vertical scrolling mode - Grid: %.0fpx, Available: %.0fpx"),
            GridWidth, AvailableWidth);
    }

    ScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
    ScrollBox->SetAnimateWheelScrolling(true);
    ScrollBox->SetScrollWhenFocusChanges(EScrollWhenFocusChanges::NoScroll);
}

void UInventoryGridWidget::CreateSlotWidgets()
{
    if (!GridPanel || !InventoryComponent || !SlotWidgetClass)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("Cannot create slots - missing required references"));
        return;
    }

    int32 InventorySize = InventoryComponent->MaxInventorySlots;

    if (InventorySize <= 0 || InventorySize > 500)
    {
        UE_LOG(LogInventoryInteractableSystem, Error,
            TEXT("Invalid inventory size: %d (must be 1-500)"), InventorySize);
        return;
    }

    UE_LOG(LogInventoryInteractableSystem, Warning,
        TEXT(">>> Creating %d slot widgets with class: %s"),
        InventorySize, *SlotWidgetClass->GetName());

    SlotWidgets.Reserve(InventorySize);

    for (int32 i = 0; i < InventorySize; ++i)
    {
        UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(GetWorld(), SlotWidgetClass);

        if (!SlotWidget)
        {
            UE_LOG(LogInventoryInteractableSystem, Error,
                TEXT("Failed to create slot widget at index %d"), i);
            continue;
        }

        SlotWidget->InitializeSlot(InventoryType, i, InventoryComponent);

        int32 Row = i / Columns;
        int32 Column = i % Columns;

        UUniformGridSlot* GridSlot = GridPanel->AddChildToUniformGrid(SlotWidget, Row, Column);

        if (GridSlot)
        {
            GridSlot->SetHorizontalAlignment(HAlign_Fill);
            GridSlot->SetVerticalAlignment(VAlign_Fill);
        }

        SlotWidgets.Add(SlotWidget);
    }

    UE_LOG(LogInventoryInteractableSystem, Warning,
        TEXT("Slot creation complete: %d widgets created"), SlotWidgets.Num());

    // Force refresh all slots immediately after creation
    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (SlotWidgets[i])
        {
            SlotWidgets[i]->RefreshSlot();
        }
    }

    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("All slots force-refreshed"));
}

UCanvasPanel* UInventoryGridWidget::FindRootCanvas()
{
    if (CachedRootCanvas.IsValid())
    {
        return CachedRootCanvas.Get();
    }

    // Walk up widget tree to find a canvas panel OUTSIDE the scroll box
    UWidget* CurrentWidget = this;
    bool bPassedScrollBox = false;

    while (CurrentWidget)
    {
        if (Cast<UScrollBox>(CurrentWidget))
        {
            bPassedScrollBox = true;
            CurrentWidget = CurrentWidget->GetParent();
            continue;
        }

        if (bPassedScrollBox || !Cast<UScrollBox>(this))
        {
            if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(CurrentWidget))
            {
                CachedRootCanvas = Canvas;
                return Canvas;
            }
        }

        CurrentWidget = CurrentWidget->GetParent();
    }

    return nullptr;
}
