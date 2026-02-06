// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InventoryGridWidget.h"
#include "Components/UniformGridSlot.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Windwalker_Productions_SharedDefaults/Public/WW_TagLibrary.h"
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
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("✅ InventoryGridWidget constructed"));
    
}

void UInventoryGridWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
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
        if (FMath::Abs(CurrentAvailableWidth - CachedAvailableWidth) > 10.0f) // 10 pixel threshold
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
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ InitializeGrid: InventoryComponent is NULL!"));
        return;
    }
    
    if (!ValidateConfiguration())
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ InitializeGrid: Configuration validation failed!"));
        return;
    }
    
    InventoryComponent = InInventoryComponent;
    
    // Set default inventory type if not set
    if (!InventoryType.IsValid())
    {
        InventoryType = FWWTagLibrary::Inventory_Type_PlayerInventory();
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("⚠️ InventoryType not set, defaulting to PlayerInventory"));
    }
    
    // Clear existing slots
    ClearGrid();
    
    // Create new slots
    CreateSlotWidgets();
    
    // Update scrolling settings after creation
    UpdateScrollBoxSettings();
    
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("✅✅✅ InventoryGridWidget initialized: %d slots in %d columns ✅✅✅"), 
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
    // Calculate total width needed for all columns
    // Width = (SlotSize * Columns) + (SlotPadding * (Columns - 1))
    float TotalWidth = (SlotSize * Columns) + (SlotPadding * (Columns - 1));
    
    // Add some margin for safety (20 pixels)
    TotalWidth += 20.0f;
    
    return TotalWidth;
}

float UInventoryGridWidget::GetAvailableWidth() const
{
    if (!ScrollBox)
    {
        return 0.0f;
    }
    
    // Get the cached geometry of the ScrollBox
    FGeometry ScrollBoxGeometry = ScrollBox->GetCachedGeometry();
    FVector2D LocalSize = ScrollBoxGeometry.GetLocalSize();
    
    // If size is invalid, try to get from canvas slot
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
// INTERNAL HELPERS
// ============================================================================

bool UInventoryGridWidget::ValidateConfiguration() const
{
    bool bIsValid = true;
    
    if (!GridPanel)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("❌ GridPanel is NULL! Add a UniformGridPanel to your Blueprint Designer with variable name 'GridPanel'"));
        bIsValid = false;
    }
    
    if (!ScrollBox)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("❌ ScrollBox is NULL! Add a ScrollBox to your Blueprint Designer with variable name 'ScrollBox'"));
        bIsValid = false;
    }
    
    if (!SlotWidgetClass)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("❌ SlotWidgetClass is not set! Set it to WBP_InventorySlot in Blueprint Class Defaults"));
        bIsValid = false;
    }
    
    if (Columns <= 0 || Columns > 20)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("❌ Invalid Columns value: %d (must be 1-20)"), Columns);
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
    
    // Determine if horizontal scrolling is needed
    bool bNeedsHorizontalScroll = GridWidth > AvailableWidth && AvailableWidth >= MinWidthForHorizontalScroll;
    
    if (bNeedsHorizontalScroll)
    {
        // Enable horizontal scrolling
        ScrollBox->SetOrientation(EOrientation::Orient_Horizontal);
        ScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
        
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("🔄 Horizontal scrolling ENABLED - Grid: %.0fpx, Available: %.0fpx"), 
            GridWidth, AvailableWidth);
    }
    else
    {
        // Use vertical scrolling only
        ScrollBox->SetOrientation(EOrientation::Orient_Vertical);
        ScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
        
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("⬆️ Vertical scrolling mode - Grid: %.0fpx, Available: %.0fpx"), 
            GridWidth, AvailableWidth);
    }
    
    // Always allow mouse wheel scrolling
    ScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
    ScrollBox->SetAnimateWheelScrolling(true);
    ScrollBox->SetScrollWhenFocusChanges(EScrollWhenFocusChanges::NoScroll);
}

