// InventorySearchSortWidget.cpp

#include "UI/HelperUI/InventorySearchSortWidget.h"
#include "Components/ScrollBox.h"
#include "Components/UniformGridPanel.h"
#include "UI/InventorySlotWidget.h"
#include "UI/HelperUI/WW_SearchResultEntryWidget.h"
#include "Components/InventoryComponent.h"
#include "Actors/Interactables/Item/Data/ItemData.h"
#include "WW_TagLibrary.h"
#include "Engine/DataTable.h"
#include "Subsystems/WidgetManager.h"

UInventorySearchSortWidget::UInventorySearchSortWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxDropdownResults = 5;
	OwnerInventoryComp = nullptr;
	InventoryGrid = nullptr;
}

void UInventorySearchSortWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInventorySearchSortWidget::SetInventoryGrid(UUniformGridPanel* Grid)
{
	InventoryGrid = Grid;
	CacheSlotWidgets(); // Re-cache slots from new grid
}

void UInventorySearchSortWidget::OnKeyboardFocusRecivedforSearchBar()
{
	Super::OnKeyboardFocusRecivedforSearchBar();
}

void UInventorySearchSortWidget::OnSearchTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	Super::OnSearchTextCommitted(Text, CommitMethod);
}

void UInventorySearchSortWidget::ClearSearchText()
{
	if (EditableText_SearchBar)
	{
		FString SearchText = EditableText_SearchBar->GetText().ToString();
		FString InitText = FString("Type");
		if (SearchText.Contains(InitText, ESearchCase::IgnoreCase))
		{
			EditableText_SearchBar->SetText(FText::GetEmpty());
		}
	}
}

void UInventorySearchSortWidget::OnSearchTextChanged(const FText& NewText)
{

	FString SearchText = NewText.ToString();
    
	// Clear placeholder text on first keystroke
	if (SearchText.Contains(TEXT("Type"), ESearchCase::IgnoreCase))
	{
		EditableText_SearchBar->SetText(FText::GetEmpty());
		//Restore all highlighted slots
		for (UInventorySlotWidget* InventorySlot : AllSlotWidgets)
		{
			if (InventorySlot) InventorySlot->SetHighlight(false);
		}
		return; // Don't process this change
	}
	
	if (FilteredSlotIndices.Num() > 0)
	{
		for (int32 SlotIndex : FilteredSlotIndices)
		{
			if (SlotIndex >= 0 && SlotIndex < AllSlotWidgets.Num())
			{
				UInventorySlotWidget* SlotWidget = AllSlotWidgets[SlotIndex];
				if (SlotWidget)
				{
					SlotWidget->SetHighlight(true);
				}
			}
		}
	}
	Super::OnSearchTextChanged(NewText);
}

void UInventorySearchSortWidget::InitializeWidget(UInventoryComponent* InventoryComp)
{
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Error, TEXT("UInventorySearchSortWidget::InitializeWidget - Invalid InventoryComponent"));
		return;
	}

	OwnerInventoryComp = InventoryComp;
	CacheSlotWidgets();
	CacheOriginalOrder();
	
	CurrentSortTag = FWWTagLibrary::UI_Inventory_Sort_Custom();
	LastSortBeforeSearch = CurrentSortTag;
}

void UInventorySearchSortWidget::CacheSlotWidgets()
{
	AllSlotWidgets.Empty();
	
	if (!InventoryGrid)
	{
		return;
	}

	// Iterate through all children in the grid
	for (int32 i = 0; i < InventoryGrid->GetChildrenCount(); ++i)
	{
		UInventorySlotWidget* SlotWidget = Cast<UInventorySlotWidget>(InventoryGrid->GetChildAt(i));
		if (SlotWidget)
		{
			AllSlotWidgets.Add(SlotWidget);
		}
	}
}

void UInventorySearchSortWidget::CacheOriginalOrder()
{
	OriginalSlotOrder.Empty();
	
	for (int32 i = 0; i < AllSlotWidgets.Num(); ++i)
	{
		OriginalSlotOrder.Add(i);
	}
}

