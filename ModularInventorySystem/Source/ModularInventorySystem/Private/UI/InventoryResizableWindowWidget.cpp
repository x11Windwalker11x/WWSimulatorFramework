// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InventoryResizableWindowWidget.h"
#include "Operations/AWF_DragDropOperation.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Operations/AWF_DragDropOperation.h"
#include "Debug/DebugSubsystem.h"
#include "WW_TagLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/HelperUI/InventorySearchSortWidget.h"

// ============================================================================
// WIDGET LIFECYCLE
// ============================================================================

UInventoryResizableWindowWidget::UInventoryResizableWindowWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WidgetCategoryTag = FWWTagLibrary::UI_Widget_Category_Window();
    bAutoRegister = true;
}

void UInventoryResizableWindowWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Set title
    if (Text_Title)
    {
        Text_Title->SetText(WindowTitle);
    }
    
    // Bind close button
    if (Button_Close)
    {
        Button_Close->OnClicked.AddDynamic(this, &UInventoryResizableWindowWidget::OnCloseButtonClicked);
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("✅ InventoryResizableWindowWidget constructed"));
}

void UInventoryResizableWindowWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // Close any active context menu when inventory window closes
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UWidgetManager* WidgetMgr = UWidgetManager::Get(PC))
        {
            WidgetMgr->CloseActiveContextMenu();
            
            UE_LOG(LogTemp, Log, TEXT("Inventory window closed - context menu destroyed"));
        }
    }
    
    // Unbind close button
    if (Button_Close)
    {
        Button_Close->OnClicked.RemoveDynamic(this, &UInventoryResizableWindowWidget::OnCloseButtonClicked);
    }
    

}

// ============================================================================
// DRAG AND DROP SYSTEM (PROPER UMG PATTERN)
// ============================================================================

FReply UInventoryResizableWindowWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // DON'T start drag if clicking on search widget
    if (SearchSortWidget && SearchSortWidget->IsHovered())
    {
        return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    }

    // Check if left mouse button is pressed and cursor is over the title bar
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && Border_TitleBar && Border_TitleBar->IsHovered())
    {
        // Calculate the drag offset (where on the widget the user clicked)
        DragOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        
        UE_LOG(LogInventoryInteractableSystem, Log, TEXT("🖱️ Starting drag detection. Offset: %.1f, %.1f"), DragOffset.X, DragOffset.Y);
        
        // Return handled and detect drag with the title bar
        // This will trigger NativeOnDragDetected when the mouse moves
        return UWidgetBlueprintLibrary::DetectDragIfPressed(
            InMouseEvent,
            this,
            EKeys::LeftMouseButton
        ).NativeReply;
    }
    
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventoryResizableWindowWidget::NativeOnDragDetected(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent,
    UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    UAWF_DragDropOperation* DragDropOp = NewObject<UAWF_DragDropOperation>(this);
    if (!DragDropOp)
    {
        return;
    }

    // Store reference to the REAL widget
    DragDropOp->WidgetReference = this;
    DragDropOp->DragOffset = DragOffset;

    // 🔥 CREATE A SEPARATE DRAG VISUAL
    UInventoryResizableWindowWidget* DragVisual =
        CreateWidget<UInventoryResizableWindowWidget>(
            GetWorld(),
            GetClass()
        );

    DragVisual->SetVisibility(ESlateVisibility::HitTestInvisible);

    DragDropOp->DefaultDragVisual = DragVisual;
    DragDropOp->Pivot = EDragPivot::MouseDown;

    // Keep the real widget in place (but non-interactive)
    //SetVisibility(ESlateVisibility::SelfHitTestInvisible);

    OutOperation = DragDropOp;
}

void UInventoryResizableWindowWidget::NativeOnDragCancelled(
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

    //SetVisibility(ESlateVisibility::Visible);
}


