// ItemPreviewWidget_Base.h
// Base widget for item preview/tooltip display - extend in MIS for item-specific variants

#pragma once

#include "CoreMinimal.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "Engine/StreamableManager.h"
#include "ItemPreviewWidget_Base.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;
class UVerticalBox;

/**
 * UItemPreviewWidget_Base
 * Abstract base class for item preview/tooltip widgets
 * Displays FItemPreviewData - extend in MIS for type-specific layouts
 */
UCLASS(Abstract)
class MODULARINVENTORYSYSTEM_API UItemPreviewWidget_Base : public UManagedWidget_Master
{
    GENERATED_BODY()

public:
    /**
     * Set the item data to display
     * @param PreviewData - The item preview data to display
     */
    UFUNCTION(BlueprintCallable, Category = "Item Preview")
    virtual void SetItemData(const FItemPreviewData& PreviewData);

    /**
     * Clear the preview display
     */
    UFUNCTION(BlueprintCallable, Category = "Item Preview")
    virtual void ClearPreview();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ============================================================================
    // BOUND WIDGETS (Optional - child classes define layout)
    // ============================================================================

    /** Item icon */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_Icon;

    /** Item display name */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Name;

    /** Item description */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Description;

    /** Rarity text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Rarity;

    /** Item type text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_ItemType;

    /** Stack quantity */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Quantity;

    /** Item value */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Value;

    /** Quality percentage */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Quality;

    /** Container for stat rows (child classes populate) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> Box_Stats;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    /** Default icon when loading */
    UPROPERTY(EditDefaultsOnly, Category = "Item Preview|Icons")
    TObjectPtr<UTexture2D> DefaultIcon;

    /** Rarity colors by tag name */
    UPROPERTY(EditDefaultsOnly, Category = "Item Preview|Colors")
    TMap<FName, FLinearColor> RarityColors;

    // ============================================================================
    // BLUEPRINT EVENTS
    // ============================================================================

    /** Called after base data is set - override for type-specific display */
    UFUNCTION(BlueprintNativeEvent, Category = "Item Preview")
    void OnItemDataSet(const FItemPreviewData& PreviewData);

    /** Called to populate stats box - override in child classes */
    UFUNCTION(BlueprintNativeEvent, Category = "Item Preview")
    void PopulateStats(const TMap<FName, float>& Stats);

    // ============================================================================
    // HELPERS
    // ============================================================================

    /** Get color for rarity tag */
    UFUNCTION(BlueprintPure, Category = "Item Preview")
    FLinearColor GetRarityColor(const FGameplayTag& RarityTag) const;

    /** Format stat value for display */
    UFUNCTION(BlueprintPure, Category = "Item Preview")
    static FText FormatStatValue(float Value, const FString& Suffix = TEXT(""));

private:
    /** Handle for async icon loading */
    TSharedPtr<FStreamableHandle> IconLoadHandle;

    /** Callback when icon loads */
    void OnIconLoaded();

    /** Current preview data (cached for async callbacks) */
    FItemPreviewData CachedPreviewData;
};