void UInventorySearchSortWidget::PopulateSearchDropdown(const FString& SearchTerm)
{
	//Called by parent function
	// if (!ScrollBox_SearchResultsDropdown || !SearchResultEntryClass)
	// {
	// 	return;
	// }
	Super::PopulateSearchDropdown(SearchTerm);

	FString InfoText = FString::Printf(TEXT("PopulateSearchDropdown called with: '%s'"), *SearchTerm);
	UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Widget(), InfoText, true, EDebugVerbosity::Warning);
    
	InfoText = FString::Printf(TEXT("ScrollBox valid: %s"), ScrollBox_SearchResultsDropdown ? TEXT("YES") : TEXT("NO"));
	UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Widget(), InfoText, true, EDebugVerbosity::Warning);
    
	InfoText = FString::Printf(TEXT("EntryClass valid: %s"), SearchResultEntryClass ? TEXT("YES") : TEXT("NO"));
	UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_Widget(), InfoText, true, EDebugVerbosity::Warning);


	ScrollBox_SearchResultsDropdown->ClearChildren();
	FilteredSlotIndices = FilterSlotsBySearch(SearchTerm);

	const int32 ResultCount = FMath::Min(FilteredSlotIndices.Num(), MaxDropdownResults);
	for (int32 i = 0; i < ResultCount; ++i)
	{
		UWW_SearchResultEntryWidget* Entry = CreateWidget<UWW_SearchResultEntryWidget>(GetOwningPlayer(), SearchResultEntryClass);
		if (Entry)
		{
			Entry->InitializeEntry(FilteredSlotIndices[i], this);
			ScrollBox_SearchResultsDropdown->AddChild(Entry);
		}
	}
}

void UInventorySearchSortWidget::ApplySortByTag(const FGameplayTag& SortTag)
{
	if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Custom())
	{
		RestoreOriginalOrder();
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Name_Ascending())
	{
		SortByName(true);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Name_Descending())
	{
		SortByName(false);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Rarity_Ascending())
	{
		SortByRarity(true);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Rarity_Descending())
	{
		SortByRarity(false);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Type_Ascending())
	{
		SortByType(true);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Type_Descending())
	{
		SortByType(false);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Weight_Ascending())
	{
		SortByWeight(true);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Weight_Descending())
	{
		SortByWeight(false);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Value_Ascending())
	{
		SortByValue(true);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Value_Descending())
	{
		SortByValue(false);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Quantity_Ascending())
	{
		SortByQuantity(true);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Quantity_Descending())
	{
		SortByQuantity(false);
	}
	else if (SortTag == FWWTagLibrary::UI_Inventory_Sort_Search())
	{
		SortBySearch();
	}

	ReorderGridVisually();
}

void UInventorySearchSortWidget::RefreshGridVisuals()
{
	if (!EditableText_SearchBar)
	{
		return;
	}

	const FString SearchText = EditableText_SearchBar->GetText().ToString();
	const bool bIsSearching = !SearchText.IsEmpty() && !SearchText.Equals(TEXT("Type..."), ESearchCase::IgnoreCase);

	for (int32 i = 0; i < AllSlotWidgets.Num(); ++i)
	{
		UInventorySlotWidget* SlotWidget = AllSlotWidgets[i];
		if (!SlotWidget)
		{
			continue;
		}

		if (bIsSearching)
		{
			const bool bMatches = DoesSlotMatchSearch(i, SearchText);
			SlotWidget->SetRenderOpacity(bMatches ? 1.0f : 0.3f);
		}
		else
		{
			SlotWidget->SetRenderOpacity(1.0f);
		}
	}
}
void UInventorySearchSortWidget::OnSearchBarClicked()
{
	Super::OnSearchBarClicked();
}

