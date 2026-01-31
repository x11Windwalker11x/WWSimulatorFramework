// WW_SearchResultEntryWidget.cpp

#include "UI/HelperUI/WW_SearchResultEntryWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/HelperUI/InventorySearchSortWidget.h"
#include "Lib/Data/ModularInventorySystem/InvnetoryData.h"
#include "Actors/Interactables/Item/Data/ItemData.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UWW_SearchResultEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button click
	if (Button_Entry)
	{
		Button_Entry->OnClicked.AddDynamic(this, &UWW_SearchResultEntryWidget::OnEntryClicked);
	}
}

void UWW_SearchResultEntryWidget::InitializeEntry(int32 InSlotIndex, UInventorySearchSortWidget* InParentWidget)
{
	SlotIndex = InSlotIndex;
	ParentSearchWidget = InParentWidget;

	if (!ParentSearchWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UWW_SearchResultEntryWidget::InitializeEntry - Invalid ParentSearchWidget"));
		return;
	}

	// Get slot data
	FInventorySlot* SlotData = ParentSearchWidget->GetSlotData(SlotIndex);
	if (!SlotData|| SlotData->IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UWW_SearchResultEntryWidget::InitializeEntry - Invalid or empty slot at index %d"), SlotIndex);
		return;
	}

	// Get item data from DataTable
	FItemData* ItemData = ParentSearchWidget->GetItemData(SlotData->ItemID);
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWW_SearchResultEntryWidget::InitializeEntry - Failed to find ItemData for ItemID: %s"), *SlotData->ItemID.ToString());
		return;
	}

	// Set item name (DisplayName is FName, convert to text)
	if (Text_ItemName)
	{
		Text_ItemName->SetText(FText::FromName(ItemData->DisplayName));
	}

	// Set quantity
	if (Text_Quantity)
	{
		if (SlotData->Quantity > 1)
		{
			Text_Quantity->SetText(FText::AsNumber(SlotData->Quantity));
			Text_Quantity->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Text_Quantity->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Set item icon (async loading)
	if (Image_Icon && ItemData->IconTexture.ToSoftObjectPath().IsValid())
	{
		// Check if already loaded
		if (UTexture2D* LoadedIcon = ItemData->IconTexture.Get())
		{
			Image_Icon->SetBrushFromTexture(LoadedIcon);
		}
		else
		{
			// Async load the icon
			TSoftObjectPtr<UTexture2D> IconSoftPtr = ItemData->IconTexture;
			TWeakObjectPtr<UWW_SearchResultEntryWidget> WeakThis(this);
			
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			Streamable.RequestAsyncLoad(IconSoftPtr.ToSoftObjectPath(), 
				FStreamableDelegate::CreateLambda([WeakThis, IconSoftPtr]()
				{
					if (WeakThis.IsValid() && WeakThis->Image_Icon)
					{
						if (UTexture2D* LoadedIcon = IconSoftPtr.Get())
						{
							WeakThis->Image_Icon->SetBrushFromTexture(LoadedIcon);
						}
					}
				})
			);
		}
	}

	// Set rarity color border
	if (Image_RarityBorder)
	{
		FLinearColor RarityColor = GetRarityColor(ItemData);
		Image_RarityBorder->SetColorAndOpacity(RarityColor);
	}
}

void UWW_SearchResultEntryWidget::OnEntryClicked()
{
	if (ParentSearchWidget)
	{
		ParentSearchWidget->OnSearchResultClicked(SlotIndex);
	}
}

FLinearColor UWW_SearchResultEntryWidget::GetRarityColor(const FItemData* ItemData) const
{
	if (!ItemData)
	{
		return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White default
	}

	// Use the GetRarityColor() method from FItemData
	return ItemData->GetRarityColor();
}