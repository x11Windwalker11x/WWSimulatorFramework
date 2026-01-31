// SearchSortWidgetBase.cpp

#include "UI/HelperUI/SearchSortwidget_Master.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Subsystems/WidgetManager.h"
#include "WW_TagLibrary.h"
#include "UI/InventorySlotWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/InventoryResizableWindowWidget.h"

USearchSortWidget_Master::USearchSortWidget_Master(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set widget category for WidgetManager handling
	WidgetCategoryTag = FWWTagLibrary::UI_Widget_Category_ContextMenu();
	bAutoRegister = true;
	
	// Initialize to Custom sort mode
	CurrentSortTag = FWWTagLibrary::UI_Inventory_Sort_Custom();
	LastSortBeforeSearch = CurrentSortTag;
	CurrentActiveSlot = nullptr;
}

void USearchSortWidget_Master::NativeConstruct()
{
	Super::NativeConstruct();

	// Get player controller and widget manager
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (OwningPlayer && OwningPlayer->IsLocalPlayerController())
	{
		PC = OwningPlayer;
		WidgetManager = UWidgetManager::Get(PC);
	}

	// Initialize search bar with placeholder
	if (EditableText_SearchBar)
	{
		EditableText_SearchBar->OnTextChanged.AddDynamic(this, &USearchSortWidget_Master::OnSearchTextChanged);
		EditableText_SearchBar->OnTextCommitted.AddDynamic(this, &USearchSortWidget_Master::OnSearchTextCommitted);
		EditableText_SearchBar->SetText(FText::FromString(TEXT("Type...")));
		EditableText_SearchBar->SetToolTipText(FText::GetEmpty());
	}
	
	// Bind overlay button for clearing placeholder
	if (Button_SearchBarOverlay)
	{
		Button_SearchBarOverlay->OnClicked.AddDynamic(this, &USearchSortWidget_Master::OnSearchBarClicked);
		Button_SearchBarOverlay->SetVisibility(ESlateVisibility::Visible); // Start visible
	}

	// Initialize sort cycle order and display names
	InitializeSortCycle();

	// Bind sort button
	if (Button_Sort)
	{
		Button_Sort->OnClicked.AddDynamic(this, &USearchSortWidget_Master::CycleSortMode);
	}

	// Initialize dropdown as hidden
	if (ScrollBox_SearchResultsDropdown)
	{
		ScrollBox_SearchResultsDropdown->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Set initial button text
	if (Text_SortMode)
	{
		Text_SortMode->SetText(GetSortButtonText());
	}
}

void USearchSortWidget_Master::InitializeSortCycle()
{
	// Build the sort cycle order (13 modes total)
	SortCycleOrder = {
		FWWTagLibrary::UI_Inventory_Sort_Custom(),
		FWWTagLibrary::UI_Inventory_Sort_Name_Ascending(),
		FWWTagLibrary::UI_Inventory_Sort_Name_Descending(),
		FWWTagLibrary::UI_Inventory_Sort_Rarity_Ascending(),
		FWWTagLibrary::UI_Inventory_Sort_Rarity_Descending(),
		FWWTagLibrary::UI_Inventory_Sort_Type_Ascending(),
		FWWTagLibrary::UI_Inventory_Sort_Type_Descending(),
		FWWTagLibrary::UI_Inventory_Sort_Weight_Ascending(),
		FWWTagLibrary::UI_Inventory_Sort_Weight_Descending(),
		FWWTagLibrary::UI_Inventory_Sort_Value_Ascending(),
		FWWTagLibrary::UI_Inventory_Sort_Value_Descending(),
		FWWTagLibrary::UI_Inventory_Sort_Quantity_Ascending(),
		FWWTagLibrary::UI_Inventory_Sort_Quantity_Descending()
	};

	// Build display name mappings
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Custom(), FText::FromString(TEXT("Custom")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Name_Ascending(), FText::FromString(TEXT("Name ↑")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Name_Descending(), FText::FromString(TEXT("Name ↓")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Rarity_Ascending(), FText::FromString(TEXT("Rarity ↑")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Rarity_Descending(), FText::FromString(TEXT("Rarity ↓")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Type_Ascending(), FText::FromString(TEXT("Type ↑")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Type_Descending(), FText::FromString(TEXT("Type ↓")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Weight_Ascending(), FText::FromString(TEXT("Weight ↑")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Weight_Descending(), FText::FromString(TEXT("Weight ↓")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Value_Ascending(), FText::FromString(TEXT("Value ↑")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Value_Descending(), FText::FromString(TEXT("Value ↓")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Quantity_Ascending(), FText::FromString(TEXT("Quantity ↑")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Quantity_Descending(), FText::FromString(TEXT("Quantity ↓")));
	SortDisplayNames.Add(FWWTagLibrary::UI_Inventory_Sort_Search(), FText::FromString(TEXT("Search")));
}

void USearchSortWidget_Master::OnSearchBarClicked()
{
	UE_LOG(LogTemp, Error, TEXT("🔴 OnSearchBarClicked START"));
    
	if (!EditableText_SearchBar)
	{
		UE_LOG(LogTemp, Error, TEXT("🔴 No EditableText_SearchBar"));
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("🔴 EditableText visibility BEFORE: %d"), (int32)EditableText_SearchBar->GetVisibility());
	UE_LOG(LogTemp, Error, TEXT("🔴 Button visibility BEFORE: %d"), Button_SearchBarOverlay ? (int32)Button_SearchBarOverlay->GetVisibility() : -1);
    
	FString CurrentText = EditableText_SearchBar->GetText().ToString();
    
	if (CurrentText.Contains(TEXT("Type..."), ESearchCase::IgnoreCase))
	{
		EditableText_SearchBar->SetText(FText::GetEmpty());
		UE_LOG(LogTemp, Error, TEXT("🔴 Cleared text"));
	}
    
	EditableText_SearchBar->SetKeyboardFocus();
	UE_LOG(LogTemp, Error, TEXT("🔴 Set focus"));
    
	if (Button_SearchBarOverlay)
	{
		Button_SearchBarOverlay->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Error, TEXT("🔴 Button collapsed"));
	}
    
	UE_LOG(LogTemp, Error, TEXT("🔴 EditableText visibility AFTER: %d"), (int32)EditableText_SearchBar->GetVisibility());
    
	// ... rest
    
	UE_LOG(LogTemp, Error, TEXT("🔴 OnSearchBarClicked END"));
}

void USearchSortWidget_Master::OnKeyboardFocusRecivedforSearchBar()
{
	// Keep UI mode when search bar is focused
	if (PC && PC->IsLocalPlayerController())
	{
		
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(EditableText_SearchBar->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
		FString InfoText = FString::Printf(TEXT("OnKeyboardFocusRecivedforSearchBar(), Widget: %s"), *GetName());
		UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Widget(), InfoText, false, EDebugVerbosity::Warning);
	}

}

void USearchSortWidget_Master::PopulateDropDownWidget(const FText& SearchTerm)
{
	if (!ScrollBox_SearchResultsDropdown || !SearchResultEntryClass)
	{
		UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Widget(), TEXT("EARLY EXIT - Missing ScrollBox or EntryClass"), true, EDebugVerbosity::Error);
		return;
	}
}


void USearchSortWidget_Master::OnSearchTextChanged(const FText& NewText)
{
	UE_LOG(LogTemp, Warning, TEXT("🟡 OnSearchTextChanged: '%s'"), *NewText.ToString());

	if (!EditableText_SearchBar)
	{
		return;
	}
	
	FString SearchString = NewText.ToString();

	// Ignore if it's exactly the placeholder
	if (SearchString.Equals(TEXT("Type..."), ESearchCase::IgnoreCase))
	{
		return;
	}

	// First character typed -> focus first slot if no active slot
	if (SearchString.Len() == 1 && !CurrentActiveSlot)
	{
		UInventorySlotWidget* FirstSlot = GetFirstSlotWidget();
		if (FirstSlot)
		{
			SetActiveSlot(FirstSlot);
		}
	}

	// Handle empty search (user cleared all text)
	if (SearchString.IsEmpty())
	{
		// Close dropdown
		if (ScrollBox_SearchResultsDropdown)
		{
			ScrollBox_SearchResultsDropdown->SetVisibility(ESlateVisibility::Collapsed);
		}
		
		// Restore last sort mode
		CurrentSortTag = LastSortBeforeSearch;
		ApplySortByTag(CurrentSortTag);
		RefreshGridVisuals();
		
		// Update sort button text
		if (Text_SortMode)
		{
			Text_SortMode->SetText(GetSortButtonText());
		}
		
		// DON'T restore placeholder here - let OnSearchTextCommitted handle it
		return;
	}

	// Active search -> switch to Search mode
	if (CurrentSortTag != FWWTagLibrary::UI_Inventory_Sort_Search())
	{
		LastSortBeforeSearch = CurrentSortTag;
		CurrentSortTag = FWWTagLibrary::UI_Inventory_Sort_Search();
	}
	
	// Populate dropdown with matching results
	PopulateSearchDropdown(SearchString);
	
	// Show/hide dropdown based on results
	if (ScrollBox_SearchResultsDropdown)
	{
		ScrollBox_SearchResultsDropdown->SetVisibility(
			FilteredSlotIndices.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
		);
	}
	
	// Dim non-matching slots
	RefreshGridVisuals();
	
	// Update sort button text
	if (Text_SortMode)
	{
		Text_SortMode->SetText(GetSortButtonText());
	}
}

void USearchSortWidget_Master::OnSearchTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	UE_LOG(LogTemp, Error, TEXT("🔴 OnSearchTextCommitted called! Method: %d"), (int32)CommitMethod);

	if (!EditableText_SearchBar)
	{
		return;
	}

	FString SearchText = EditableText_SearchBar->GetText().ToString();
	FText PlaceholderText = FText::FromString(TEXT("Type..."));
	
	// User clicked away or cleared text
	if (CommitMethod == ETextCommit::OnCleared || CommitMethod == ETextCommit::OnUserMovedFocus)
	{
		RefreshGridVisuals();
		
		if (SearchText.IsEmpty())
		{
			// Restore placeholder
			EditableText_SearchBar->SetText(PlaceholderText);
			
			// Show button overlay again
			if (Button_SearchBarOverlay)
			{
				Button_SearchBarOverlay->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
	// User pressed Enter
	else if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::Default)
	{
		// Close dropdown
		if (ScrollBox_SearchResultsDropdown)
		{
			ScrollBox_SearchResultsDropdown->SetVisibility(ESlateVisibility::Collapsed);
		}
		
		// Restore placeholder
		EditableText_SearchBar->SetText(PlaceholderText);
		
		// Show button overlay again
		if (Button_SearchBarOverlay)
		{
			Button_SearchBarOverlay->SetVisibility(ESlateVisibility::Visible);
		}
		
		// Return focus to inventory window
		UUserWidget* InventoryWindow = nullptr;
		if (WidgetManager)
		{
			InventoryWindow = WidgetManager->GetWidget(UInventoryResizableWindowWidget::StaticClass());
		}
		//RefreshGridVisuals
		RefreshGridVisuals();
		if (InventoryWindow && PC && PC->IsLocalPlayerController())
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(InventoryWindow->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
		else if (!InventoryWindow)
		{
			UE_LOG(LogDebugSystem, Warning, TEXT("SearchWidget: Could not find InventoryWindow to return focus"));
		}
	}
}

FReply USearchSortWidget_Master::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Handle ESC key while search bar has focus
	if (InKeyEvent.GetKey() == EKeys::Escape && EditableText_SearchBar && EditableText_SearchBar->HasKeyboardFocus())
	{
		// Close dropdown
		if (ScrollBox_SearchResultsDropdown)
		{
			ScrollBox_SearchResultsDropdown->SetVisibility(ESlateVisibility::Collapsed);
		}
		//RefreshGridVisuals
		RefreshGridVisuals();
		
		// Clear search text and restore placeholder
		EditableText_SearchBar->SetText(FText::FromString(TEXT("Type...")));
		
		// Show button overlay
		if (Button_SearchBarOverlay)
		{
			Button_SearchBarOverlay->SetVisibility(ESlateVisibility::Visible);
		}
		
		// Return focus to CurrentActiveSlot (or clear focus if none)
		if (CurrentActiveSlot && CurrentActiveSlot->IsValidLowLevel())
		{
			CurrentActiveSlot->SetKeyboardFocus();
		}
		else
		{
			FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
		}
		
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void USearchSortWidget_Master::OnSearchBarFocusReceived()
{
	// Reopen dropdown if there's actual search text (not placeholder)
	if (EditableText_SearchBar && ScrollBox_SearchResultsDropdown)
	{
		FString CurrentText = EditableText_SearchBar->GetText().ToString();
		
		if (!CurrentText.IsEmpty() && !CurrentText.Equals(TEXT("Type..."), ESearchCase::IgnoreCase))
		{
			ScrollBox_SearchResultsDropdown->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void USearchSortWidget_Master::CycleSortMode()
{
	// Don't cycle if in Search mode - must clear search first
	if (CurrentSortTag == FWWTagLibrary::UI_Inventory_Sort_Search())
	{
		return;
	}

	// Find current index in cycle order
	int32 CurrentIndex = SortCycleOrder.IndexOfByKey(CurrentSortTag);
	
	if (CurrentIndex == INDEX_NONE)
	{
		// Shouldn't happen, but fallback to Custom
		CurrentSortTag = FWWTagLibrary::UI_Inventory_Sort_Custom();
		CurrentIndex = 0;
	}
	else
	{
		// Advance to next mode (wrap around)
		CurrentIndex = (CurrentIndex + 1) % SortCycleOrder.Num();
		CurrentSortTag = SortCycleOrder[CurrentIndex];
	}

	// Apply new sort
	ApplySortByTag(CurrentSortTag);
	
	// Update button text
	if (Text_SortMode)
	{
		Text_SortMode->SetText(GetSortButtonText());
	}
}

FText USearchSortWidget_Master::GetSortButtonText() const
{
	const FText* DisplayName = SortDisplayNames.Find(CurrentSortTag);
	return DisplayName ? *DisplayName : FText::FromString(TEXT("Unknown"));
}

void USearchSortWidget_Master::RevertToCustomSort()
{
	CurrentSortTag = FWWTagLibrary::UI_Inventory_Sort_Custom();
	LastSortBeforeSearch = CurrentSortTag;
	
	// Update button text
	if (Text_SortMode)
	{
		Text_SortMode->SetText(GetSortButtonText());
	}
	
	// Apply Custom sort
	ApplySortByTag(CurrentSortTag);
}

void USearchSortWidget_Master::NotifyItemDragDropped()
{
	RevertToCustomSort();
}