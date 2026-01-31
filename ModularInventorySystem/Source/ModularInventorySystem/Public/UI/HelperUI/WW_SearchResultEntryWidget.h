#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WW_SearchResultEntryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UInventorySearchSortWidget;
struct FItemData;

/**
 * Dropdown entry widget for search results
 * Displays: Icon, Name, Quantity, Rarity border
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UWW_SearchResultEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Initialize this entry with slot data
	 * @param InSlotIndex - Flat slot index in inventory (0-199)
	 * @param InParentWidget - Parent search widget for callbacks
	 */
	void InitializeEntry(int32 InSlotIndex, UInventorySearchSortWidget* InParentWidget);

protected:
	virtual void NativeConstruct() override;

	// ============================================================================
	// WIDGET BINDINGS
	// ============================================================================

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Entry;

	UPROPERTY(meta = (BindWidget))
	UImage* Image_Icon;

	UPROPERTY(meta = (BindWidget))
	UImage* Image_RarityBorder;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ItemName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Quantity;

	// ============================================================================
	// STATE
	// ============================================================================

	int32 SlotIndex;

	UPROPERTY()
	UInventorySearchSortWidget* ParentSearchWidget;

	// ============================================================================
	// EVENTS
	// ============================================================================

	UFUNCTION()
	void OnEntryClicked();

	FLinearColor GetRarityColor(const FItemData* ItemData) const;
};