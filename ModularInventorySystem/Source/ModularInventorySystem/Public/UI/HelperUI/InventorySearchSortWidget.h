

#pragma once

#include "CoreMinimal.h"
#include "UI/HelperUI/SearchSortWidget_Master.h"
#include "Lib/Data/ModularInventorySystem/InvnetoryData.h"
#include "InventorySearchSortWidget.generated.h"

class UInventoryComponent;
class UInventorySlotWidget;
class UUniformGridPanel;
class UWW_SearchResultEntryWidget;

class UDataTable;
struct FItemData;



/**
 * Inventory-specific search and sort widget implementation
 * Manages:
 * - 200-slot inventory grid (PlayerInventory only)
 * - Search filtering across Name, Type, Description, Rarity
 * - Sort by Name, Rarity, Type, Weight, Value, Quantity
 * - Destiny 2-style dimming (non-matching slots at 30% opacity)
 * - Dropdown with top 5 search results
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UInventorySearchSortWidget : public USearchSortWidget_Master
{
	GENERATED_BODY()

public:
	UInventorySearchSortWidget(const FObjectInitializer& ObjectInitializer);
	virtual FInventorySlot* GetSlotData(int32 SlotIndex) override;
	virtual FItemData* GetItemData(FName ItemID) override;
	virtual void OnSearchResultClicked(int32 SlotIndex) override;
	/**
 * Set reference to the inventory grid to manipulate
 * @param Grid - The UniformGridPanel containing slot widgets
 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventoryGrid(UUniformGridPanel* Grid);

protected:
	// ============================================================================
	// WIDGET BINDINGS
	// ============================================================================
	

	virtual void OnKeyboardFocusRecivedforSearchBar() override;
	UFUNCTION()
	virtual void OnSearchTextCommitted(const FText& Text, ETextCommit::Type CommitMethod) override;
	
	UFUNCTION(BlueprintCallable, Category = "Search")
	void ClearSearchText();


	/** Main inventory grid container */
	UPROPERTY(meta = (ToolTip = "Gonna have to set it up in wbp"))
	UUniformGridPanel* InventoryGrid;
	

	// ============================================================================
	// CONFIGURATION
	// ============================================================================



	/** Maximum number of results to show in dropdown */
	UPROPERTY(EditDefaultsOnly, Category = "Search")
	int32 MaxDropdownResults;
	
	UFUNCTION()
	void OnSearchBarMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent);

	// ============================================================================
	// STATE
	// ============================================================================

	/** Reference to player's inventory component */
	UPROPERTY()
	UInventoryComponent* OwnerInventoryComp;

	/** All slot widgets in the grid */
	UPROPERTY()
	TArray<UInventorySlotWidget*> AllSlotWidgets;

	/** Cached original slot order for Custom sort mode */
	UPROPERTY()
	TArray<int32> OriginalSlotOrder;

	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void NativeConstruct() override;

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the widget with inventory component and cache all slots
	 * @param InventoryComp - Player's inventory component
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeWidget(UInventoryComponent* InventoryComp);

	/**
	 * Cache all slot widgets from the grid
	 */
	void CacheSlotWidgets();

	/**
	 * Cache original slot order for Custom sort restoration
	 */
	void CacheOriginalOrder();

	// ============================================================================
	// ABSTRACT INTERFACE IMPLEMENTATION
	// ============================================================================

	virtual void PopulateSearchDropdown(const FString& SearchTerm) override;
	virtual void ApplySortByTag(const FGameplayTag& SortTag) override;
	virtual void RefreshGridVisuals() override;
	virtual bool DoesSlotMatchSearch(int32 SlotIndex, const FString& SearchTerm) override;
	virtual void SetActiveSlot(UInventorySlotWidget* SlotWidget) override;
	virtual UInventorySlotWidget* GetFirstSlotWidget() override;
	
	// ============================================================================
	// SORTING HELPERS
	// ============================================================================

	/**
	 * Sort slots by name (ascending or descending)
	 * @param bAscending - True for A-Z, false for Z-A
	 */
	void SortByName(bool bAscending);

	/**
	 * Sort slots by rarity (ascending: Common->Legendary, descending: Legendary->Common)
	 * @param bAscending - True for Common->Legendary, false for Legendary->Common
	 */
	void SortByRarity(bool bAscending);

	/**
	 * Sort slots by type (alphabetically or by custom priority)
	 * @param bAscending - True for A-Z, false for Z-A
	 */
	void SortByType(bool bAscending);

	/**
	 * Sort slots by weight
	 * @param bAscending - True for Lightest->Heaviest, false for Heaviest->Lightest
	 */
	void SortByWeight(bool bAscending);

	/**
	 * Sort slots by value/price
	 * @param bAscending - True for Cheapest->Expensive, false for Expensive->Cheapest
	 */
	void SortByValue(bool bAscending);

	/**
	 * Sort slots by quantity/stack size
	 * @param bAscending - True for Lowest->Highest, false for Highest->Lowest
	 */
	void SortByQuantity(bool bAscending);

	/**
	 * Sort for search mode - move matching slots to top
	 */
	void SortBySearch();

	/**
	 * Restore original slot order (Custom mode)
	 */
	void RestoreOriginalOrder();

	/**
	 * Apply visual reordering to grid based on FilteredSlotIndices array
	 * Non-matching slots stay in original positions but dimmed
	 */
	void ReorderGridVisually();

	// ============================================================================
	// SEARCH HELPERS
	// ============================================================================

	/**
	 * Filter all slots based on search term
	 * @param SearchTerm - Text to search for
	 * @return Array of matching slot indices
	 */
	TArray<int32> FilterSlotsBySearch(const FString& SearchTerm);

	/**
	 * Highlight/select a slot in the grid
	 * @param SlotIndex - Slot index to highlight
	 */
	void HighlightSlotInGrid(int32 SlotIndex);

	/**
	 * Get rarity sort value for comparison
	 * @param RarityTag - Rarity gameplay tag
	 * @return Integer value (0=Common, 4=Legendary)
	 */
	int32 GetRaritySortValue(const FGameplayTag& RarityTag) const;

	/**
	 * Extract rarity tag from item data's GameplayTags container
	 * @param ItemData - Item to check
	 * @return Rarity tag (e.g. Inventory.Item.Rarity.Common)
	 */
	FGameplayTag GetRarityTag(const FItemData* ItemData) const;

	/**
	 * Extract type tag from item data's GameplayTags container
	 * @param ItemData - Item to check
	 * @return Type tag (e.g. Inventory.Item.Type.Weapon)
	 */
	FGameplayTag GetTypeTag(const FItemData* ItemData) const;

	virtual void OnSearchTextChanged(const FText& NewText) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void OnSearchBarClicked() override;
};