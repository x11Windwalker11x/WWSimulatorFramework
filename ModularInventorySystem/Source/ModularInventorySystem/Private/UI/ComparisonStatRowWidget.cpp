// ComparisonStatRowWidget.cpp
// Single row widget displaying one stat comparison between two items

#include "UI/ComparisonStatRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UComparisonStatRowWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UComparisonStatRowWidget::SetComparisonData(const FStatComparisonEntry& ComparisonEntry)
{
    // Stat name
    if (Text_StatName)
    {
        Text_StatName->SetText(ComparisonEntry.StatDisplayName);
    }

    // Current value with suffix
    if (Text_CurrentValue)
    {
        FString CurrentStr = FString::Printf(TEXT("%.1f%s"),
            ComparisonEntry.CurrentValue, *ComparisonEntry.UnitSuffix);
        Text_CurrentValue->SetText(FText::FromString(CurrentStr));
    }

    // Compare value with suffix
    if (Text_CompareValue)
    {
        FString CompareStr = FString::Printf(TEXT("%.1f%s"),
            ComparisonEntry.CompareValue, *ComparisonEntry.UnitSuffix);
        Text_CompareValue->SetText(FText::FromString(CompareStr));
    }

    // Difference with +/- prefix
    if (Text_Difference)
    {
        float Diff = ComparisonEntry.GetDifference();
        FString DiffStr = FString::Printf(TEXT("%s%.1f%s"),
            Diff >= 0 ? TEXT("+") : TEXT(""),
            Diff, *ComparisonEntry.UnitSuffix);
        Text_Difference->SetText(FText::FromString(DiffStr));
    }

    UpdateVisuals(ComparisonEntry);
}

void UComparisonStatRowWidget::UpdateVisuals(const FStatComparisonEntry& Entry)
{
    FLinearColor ResultColor = NeutralColor;

    if (Entry.IsBetter())
    {
        ResultColor = BetterColor;
    }
    else if (Entry.IsWorse())
    {
        ResultColor = WorseColor;
    }

    // Apply color to difference text
    if (Text_Difference)
    {
        Text_Difference->SetColorAndOpacity(FSlateColor(ResultColor));
    }

    // Apply color to arrow if present
    if (Image_Arrow)
    {
        Image_Arrow->SetColorAndOpacity(ResultColor);
    }
}