void UInventorySearchSortWidget::OnSearchResultClicked(int32 SlotIndex)
{
	HighlightSlotInGrid(SlotIndex);
	
	if (ScrollBox_SearchResultsDropdown)
	{
		ScrollBox_SearchResultsDropdown->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInventorySearchSortWidget::OnSearchBarMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	if (EditableText_SearchBar && MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		EditableText_SearchBar->SetText(FText::GetEmpty());
	}
	else if (EditableText_SearchBar && MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		EditableText_SearchBar->SetText(FText::GetEmpty());
	}
	
}
bool UInventorySearchSortWidget::DoesSlotMatchSearch(int32 SlotIndex, const FString& SearchTerm)
{
	FInventorySlot* InventorySlot = GetSlotData(SlotIndex);
	if (!InventorySlot || InventorySlot->IsEmpty())
	{
		return false;
	}

	FItemData* ItemData = GetItemData(InventorySlot->ItemID);
	if (!ItemData)
	{
		return false;
	}

	FString LowerSearch = SearchTerm.ToLower();
	
	if (ItemData->DisplayName.ToString().ToLower().Contains(LowerSearch))
	{
		return true;
	}
	
	if (ItemData->Description.ToString().ToLower().Contains(LowerSearch))
	{
		return true;
	}
	
	FGameplayTag TypeTag = GetTypeTag(ItemData);
	if (TypeTag.IsValid() && TypeTag.ToString().ToLower().Contains(LowerSearch))
	{
		return true;
	}
	
	FGameplayTag RarityTag = GetRarityTag(ItemData);
	if (RarityTag.IsValid() && RarityTag.ToString().ToLower().Contains(LowerSearch))
	{
		return true;
	}
	
	return false;
}

void UInventorySearchSortWidget::SetActiveSlot(UInventorySlotWidget* SlotWidget)
{
	CurrentActiveSlot = SlotWidget;
	
	if (SlotWidget)
	{
		SlotWidget->SetKeyboardFocus();
	}
}

UInventorySlotWidget* UInventorySearchSortWidget::GetFirstSlotWidget()
{
	return AllSlotWidgets.Num() > 0 ? AllSlotWidgets[0] : nullptr;
}

FInventorySlot* UInventorySearchSortWidget::GetSlotData(int32 SlotIndex)
{
	static FInventorySlot TempSlot;
	if (OwnerInventoryComp && OwnerInventoryComp->GetSlotByIndex(SlotIndex, TempSlot))
	{
		return &TempSlot;
	}
	return nullptr;
}

FItemData* UInventorySearchSortWidget::GetItemData(FName ItemID)
{
	static FItemData TempData;
	if (OwnerInventoryComp && OwnerInventoryComp->GetItemData(ItemID, TempData))
	{
		return &TempData;
	}
	return nullptr;
}

void UInventorySearchSortWidget::SortByName(bool bAscending)
{
	if (!OwnerInventoryComp)
	{
		return;
	}

	TArray<int32> SlotsToSort;
	for (int32 SlotIndex : OriginalSlotOrder)
	{
		FInventorySlot* InventorySlot = GetSlotData(SlotIndex);
		if (InventorySlot && !InventorySlot->IsEmpty())
		{
			SlotsToSort.Add(SlotIndex);
		}
	}

	SlotsToSort.Sort([this, bAscending](int32 A, int32 B)
	{
		FInventorySlot* SlotA = GetSlotData(A);
		FInventorySlot* SlotB = GetSlotData(B);
		
		if (!SlotA || !SlotB) return false;

		FItemData* DataA = GetItemData(SlotA->ItemID);
		FItemData* DataB = GetItemData(SlotB->ItemID);
		
		if (!DataA || !DataB) return false;

		FString NameA = DataA->DisplayName.ToString();
		FString NameB = DataB->DisplayName.ToString();

		return bAscending ? (NameA < NameB) : (NameA > NameB);
	});

	FilteredSlotIndices = SlotsToSort;
}

void UInventorySearchSortWidget::SortByRarity(bool bAscending)
{
	if (!OwnerInventoryComp)
	{
		return;
	}

	TArray<int32> SlotsToSort;
	for (int32 SlotIndex : OriginalSlotOrder)
	{
		FInventorySlot* InventorySlot = GetSlotData(SlotIndex);
		if (InventorySlot && !InventorySlot->IsEmpty())
		{
			SlotsToSort.Add(SlotIndex);
		}
	}

	SlotsToSort.Sort([this, bAscending](int32 A, int32 B)
	{
		FInventorySlot* SlotA = GetSlotData(A);
		FInventorySlot* SlotB = GetSlotData(B);
		
		if (!SlotA || !SlotB) return false;

		FItemData* DataA = GetItemData(SlotA->ItemID);
		FItemData* DataB = GetItemData(SlotB->ItemID);
		
		if (!DataA || !DataB) return false;

		int32 RarityA = GetRaritySortValue(GetRarityTag(DataA));
		int32 RarityB = GetRaritySortValue(GetRarityTag(DataB));

		return bAscending ? (RarityA < RarityB) : (RarityA > RarityB);
	});

	FilteredSlotIndices = SlotsToSort;
}

void UInventorySearchSortWidget::SortByType(bool bAscending)
{
	if (!OwnerInventoryComp)
	{
		return;
	}

	TArray<int32> SlotsToSort;
	for (int32 SlotIndex : OriginalSlotOrder)
	{
		FInventorySlot* InventorySlot = GetSlotData(SlotIndex);
		if (InventorySlot && !InventorySlot->IsEmpty())
		{
			SlotsToSort.Add(SlotIndex);
		}
	}

	SlotsToSort.Sort([this, bAscending](int32 A, int32 B)
	{
		FInventorySlot* SlotA = GetSlotData(A);
		FInventorySlot* SlotB = GetSlotData(B);
		
		if (!SlotA || !SlotB) return false;

		FItemData* DataA = GetItemData(SlotA->ItemID);
		FItemData* DataB = GetItemData(SlotB->ItemID);
		
		if (!DataA || !DataB) return false;

		FString TypeA = GetTypeTag(DataA).ToString();
		FString TypeB = GetTypeTag(DataB).ToString();

		return bAscending ? (TypeA < TypeB) : (TypeA > TypeB);
	});

	FilteredSlotIndices = SlotsToSort;
}

void UInventorySearchSortWidget::SortByWeight(bool bAscending)
{
	if (!OwnerInventoryComp)
	{
		return;
	}

	TArray<int32> SlotsToSort;
	for (int32 SlotIndex : OriginalSlotOrder)
	{
		FInventorySlot* InventorySlot = GetSlotData(SlotIndex);
		if (InventorySlot && !InventorySlot->IsEmpty())
		{
			SlotsToSort.Add(SlotIndex);
		}
	}

	SlotsToSort.Sort([this, bAscending](int32 A, int32 B)
	{
		FInventorySlot* SlotA = GetSlotData(A);
		FInventorySlot* SlotB = GetSlotData(B);
		
		if (!SlotA || !SlotB) return false;

		FItemData* DataA = GetItemData(SlotA->ItemID);
		FItemData* DataB = GetItemData(SlotB->ItemID);
		
		if (!DataA || !DataB) return false;

		return bAscending ? (DataA->Weight < DataB->Weight) : (DataA->Weight > DataB->Weight);
	});

	FilteredSlotIndices = SlotsToSort;
}

void UInventorySearchSortWidget::SortByValue(bool bAscending)
{
	if (!OwnerInventoryComp)
	{
		return;
	}

	TArray<int32> SlotsToSort;
	for (int32 SlotIndex : OriginalSlotOrder)
	{
		FInventorySlot* InventorySlot = GetSlotData(SlotIndex);
		if (InventorySlot && !InventorySlot->IsEmpty())
		{
			SlotsToSort.Add(SlotIndex);
		}
	}

	SlotsToSort.Sort([this, bAscending](int32 A, int32 B)
	{
		FInventorySlot* SlotA = GetSlotData(A);
		FInventorySlot* SlotB = GetSlotData(B);
		
		if (!SlotA || !SlotB) return false;

		FItemData* DataA = GetItemData(SlotA->ItemID);
		FItemData* DataB = GetItemData(SlotB->ItemID);
		
		if (!DataA || !DataB) return false;

		return bAscending ? (DataA->BaseValue < DataB->BaseValue) : (DataA->BaseValue > DataB->BaseValue);
	});

	FilteredSlotIndices = SlotsToSort;
}

void UInventorySearchSortWidget::SortByQuantity(bool bAscending)
{
	if (!OwnerInventoryComp)
	{
		return;
	}

	TArray<int32> SlotsToSort;
	for (int32 SlotIndex : OriginalSlotOrder)
	{
		FInventorySlot* InventorySlot = GetSlotData(SlotIndex);
		if (InventorySlot && !InventorySlot->IsEmpty())
		{
			SlotsToSort.Add(SlotIndex);
		}
	}

	SlotsToSort.Sort([this, bAscending](int32 A, int32 B)
	{
		FInventorySlot* SlotA = GetSlotData(A);
		FInventorySlot* SlotB = GetSlotData(B);
		
		if (!SlotA || !SlotB) return false;

		return bAscending ? (SlotA->Quantity < SlotB->Quantity) : (SlotA->Quantity > SlotB->Quantity);
	});

	FilteredSlotIndices = SlotsToSort;
}

void UInventorySearchSortWidget::SortBySearch()
{
	if (!EditableText_SearchBar)
	{
		return;
	}

	const FString SearchText = EditableText_SearchBar->GetText().ToString();
	FilteredSlotIndices = FilterSlotsBySearch(SearchText);
}

void UInventorySearchSortWidget::RestoreOriginalOrder()
{
	FilteredSlotIndices = OriginalSlotOrder;
}

void UInventorySearchSortWidget::ReorderGridVisually()
{
	// Destiny 2-style: Keep slots in place, just dim non-matches
	// Actual reordering would require recreating grid slots
}

TArray<int32> UInventorySearchSortWidget::FilterSlotsBySearch(const FString& SearchTerm)
{
	TArray<int32> MatchingSlots;

	for (int32 i = 0; i < AllSlotWidgets.Num(); ++i)
	{
		if (DoesSlotMatchSearch(i, SearchTerm))
		{
			MatchingSlots.Add(i);
		}
	}

	return MatchingSlots;
}

void UInventorySearchSortWidget::HighlightSlotInGrid(int32 SlotIndex)
{
	if (SlotIndex >= 0 && SlotIndex < AllSlotWidgets.Num())
	{
		UInventorySlotWidget* SlotWidget = AllSlotWidgets[SlotIndex];
		if (SlotWidget)
		{
			SetActiveSlot(SlotWidget);
		}
	}
}

int32 UInventorySearchSortWidget::GetRaritySortValue(const FGameplayTag& RarityTag) const
{
	if (RarityTag == FWWTagLibrary::Inventory_Item_Rarity_Common()) return 0;
	if (RarityTag == FWWTagLibrary::Inventory_Item_Rarity_Uncommon()) return 1;
	if (RarityTag == FWWTagLibrary::Inventory_Item_Rarity_Rare()) return 2;
	if (RarityTag == FWWTagLibrary::Inventory_Item_Rarity_Epic()) return 3;
	if (RarityTag == FWWTagLibrary::Inventory_Item_Rarity_Legendary()) return 4;
	return 0;
}

FGameplayTag UInventorySearchSortWidget::GetRarityTag(const FItemData* ItemData) const
{
	if (!ItemData)
	{
		return FGameplayTag();
	}

	FGameplayTagContainer Tags = ItemData->GameplayTags;
	
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Rarity_Legendary()))
		return FWWTagLibrary::Inventory_Item_Rarity_Legendary();
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Rarity_Epic()))
		return FWWTagLibrary::Inventory_Item_Rarity_Epic();
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Rarity_Rare()))
		return FWWTagLibrary::Inventory_Item_Rarity_Rare();
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Rarity_Uncommon()))
		return FWWTagLibrary::Inventory_Item_Rarity_Uncommon();
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Rarity_Common()))
		return FWWTagLibrary::Inventory_Item_Rarity_Common();
	
	return FGameplayTag();
}

