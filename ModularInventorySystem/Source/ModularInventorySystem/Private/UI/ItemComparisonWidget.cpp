// ItemComparisonWidget.cpp
// Side-by-side item comparison widget for equip/swap decisions

#include "UI/ItemComparisonWidget.h"
#include "UI/ComparisonStatRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"

void UItemComparisonWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UItemComparisonWidget::NativeDestruct()
{
	// Cancel async loads
	if (CurrentIconHandle.IsValid())
	{
		CurrentIconHandle->CancelHandle();
		CurrentIconHandle.Reset();
	}
	if (CompareIconHandle.IsValid())
	{
		CompareIconHandle->CancelHandle();
		CompareIconHandle.Reset();
	}

	ClearStatRows();

	Super::NativeDestruct();
}

void UItemComparisonWidget::SetComparisonItems(const FItemPreviewData& CurrentItem, const FItemPreviewData& CompareItem)
{
	// Populate left side (current item)
	PopulateItemDisplay(CurrentItem, Text_CurrentName, Image_CurrentIcon, Text_CurrentRarity, Text_CurrentValue, CurrentIconHandle);

	// Populate right side (compare item)
	PopulateItemDisplay(CompareItem, Text_CompareName, Image_CompareIcon, Text_CompareRarity, Text_CompareValue, CompareIconHandle);

	// Build stat comparison rows
	BuildStatRows(CurrentItem, CompareItem);

	// Notify BP
	OnComparisonSet(CurrentItem, CompareItem);
}

void UItemComparisonWidget::ClearComparison()
{
	// Clear text displays
	if (Text_CurrentName) Text_CurrentName->SetText(FText::GetEmpty());
	if (Text_CompareName) Text_CompareName->SetText(FText::GetEmpty());
	if (Text_CurrentRarity) Text_CurrentRarity->SetText(FText::GetEmpty());
	if (Text_CompareRarity) Text_CompareRarity->SetText(FText::GetEmpty());
	if (Text_CurrentValue) Text_CurrentValue->SetText(FText::GetEmpty());
	if (Text_CompareValue) Text_CompareValue->SetText(FText::GetEmpty());

	// Clear icons
	if (Image_CurrentIcon) Image_CurrentIcon->SetBrushFromTexture(nullptr);
	if (Image_CompareIcon) Image_CompareIcon->SetBrushFromTexture(nullptr);

	// Cancel async loads
	if (CurrentIconHandle.IsValid())
	{
		CurrentIconHandle->CancelHandle();
		CurrentIconHandle.Reset();
	}
	if (CompareIconHandle.IsValid())
	{
		CompareIconHandle->CancelHandle();
		CompareIconHandle.Reset();
	}

	ClearStatRows();

	OnComparisonCleared();
}

// ============================================================================
// INTERNAL
// ============================================================================

void UItemComparisonWidget::PopulateItemDisplay(const FItemPreviewData& ItemData, UTextBlock* NameText, UImage* IconImage, UTextBlock* RarityText, UTextBlock* ValueText, TSharedPtr<FStreamableHandle>& OutHandle)
{
	// Name
	if (NameText)
	{
		NameText->SetText(ItemData.DisplayName);
	}

	// Value
	if (ValueText)
	{
		ValueText->SetText(FText::FromString(
			FString::Printf(TEXT("%d"), ItemData.Value)
		));
	}

	// Rarity tag as display text
	if (RarityText && ItemData.Rarity.IsValid())
	{
		RarityText->SetText(FText::FromString(ItemData.Rarity.ToString()));
	}

	// Async icon loading
	if (IconImage && !ItemData.Icon.IsNull())
	{
		// Cancel previous load
		if (OutHandle.IsValid())
		{
			OutHandle->CancelHandle();
		}

		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		OutHandle = StreamableManager.RequestAsyncLoad(
			ItemData.Icon.ToSoftObjectPath(),
			FStreamableDelegate::CreateWeakLambda(IconImage, [IconImage, SoftIcon = ItemData.Icon]()
			{
				if (UTexture2D* LoadedTexture = SoftIcon.Get())
				{
					IconImage->SetBrushFromTexture(LoadedTexture);
				}
			})
		);
	}
}

void UItemComparisonWidget::BuildStatRows(const FItemPreviewData& CurrentItem, const FItemPreviewData& CompareItem)
{
	ClearStatRows();

	if (!Box_StatRows || !StatRowWidgetClass)
	{
		return;
	}

	// Collect all unique stat names from both items
	TSet<FName> AllStats;
	for (const auto& Pair : CurrentItem.Stats)
	{
		AllStats.Add(Pair.Key);
	}
	for (const auto& Pair : CompareItem.Stats)
	{
		AllStats.Add(Pair.Key);
	}

	// Create a comparison row for each stat
	for (const FName& StatName : AllStats)
	{
		UComparisonStatRowWidget* RowWidget = CreateWidget<UComparisonStatRowWidget>(GetOwningPlayer(), StatRowWidgetClass);
		if (!RowWidget)
		{
			continue;
		}

		FStatComparisonEntry Entry;
		Entry.StatName = StatName;
		Entry.StatDisplayName = FText::FromName(StatName);
		Entry.CurrentValue = CurrentItem.Stats.Contains(StatName) ? CurrentItem.Stats[StatName] : 0.0f;
		Entry.CompareValue = CompareItem.Stats.Contains(StatName) ? CompareItem.Stats[StatName] : 0.0f;
		Entry.bHigherIsBetter = true; // Default assumption

		RowWidget->SetComparisonData(Entry);

		Box_StatRows->AddChild(RowWidget);
		SpawnedStatRows.Add(RowWidget);
	}
}

void UItemComparisonWidget::ClearStatRows()
{
	if (Box_StatRows)
	{
		Box_StatRows->ClearChildren();
	}
	SpawnedStatRows.Empty();
}

// ============================================================================
// BLUEPRINT EVENTS
// ============================================================================

void UItemComparisonWidget::OnComparisonSet_Implementation(const FItemPreviewData& CurrentItem, const FItemPreviewData& CompareItem)
{
	// Default: no-op - override for custom effects
}

void UItemComparisonWidget::OnComparisonCleared_Implementation()
{
	// Default: no-op
}
