// ContextMenuWidget.cpp
#include "UI/HelperUI/ContextMenu/ContextMenuWidget_Master.h"
#include "WW_TagLibrary.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"

UContextMenuWidget_Master::UContextMenuWidget_Master(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WidgetCategoryTag = FWWTagLibrary::UI_Widget_Category_ContextMenu();
    bAutoRegister = true;
}

void UContextMenuWidget_Master::NativeConstruct()
{
    Super::NativeConstruct();
    
    bCheckOutsideClicks = false;
    
    // Set input mode to game and UI
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
    
    SetKeyboardFocus();
    
    // Wait one frame before checking outside clicks (prevents immediate closure)
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        bCheckOutsideClicks = true;
    });
}

FReply UContextMenuWidget_Master::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bCheckOutsideClicks)
    {
        FVector2D LocalMousePos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        FVector2D MenuSize = InGeometry.GetLocalSize();
        
        // Check if click is outside menu bounds
        if (LocalMousePos.X < 0 || LocalMousePos.X > MenuSize.X ||
            LocalMousePos.Y < 0 || LocalMousePos.Y > MenuSize.Y)
        {
            CloseMenu();
            return FReply::Handled();
        }
    }
    
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UContextMenuWidget_Master::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Close menu on Escape key
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        CloseMenu();
        return FReply::Handled();
    }
    
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UContextMenuWidget_Master::AddMenuItem(const FString& ItemName, const FString& ItemLabel)
{
    if (!MenuItemContainer)
    {
        UE_LOG(LogTemp, Warning, TEXT("MenuItemContainer is null!"));
        return;
    }

    // Create button
    UButton* MenuButton = NewObject<UButton>(this);
    
    // Disable button if it's a header
    if (ItemName == "Header")
    {
        MenuButton->SetIsEnabled(false);
    }
    
    // Create text block
    UTextBlock* ButtonText = NewObject<UTextBlock>(this);
    ButtonText->SetText(FText::FromString(ItemLabel));
    ButtonText->SetJustification(ETextJustify::Left);
    
    FSlateFontInfo FontInfo = ButtonText->GetFont();
    
    // Make header text bold and larger
    if (ItemName == "Header")
    {
        FontInfo.Size = 16;
        ButtonText->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
    }
    else
    {
        FontInfo.Size = 14;
        ButtonText->SetColorAndOpacity(FLinearColor::White);
    }
    
    ButtonText->SetFont(FontInfo);
    
    // Add text to button
    MenuButton->AddChild(ButtonText);
    
    // Store button-to-name mapping
    ButtonToItemNameMap.Add(MenuButton, ItemName);
    
    // Bind click event (only if not a header)
    if (ItemName != "Header")
    {
        MenuButton->OnClicked.AddDynamic(this, &UContextMenuWidget_Master::OnMenuItemButtonClicked);
    }
    
    // Add to container
    UVerticalBoxSlot* ButtonSlot = MenuItemContainer->AddChildToVerticalBox(MenuButton);
    ButtonSlot->SetPadding(FMargin(10.0f, 5.0f));
    ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
}


void UContextMenuWidget_Master::ClearMenuItems()
{
    if (MenuItemContainer)
    {
        MenuItemContainer->ClearChildren();
    }
    
    // Clear the mapping
    ButtonToItemNameMap.Empty();
}

void UContextMenuWidget_Master::OnMenuItemButtonClicked()
{
    // Find which button was clicked by checking which one is hovered
    for (const auto& Pair : ButtonToItemNameMap)
    {
        if (Pair.Key && Pair.Key->IsHovered())
        {
            // Broadcast the event
            OnMenuItemClicked.Broadcast(Pair.Value);
            
            // Close the menu
            CloseMenu();
            return;
        }
    }
}

void UContextMenuWidget_Master::CloseMenu()
{
    // Restore input mode
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
    
    // Remove from viewport
    RemoveFromParent();
}