bool UInventoryResizableWindowWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

    UAWF_DragDropOperation* WindowDragOp =
        Cast<UAWF_DragDropOperation>(InOperation);

    if (!WindowDragOp || !WindowDragOp->WidgetReference)
    {
        UE_LOG(LogInventoryInteractableSystem, Error,
            TEXT("❌ Invalid drag operation"));
        return false;
    }

    UInventoryResizableWindowWidget* DraggedWidget =
        Cast<UInventoryResizableWindowWidget>(WindowDragOp->WidgetReference);

    if (!DraggedWidget)
    {
        UE_LOG(LogInventoryInteractableSystem, Error,
            TEXT("❌ Widget cast failed"));
        return false;
    }

    // Convert mouse position to LOCAL space of the DROP TARGET
    const FVector2D LocalMousePosition =
        InGeometry.AbsoluteToLocal(
            InDragDropEvent.GetScreenSpacePosition()
        );

    // Apply stored offset
    const FVector2D NewPosition =
        LocalMousePosition - WindowDragOp->DragOffset;

    // 🔥 IMPORTANT PART — viewport window movement
    DraggedWidget->SetPositionInViewport(
        NewPosition,
        false   // keep DPI scaling
    );

    DraggedWidget->SetVisibility(ESlateVisibility::Visible);

    UE_LOG(LogInventoryInteractableSystem, Log,
        TEXT("✅ Window dropped at %.1f, %.1f"),
        NewPosition.X, NewPosition.Y);

    return true;
}



// ============================================================================
// PUBLIC API
// ============================================================================

void UInventoryResizableWindowWidget::InitializeWindow(UInventoryComponent* InInventoryComponent)
{
    if (!InInventoryComponent)
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ InitializeWindow: InventoryComponent is NULL!"));
        return;
    }
    
    InventoryComponent = InInventoryComponent;
    
    // Initialize the grid
    if (InventoryGrid)
    {
        InventoryGrid->InitializeGrid(InInventoryComponent);
        UE_LOG(LogInventoryInteractableSystem, Log, TEXT("✅ InventoryResizableWindowWidget initialized with grid"));
    }
    else
    {
        UE_LOG(LogInventoryInteractableSystem, Error, TEXT("❌ InventoryGrid is NULL!"));
    }
    if (SearchSortWidgetClass)
    {
        SearchSortWidget = CreateWidget<USearchSortWidget_Master>(GetOwningPlayer(),SearchSortWidgetClass);
        if (SearchSortWidget)
        {
            //Init
            UInventorySearchSortWidget* InventorySearchSortWidget = Cast<UInventorySearchSortWidget>(SearchSortWidget);
            if (InventorySearchSortWidget)
            {
                InventorySearchSortWidget->SetInventoryGrid(InventoryGrid->GridPanel);
            }
        }
        
    }
}

void UInventoryResizableWindowWidget::ToggleWindow()
{
    if (IsVisible())
    {
        HideWindow();
    }
    else
    {
        ShowWindow();
    }
}

void UInventoryResizableWindowWidget::ShowWindow()
{
    SetVisibility(ESlateVisibility::Visible);
    
    // Get actual owning player controller from InventoryComponent
    APlayerController* PC = nullptr;
    if (InventoryComponent)
    {
        if (APawn* OwnerPawn = Cast<APawn>(InventoryComponent->GetOwner()))
        {
            PC = Cast<APlayerController>(OwnerPawn->GetController());
        }
    }
    
    if (PC)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(true);
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("Inventory window shown"));
}

void UInventoryResizableWindowWidget::HideWindow()
{
    SetVisibility(ESlateVisibility::Collapsed);
    
    // Get actual owning player controller from InventoryComponent
    APlayerController* PC = nullptr;
    if (InventoryComponent)
    {
        if (APawn* OwnerPawn = Cast<APawn>(InventoryComponent->GetOwner()))
        {
            PC = Cast<APlayerController>(OwnerPawn->GetController());
        }
    }
    
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("Inventory window hidden"));
}

void UInventoryResizableWindowWidget::SetWindowSize(FVector2D NewSize)
{
    // Clamp to min/max
    NewSize.X = FMath::Clamp(NewSize.X, MinSize.X, MaxSize.X);
    NewSize.Y = FMath::Clamp(NewSize.Y, MinSize.Y, MaxSize.Y);
    
    CurrentSize = NewSize;
    
    // Use WidgetLayoutLibrary for canvas slot access
    if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(this))
    {
        CanvasSlot->SetSize(CurrentSize);
        UE_LOG(LogInventoryInteractableSystem, Log, TEXT("Window resized to: %.0fx%.0f"), CurrentSize.X, CurrentSize.Y);
    }
}

// ============================================================================
// BUTTON CALLBACKS
// ============================================================================

void UInventoryResizableWindowWidget::OnCloseButtonClicked()
{
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryUICloseRequested.Broadcast();
        UE_LOG(LogInventoryInteractableSystem, Log, TEXT("Inventory window closed via button"));
    }
    else
    {
        UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_InventoryInteraction(), 
            TEXT("No valid Inventory Component found!"), 
            true, EDebugVerbosity::Critical);
    }
}