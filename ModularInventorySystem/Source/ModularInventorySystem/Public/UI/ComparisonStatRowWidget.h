// ComparisonStatRowWidget.h
// Single row widget displaying one stat comparison between two items

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "ComparisonStatRowWidget.generated.h"

class UTextBlock;
class UImage;

/**
 * UComparisonStatRowWidget
 * Displays a single stat comparison row (e.g., "Damage: 50 vs 75 (+25)")
 * Uses FStatComparisonEntry as data contract
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API UComparisonStatRowWidget : public UManagedWidget_Master
{
    GENERATED_BODY()

public:
    /**
     * Initialize the row with comparison data
     * @param ComparisonEntry - The stat comparison data to display
     */
    UFUNCTION(BlueprintCallable, Category = "Comparison")
    void SetComparisonData(const FStatComparisonEntry& ComparisonEntry);

protected:
    virtual void NativeConstruct() override;

    // ============================================================================
    // BOUND WIDGETS
    // ============================================================================

    /** Stat name display (e.g., "Damage", "Accuracy") */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_StatName;

    /** Current item's value */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_CurrentValue;

    /** Compared item's value */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_CompareValue;

    /** Difference display (+X or -X) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Difference;

    /** Optional arrow indicator */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_Arrow;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    /** Color for positive comparisons (better stats) */
    UPROPERTY(EditDefaultsOnly, Category = "Comparison|Colors")
    FLinearColor BetterColor = FLinearColor::Green;

    /** Color for negative comparisons (worse stats) */
    UPROPERTY(EditDefaultsOnly, Category = "Comparison|Colors")
    FLinearColor WorseColor = FLinearColor::Red;

    /** Color for equal/neutral comparisons */
    UPROPERTY(EditDefaultsOnly, Category = "Comparison|Colors")
    FLinearColor NeutralColor = FLinearColor::White;

private:
    /**
     * Update visual state based on comparison result
     * @param Entry - The comparison entry to derive visuals from
     */
    void UpdateVisuals(const FStatComparisonEntry& Entry);
};
