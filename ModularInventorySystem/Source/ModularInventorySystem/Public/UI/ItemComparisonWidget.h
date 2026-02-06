// ItemComparisonWidget.h
// Side-by-side item comparison widget for equip/swap decisions

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "Engine/StreamableManager.h"
#include "ItemComparisonWidget.generated.h"

class UTextBlock;
class UImage;
class UVerticalBox;
class UProgressBar;
class UComparisonStatRowWidget;

/**
 * UItemComparisonWidget
 * Displays two items side-by-side with stat comparison rows.
 * Uses FItemPreviewData for each item and generates FStatComparisonEntry rows.
 * Spawns UComparisonStatRowWidget children for each shared stat.
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UItemComparisonWidget : public UManagedWidget_Master
{
	GENERATED_BODY()

public:
	/**
	 * Set both items for comparison.
	 * @param CurrentItem - The currently equipped/held item
	 * @param CompareItem - The item being compared against
	 */
	UFUNCTION(BlueprintCallable, Category = "Comparison")
	void SetComparisonItems(const FItemPreviewData& CurrentItem, const FItemPreviewData& CompareItem);

	/** Clear comparison and hide widget */
	UFUNCTION(BlueprintCallable, Category = "Comparison")
	void ClearComparison();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ============================================================================
	// BOUND WIDGETS - CURRENT ITEM (LEFT)
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CurrentName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_CurrentIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CurrentRarity;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CurrentValue;

	// ============================================================================
	// BOUND WIDGETS - COMPARE ITEM (RIGHT)
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CompareName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_CompareIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CompareRarity;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CompareValue;

	// ============================================================================
	// BOUND WIDGETS - STAT ROWS CONTAINER
	// ============================================================================

	/** Container for dynamically spawned stat row widgets */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> Box_StatRows;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Widget class for stat comparison rows */
	UPROPERTY(EditDefaultsOnly, Category = "Comparison|Config")
	TSubclassOf<UComparisonStatRowWidget> StatRowWidgetClass;

	/** Color for better stat highlighting */
	UPROPERTY(EditDefaultsOnly, Category = "Comparison|Colors")
	FLinearColor BetterColor = FLinearColor::Green;

	/** Color for worse stat highlighting */
	UPROPERTY(EditDefaultsOnly, Category = "Comparison|Colors")
	FLinearColor WorseColor = FLinearColor::Red;

	/** Color for equal stats */
	UPROPERTY(EditDefaultsOnly, Category = "Comparison|Colors")
	FLinearColor NeutralColor = FLinearColor::White;

	// ============================================================================
	// BLUEPRINT EVENTS
	// ============================================================================

	/** Called after comparison data is set - override for custom effects */
	UFUNCTION(BlueprintNativeEvent, Category = "Comparison")
	void OnComparisonSet(const FItemPreviewData& CurrentItem, const FItemPreviewData& CompareItem);

	/** Called when comparison is cleared */
	UFUNCTION(BlueprintNativeEvent, Category = "Comparison")
	void OnComparisonCleared();

private:
	/** Populate item display (left or right side) */
	void PopulateItemDisplay(const FItemPreviewData& ItemData, UTextBlock* NameText, UImage* IconImage, UTextBlock* RarityText, UTextBlock* ValueText, TSharedPtr<FStreamableHandle>& OutHandle);

	/** Build stat comparison rows from two items' stat maps */
	void BuildStatRows(const FItemPreviewData& CurrentItem, const FItemPreviewData& CompareItem);

	/** Clear all spawned stat row widgets */
	void ClearStatRows();

	/** Async icon loading handles */
	TSharedPtr<FStreamableHandle> CurrentIconHandle;
	TSharedPtr<FStreamableHandle> CompareIconHandle;

	/** Spawned stat row widget references */
	UPROPERTY()
	TArray<TObjectPtr<UComparisonStatRowWidget>> SpawnedStatRows;
};
