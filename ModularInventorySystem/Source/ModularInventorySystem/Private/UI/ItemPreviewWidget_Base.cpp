// ItemPreviewWidget_Base.cpp
// Base widget for item preview/tooltip display

#include "UI/ItemPreviewWidget_Base.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Engine/Texture2D.h"

void UItemPreviewWidget_Base::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize default rarity colors if not set
    if (RarityColors.Num() == 0)
    {
        RarityColors.Add(FName("Legendary"), FLinearColor(1.0f, 0.5f, 0.0f));  // Orange
        RarityColors.Add(FName("Epic"), FLinearColor(0.5f, 0.0f, 1.0f));       // Purple
        RarityColors.Add(FName("Rare"), FLinearColor(0.0f, 0.4f, 1.0f));       // Blue
        RarityColors.Add(FName("Uncommon"), FLinearColor(0.0f, 0.8f, 0.0f));   // Green
        RarityColors.Add(FName("Common"), FLinearColor(0.5f, 0.5f, 0.5f));     // Gray
    }
}

void UItemPreviewWidget_Base::NativeDestruct()
{
    // Cancel any pending icon load
    if (IconLoadHandle.IsValid())
    {
        IconLoadHandle->CancelHandle();
        IconLoadHandle.Reset();
    }

    Super::NativeDestruct();
}

void UItemPreviewWidget_Base::SetItemData(const FItemPreviewData& PreviewData)
{
    CachedPreviewData = PreviewData;

    // Item name
    if (Text_Name)
    {
        Text_Name->SetText(PreviewData.DisplayName);
    }

    // Description
    if (Text_Description)
    {
        Text_Description->SetText(PreviewData.Description);
    }

    // Rarity
    if (Text_Rarity && PreviewData.Rarity.IsValid())
    {
        FString RarityStr = PreviewData.Rarity.GetTagName().ToString();
        // Extract last part of tag (e.g., "Item.Rarity.Legendary" -> "Legendary")
        int32 LastDot;
        if (RarityStr.FindLastChar('.', LastDot))
        {
            RarityStr = RarityStr.RightChop(LastDot + 1);
        }
        Text_Rarity->SetText(FText::FromString(RarityStr));
        Text_Rarity->SetColorAndOpacity(FSlateColor(GetRarityColor(PreviewData.Rarity)));
    }

    // Item type
    if (Text_ItemType && PreviewData.ItemType.IsValid())
    {
        FString TypeStr = PreviewData.ItemType.GetTagName().ToString();
        int32 LastDot;
        if (TypeStr.FindLastChar('.', LastDot))
        {
            TypeStr = TypeStr.RightChop(LastDot + 1);
        }
        Text_ItemType->SetText(FText::FromString(TypeStr));
    }

    // Quantity
    if (Text_Quantity)
    {
        if (PreviewData.Quantity > 1)
        {
            Text_Quantity->SetText(FText::FromString(FString::Printf(TEXT("x%d"), PreviewData.Quantity)));
            Text_Quantity->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            Text_Quantity->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // Value
    if (Text_Value)
    {
        Text_Value->SetText(FText::FromString(FString::Printf(TEXT("$%d"), PreviewData.Value)));
    }

    // Quality
    if (Text_Quality)
    {
        Text_Quality->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), PreviewData.CurrentQuality * 100.0f)));
    }

    // Icon (async load)
    if (Image_Icon)
    {
        if (PreviewData.Icon.IsNull())
        {
            // No icon specified - use default
            if (DefaultIcon)
            {
                Image_Icon->SetBrushFromTexture(DefaultIcon);
            }
        }
        else if (PreviewData.Icon.IsValid())
        {
            // Already loaded
            Image_Icon->SetBrushFromTexture(PreviewData.Icon.Get());
        }
        else
        {
            // Async load
            if (DefaultIcon)
            {
                Image_Icon->SetBrushFromTexture(DefaultIcon);
            }

            FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
            IconLoadHandle = StreamableManager.RequestAsyncLoad(
                PreviewData.Icon.ToSoftObjectPath(),
                FStreamableDelegate::CreateUObject(this, &UItemPreviewWidget_Base::OnIconLoaded)
            );
        }
    }

    // Populate stats
    PopulateStats(PreviewData.Stats);

    // Notify child classes
    OnItemDataSet(PreviewData);
}

void UItemPreviewWidget_Base::ClearPreview()
{
    if (Text_Name) Text_Name->SetText(FText::GetEmpty());
    if (Text_Description) Text_Description->SetText(FText::GetEmpty());
    if (Text_Rarity) Text_Rarity->SetText(FText::GetEmpty());
    if (Text_ItemType) Text_ItemType->SetText(FText::GetEmpty());
    if (Text_Quantity) Text_Quantity->SetVisibility(ESlateVisibility::Collapsed);
    if (Text_Value) Text_Value->SetText(FText::GetEmpty());
    if (Text_Quality) Text_Quality->SetText(FText::GetEmpty());

    if (Image_Icon && DefaultIcon)
    {
        Image_Icon->SetBrushFromTexture(DefaultIcon);
    }

    if (Box_Stats)
    {
        Box_Stats->ClearChildren();
    }

    CachedPreviewData = FItemPreviewData();
}

void UItemPreviewWidget_Base::OnIconLoaded()
{
    if (Image_Icon && CachedPreviewData.Icon.IsValid())
    {
        Image_Icon->SetBrushFromTexture(CachedPreviewData.Icon.Get());
    }
    IconLoadHandle.Reset();
}

void UItemPreviewWidget_Base::OnItemDataSet_Implementation(const FItemPreviewData& PreviewData)
{
    // Default implementation - child classes override for type-specific behavior
}

void UItemPreviewWidget_Base::PopulateStats_Implementation(const TMap<FName, float>& Stats)
{
    // Default implementation - child classes override to add stat rows
    // Example: Create UComparisonStatRowWidget instances and add to Box_Stats
}

FLinearColor UItemPreviewWidget_Base::GetRarityColor(const FGameplayTag& RarityTag) const
{
    if (!RarityTag.IsValid())
    {
        return FLinearColor::White;
    }

    // Extract rarity name from tag
    FString TagStr = RarityTag.GetTagName().ToString();
    int32 LastDot;
    if (TagStr.FindLastChar('.', LastDot))
    {
        TagStr = TagStr.RightChop(LastDot + 1);
    }

    const FLinearColor* FoundColor = RarityColors.Find(FName(*TagStr));
    return FoundColor ? *FoundColor : FLinearColor::White;
}

FText UItemPreviewWidget_Base::FormatStatValue(float Value, const FString& Suffix)
{
    // Format with appropriate precision
    FString FormattedStr;
    if (FMath::IsNearlyEqual(FMath::Frac(Value), 0.0f))
    {
        // Whole number
        FormattedStr = FString::Printf(TEXT("%.0f%s"), Value, *Suffix);
    }
    else
    {
        // Decimal
        FormattedStr = FString::Printf(TEXT("%.1f%s"), Value, *Suffix);
    }
    return FText::FromString(FormattedStr);
}