void UInventoryGridWidget::CreateSlotWidgets()
{
    if (!GridPanel || !InventoryComponent || !SlotWidgetClass)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ Cannot create slots - missing required references"));
        return;
    }
    
    // Get inventory size
    int32 InventorySize = InventoryComponent->MaxInventorySlots;
    
    if (InventorySize <= 0 || InventorySize > 500)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, 
            TEXT("❌ Invalid inventory size: %d (must be 1-500)"), InventorySize);
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT(">>> Creating %d slot widgets with class: %s"), 
        InventorySize, *SlotWidgetClass->GetName());
    
    // Reserve array capacity for performance
    SlotWidgets.Reserve(InventorySize);
    
    // Create slot widgets
    for (int32 i = 0; i < InventorySize; ++i)
    {
        // Create slot widget
        UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(GetWorld(), SlotWidgetClass);
        
        if (!SlotWidget)
        {
            UE_LOG(LogInventoryInteractableSystem, Error, 
                TEXT("❌ Failed to create slot widget at index %d"), i);
            continue;
        }
        
        // Initialize the slot (CRITICAL - this makes it work!)
        SlotWidget->InitializeSlot(InventoryType, i, InventoryComponent);
        
        // Calculate grid position
        int32 Row = i / Columns;
        int32 Column = i % Columns;
        
        // Add to grid panel
        UUniformGridSlot* GridSlot = GridPanel->AddChildToUniformGrid(SlotWidget, Row, Column);
        
        if (GridSlot)
        {
            // Set slot size and padding
            GridSlot->SetHorizontalAlignment(HAlign_Fill);
            GridSlot->SetVerticalAlignment(VAlign_Fill);
        }
        else
        {
            UE_LOG(LogInventoryInteractableSystem, Warning, 
                TEXT("⚠️ Failed to get GridSlot for widget %d"), i);
        }
        
        // Add to tracked array
        SlotWidgets.Add(SlotWidget);
        
        // Log progress every 50 slots
        if ((i + 1) % 50 == 0)
        {
            UE_LOG(LogInventoryInteractableSystem, Log, 
                TEXT(">>> Created %d/%d slots..."), i + 1, InventorySize);
        }
    }
    
    UE_LOG(LogInventoryInteractableSystem, Warning, 
        TEXT("✅ Slot creation complete: %d widgets created"), SlotWidgets.Num());
    
    // Force refresh all slots immediately after creation
    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (SlotWidgets[i])
        {
            SlotWidgets[i]->RefreshSlot();
        }
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("✅ All slots force-refreshed"));
}


FReply UInventoryGridWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // Close context menu
        if (UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(GetOwningPlayer()))
        {
            WidgetMgr->CloseActiveContextMenu();
        }
    }
    

    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        // Check if clicking on empty space (not on a slot)
        FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();
        StartBoxSelection(MousePos);
        
        return FReply::Handled().CaptureMouse(TakeWidget());
    }
    
    return FReply::Unhandled();
}

FReply UInventoryGridWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsBoxSelecting)
    {
        FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();
        UpdateBoxSelection(MousePos);
        return FReply::Handled();
    }
    
    return FReply::Unhandled();
}

FReply UInventoryGridWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsBoxSelecting)
    {
        EndBoxSelection();
        return FReply::Handled().ReleaseMouseCapture();
    }
    
    return FReply::Unhandled();
}