FGameplayTag UInventorySearchSortWidget::GetTypeTag(const FItemData* ItemData) const
{
	if (!ItemData)
	{
		return FGameplayTag();
	}

	FGameplayTagContainer Tags = ItemData->GameplayTags;
	
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Type_Weapon()))
		return FWWTagLibrary::Inventory_Item_Type_Weapon();
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Type_Armor()))
		return FWWTagLibrary::Inventory_Item_Type_Armor();
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Type_Consumable()))
		return FWWTagLibrary::Inventory_Item_Type_Consumable();
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Type_KeyItem()))
		return FWWTagLibrary::Inventory_Item_Type_KeyItem();
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Type_Crafting()))
		return FWWTagLibrary::Inventory_Item_Type_Crafting();
	if (Tags.HasTag(FWWTagLibrary::Inventory_Item_Type_Quest()))
		return FWWTagLibrary::Inventory_Item_Type_Quest();
	
	return FGameplayTag();
}

FReply UInventorySearchSortWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Handle ENTER key - add all filtered slots to multi-selection
	if (InKeyEvent.GetKey() == EKeys::Enter && EditableText_SearchBar && EditableText_SearchBar->HasKeyboardFocus())
	{
		for (int32 SlotIndex : FilteredSlotIndices)
		{
			if (SlotIndex >= 0 && SlotIndex < AllSlotWidgets.Num())
			{
				UInventorySlotWidget* SlotWidget = AllSlotWidgets[SlotIndex];
				if (SlotWidget && SlotWidget->HasItem() && WidgetManager && WidgetManager->CanMultiSelect(SlotWidget->InventoryType))
				{
					WidgetManager->ToggleSelection(SlotWidget->InventoryType, SlotIndex);
				}
			}
		}
	}
	
	// Call parent for ESC handling and other keys
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}