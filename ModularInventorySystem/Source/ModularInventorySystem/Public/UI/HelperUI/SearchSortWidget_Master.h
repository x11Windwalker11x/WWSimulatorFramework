// SearchSortWidgetBase.h
// Generic search/sort functionality for any item grid widget
// Child classes implement inventory-specific logic

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "GameplayTagContainer.h"
#include "WW_SearchResultEntryWidget.h"
#include "Subsystems/InventoryWidgetManager.h"
#include "SearchSortWidget_Master.generated.h"

class UEditableText;
class UScrollBox;
class UButton;
class UTextBlock;
class UInventorySlotWidget;
class UDataTable;
struct FInventorySlot;
struct FItemData;

/**
 * Abstract base class for search and sort functionality in grid-based widgets
 * Handles:
 * - Search bar with dropdown autocomplete (top 5 results)
 * - Sort cycling through multiple modes with GameplayTags
 * - ESC key handling to return focus to active slot
 * - Context menu cleanup on search bar click
 */
UCLASS(Abstract)
class MODULARINVENTORYSYSTEM_API USearchSortWidget_Master : public UManagedWidget_Master
{
	GENERATED_BODY()

public:
	USearchSortWidget_Master(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(Transient)
	APlayerController* PC;
	UPROPERTY(Transient)
	UInventoryWidgetManager* WidgetManager;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UEditableText* EditableText_SearchBar;
	
	/** Widget class for search result dropdown entries */
	UPROPERTY(EditDefaultsOnly, Category = "Search")
	TSubclassOf<UWW_SearchResultEntryWidget> SearchResultEntryClass;
	
	
	virtual void OnSearchTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	virtual void OnKeyboardFocusRecivedforSearchBar();
	virtual void PopulateDropDownWidget(const FText& SearchTerm);
	UFUNCTION()
	virtual void OnSearchBarClicked();

	/** Dropdown list showing top 5 search results */
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_SearchResultsDropdown;

	/** Button to cycle through sort modes */
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Sort;

	//Button to delete the init text on press for editabletext_searchbar
	UPROPERTY(meta = (BindWidget))
	UButton* Button_SearchBarOverlay;

	/** Text display for current sort mode (e.g. "Name ↑", "Rarity ↓", "Search") */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SortMode;

	// ============================================================================
	// STATE TRACKING
	// ============================================================================

	/** Current active sort mode tag (e.g. UI.Inventory.Sort.Name.Ascending) */
	UPROPERTY()
	FGameplayTag CurrentSortTag;

	/** Cached sort mode before entering search - restored when search is cleared */
	UPROPERTY()
	FGameplayTag LastSortBeforeSearch;

	/** Ordered list of sort tags to cycle through (13 modes total) */
	UPROPERTY()
	TArray<FGameplayTag> SortCycleOrder;

	/** Maps sort tags to display text (e.g. "Name ↑", "Rarity ↓") */
	UPROPERTY()
	TMap<FGameplayTag, FText> SortDisplayNames;

	/** Currently focused/selected slot widget - preserved across ESC key presses */
	UPROPERTY()
	UInventorySlotWidget* CurrentActiveSlot;

	/** All slots matching current search filter (array of slot indices) */
	UPROPERTY()
	TArray<int32> FilteredSlotIndices;

	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the sort cycle order and display name mappings
	 * Sets up all 13 sort modes in proper cycle order
	 */
	void InitializeSortCycle();

	// ============================================================================
	// SEARCH FUNCTIONALITY
	// ============================================================================

	/**
	 * Called when search text changes
	 * Handles:
	 * - First character typed -> focus first slot
	 * - Empty text -> restore last sort mode
	 * - Active search -> switch to Search mode, populate dropdown
	 */
	UFUNCTION()
	virtual void OnSearchTextChanged(const FText& NewText);

	/**
	 * Called when SearchBar is clicked
	 * Destroys all context menus and reopens dropdown if search text exists
	 */
	

	/**
	 * Called when SearchBar receives focus
	 * Reopens dropdown if search text exists
	 */
	UFUNCTION()
	void OnSearchBarFocusReceived();

	// ============================================================================
	// SORT FUNCTIONALITY
	// ============================================================================

	/**
	 * Cycle to next sort mode in SortCycleOrder
	 * Updates button text and applies new sort
	 */
	UFUNCTION()
	void CycleSortMode();

	/**
	 * Get display text for current sort mode
	 * @return Text like "Custom", "Name ↑", "Rarity ↓", "Search"
	 */
	FText GetSortButtonText() const;

	/**
	 * Revert to Custom sort mode
	 * Called when player manually drags/drops an item
	 */
	void RevertToCustomSort();

	// ============================================================================
	// ABSTRACT INTERFACE - Child Classes Must Implement
	// ============================================================================

	/**
	 * Populate search dropdown with top 5 matching results
	 * @param SearchTerm - The text to search for
	 */
	virtual void PopulateSearchDropdown(const FString& SearchTerm) PURE_VIRTUAL(USearchSortWidgetBase::PopulateSearchDropdown,);

	/**
	 * Apply the specified sort mode to the grid
	 * @param SortTag - The sort mode tag to apply
	 */
	virtual void ApplySortByTag(const FGameplayTag& SortTag) PURE_VIRTUAL(USearchSortWidgetBase::ApplySortByTag,);

	/**
	 * Refresh grid visuals (dim non-matching slots to 30% opacity)
	 */
	virtual void RefreshGridVisuals() PURE_VIRTUAL(USearchSortWidgetBase::RefreshGridVisuals,);

	/**
	 * Handle dropdown result selection
	 * @param SlotIndex - The slot index that was clicked
	 */
	virtual void OnSearchResultClicked(int32 SlotIndex) PURE_VIRTUAL(USearchSortWidgetBase::OnSearchResultClicked,);

	/**
	 * Check if a slot matches the search term
	 * Searches: Name, Type, Description, Rarity
	 * @param SlotIndex - Slot index to check
	 * @param SearchTerm - Text to match against
	 * @return True if slot matches search term
	 */
	virtual bool DoesSlotMatchSearch(int32 SlotIndex, const FString& SearchTerm) PURE_VIRTUAL(USearchSortWidgetBase::DoesSlotMatchSearch, return false;);

	/**
	 * Set the currently active/focused slot widget
	 * @param SlotWidget - Slot to make active
	 */
	virtual void SetActiveSlot(UInventorySlotWidget* SlotWidget) PURE_VIRTUAL(USearchSortWidgetBase::SetActiveSlot,);

	/**
	 * Get the first slot widget in the grid
	 * @return First slot widget or nullptr
	 */
	virtual UInventorySlotWidget* GetFirstSlotWidget() PURE_VIRTUAL(USearchSortWidgetBase::GetFirstSlotWidget, return nullptr;);

	/**
	 * Get slot data from index
	 * @param SlotIndex - Slot index to retrieve
	 * @return Pointer to slot data or nullptr
	 */
	virtual FInventorySlot* GetSlotData(int32 SlotIndex) PURE_VIRTUAL(USearchSortWidgetBase::GetSlotData, return nullptr;);

	/**
	 * Get item data from DataTable
	 * @param ItemID - Item ID to look up
	 * @return Pointer to item data or nullptr
	 */
	virtual FItemData* GetItemData(FName ItemID) PURE_VIRTUAL(USearchSortWidgetBase::GetItemData, return nullptr;);

public:
	/**
	 * Called by child when player drags/drops an item
	 * Reverts sort mode to Custom
	 */
	void NotifyItemDragDropped();
};