void UInventoryGridWidget::StartBoxSelection(const FVector2D& ScreenPosition)
{
    bIsBoxSelecting = true;
    BoxSelectionStart = ScreenPosition;
    BoxSelectionEnd = ScreenPosition;
    
    // Create selection box widget
    if (BoxSelectionWidgetClass && !BoxSelectionWidget)
    {
        BoxSelectionWidget = CreateWidget<UUserWidget>(GetWorld(), BoxSelectionWidgetClass);
        if (BoxSelectionWidget)
        {
            // Walk up widget tree to find a canvas panel OUTSIDE the scroll box
            UWidget* CurrentWidget = this;
            UCanvasPanel* RootCanvas = nullptr;
            
            while (CurrentWidget)
            {
                // Skip the scroll box
                if (Cast<UScrollBox>(CurrentWidget))
                {
                    UE_LOG(LogTemp, Log, TEXT("Found ScrollBox, continuing to parent..."));
                    CurrentWidget = CurrentWidget->GetParent();
                    continue;
                }
                
                // Found a canvas panel outside scroll box
                if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(CurrentWidget))
                {
                    RootCanvas = Canvas;
                    UE_LOG(LogTemp, Warning, TEXT("✅ Found Canvas Panel: %s"), *Canvas->GetName());
                    break;
                }
                
                CurrentWidget = CurrentWidget->GetParent();
            }
            
            if (RootCanvas)
            {
                // Add to canvas panel with high z-order
                UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(BoxSelectionWidget);
                if (CanvasSlot)
                {
                    CanvasSlot->SetZOrder(9999);
                    CanvasSlot->SetAnchors(FAnchors(0, 0, 0, 0));
                    
                    // CONVERT INITIAL POSITION TO LOCAL SPACE
                    FGeometry CanvasGeometry = RootCanvas->GetCachedGeometry();
                    FVector2D LocalStart = CanvasGeometry.AbsoluteToLocal(ScreenPosition);
                    
                    CanvasSlot->SetPosition(LocalStart);
                    CanvasSlot->SetSize(FVector2D(1, 1)); // Tiny initial size
                    
                    UE_LOG(LogTemp, Warning, TEXT("✅ Box added to Canvas at Local: %s"), *LocalStart.ToString());
                }
            }
            else
            {
                // Fallback to viewport (should work if canvas not found)
                BoxSelectionWidget->AddToViewport(99999);
                BoxSelectionWidget->SetPositionInViewport(ScreenPosition, false);
                BoxSelectionWidget->SetDesiredSizeInViewport(FVector2D(1, 1));
                UE_LOG(LogTemp, Warning, TEXT("⚠️ No canvas found, added to viewport"));
            }
            
            BoxSelectionWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Started box selection at: %s"), *ScreenPosition.ToString());
}


void UInventoryGridWidget::UpdateBoxSelection(const FVector2D& ScreenPosition)
{
    if (!bIsBoxSelecting) return;
    
    BoxSelectionEnd = ScreenPosition;
    
    if (BoxSelectionWidget)
    {
        FVector2D TopLeft(
            FMath::Min(BoxSelectionStart.X, BoxSelectionEnd.X),
            FMath::Min(BoxSelectionStart.Y, BoxSelectionEnd.Y)
        );
        
        FVector2D Size(
            FMath::Abs(BoxSelectionEnd.X - BoxSelectionStart.X),
            FMath::Abs(BoxSelectionEnd.Y - BoxSelectionStart.Y)
        );
        
        // Check if it's in a canvas panel slot
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(BoxSelectionWidget->Slot))
        {
            // CRITICAL: Convert screen space to local space
            UCanvasPanel* ParentCanvas = Cast<UCanvasPanel>(BoxSelectionWidget->GetParent());
            if (ParentCanvas)
            {
                FGeometry CanvasGeometry = ParentCanvas->GetCachedGeometry();
                
                // Convert screen position to local position
                FVector2D LocalTopLeft = CanvasGeometry.AbsoluteToLocal(TopLeft);
                
                // Update canvas slot position (now in local space)
                CanvasSlot->SetPosition(LocalTopLeft);
                CanvasSlot->SetSize(Size);
                
                UE_LOG(LogTemp, Log, TEXT("Canvas Box - Screen: %s, Local: %s, Size: %s"), 
                    *TopLeft.ToString(), *LocalTopLeft.ToString(), *Size.ToString());
            }
        }
        else
        {
            // Fallback to viewport positioning (already in screen space)
            BoxSelectionWidget->SetPositionInViewport(TopLeft, false);
            BoxSelectionWidget->SetDesiredSizeInViewport(Size);
            
            UE_LOG(LogTemp, Log, TEXT("Viewport Box - TopLeft: %s, Size: %s"), 
                *TopLeft.ToString(), *Size.ToString());
        }
        
        BoxSelectionWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}



void UInventoryGridWidget::EndBoxSelection()
{
    if (!bIsBoxSelecting) return;
    
    // Get rectangle bounds
    FVector2D TopLeft(
        FMath::Min(BoxSelectionStart.X, BoxSelectionEnd.X),
        FMath::Min(BoxSelectionStart.Y, BoxSelectionEnd.Y)
    );
    
    FVector2D BottomRight(
        FMath::Max(BoxSelectionStart.X, BoxSelectionEnd.X),
        FMath::Max(BoxSelectionStart.Y, BoxSelectionEnd.Y)
    );
    
    // Get all slots in rectangle
    TArray<UInventorySlotWidget*> SlotsInBox = GetSlotsInRectangle(TopLeft, BottomRight);
    
    // Select all slots
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UInventoryWidgetManager* WidgetMgr = UInventoryWidgetManager::Get(PC))
        {
            // Clear previous selection
            WidgetMgr->ClearSelection();
            
            // Add all slots in box
            for (UInventorySlotWidget* SlotWidget : SlotsInBox)  // ← Renamed from Slot to SlotWidget
            {
                if (SlotWidget && SlotWidget->HasItem())
                {
                    WidgetMgr->AddToSelection(SlotWidget->InventoryType, SlotWidget->SlotIndex);
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Box selection: Selected %d slots"), SlotsInBox.Num());
        }
    }
    
    // Clean up
    bIsBoxSelecting = false;
    if (BoxSelectionWidget)
    {
        BoxSelectionWidget->RemoveFromParent();
        BoxSelectionWidget = nullptr;
    }
}


// FIXED GetSlotsInRectangle - Replace your version with this

TArray<UInventorySlotWidget*> UInventoryGridWidget::GetSlotsInRectangle(const FVector2D& TopLeft, const FVector2D& BottomRight)
{
    TArray<UInventorySlotWidget*> SlotsInBox;
    
    if (!GridPanel) return SlotsInBox;  // ← Changed from SlotContainer to GridPanel
    
    // Iterate all slot widgets
    for (int32 i = 0; i < GridPanel->GetChildrenCount(); ++i)  // ← Changed to GridPanel
    {
        UInventorySlotWidget* SlotWidget = Cast<UInventorySlotWidget>(GridPanel->GetChildAt(i));  // ← Changed to GridPanel
        if (!SlotWidget) continue;
        
        // Get slot widget's screen position
        FGeometry SlotGeometry = SlotWidget->GetCachedGeometry();
        FVector2D SlotScreenPos = SlotGeometry.GetAbsolutePosition();
        FVector2D SlotWidgetSize = SlotGeometry.GetAbsoluteSize();  // ← Renamed from SlotSize
        
        // Check if slot overlaps with selection rectangle
        bool bOverlapsX = (SlotScreenPos.X + SlotWidgetSize.X > TopLeft.X) && (SlotScreenPos.X < BottomRight.X);
        bool bOverlapsY = (SlotScreenPos.Y + SlotWidgetSize.Y > TopLeft.Y) && (SlotScreenPos.Y < BottomRight.Y);
        
        if (bOverlapsX && bOverlapsY)
        {
            SlotsInBox.Add(SlotWidget);
        }
    }
    
    return SlotsInBox;
}