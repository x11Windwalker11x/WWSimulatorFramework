#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTags.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "ItemHelpers.generated.h"

class UDataTable;

/**
 * Static utilities for item operations.
 */
UCLASS()
class MODULARSYSTEMSBASE_API UItemHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // === DATA LOOKUP ===
    
    /** Get item data from DataTable by ID */
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static bool GetItemData(UDataTable* ItemTable, FName ItemID, FItemData& OutData);
    
    /** Check if item exists in table */
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static bool DoesItemExist(UDataTable* ItemTable, FName ItemID);
    
    /** Get all item IDs from table */
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static TArray<FName> GetAllItemIDs(UDataTable* ItemTable);
    
    /** Filter items by tag */
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static TArray<FName> FilterItemsByTag(UDataTable* ItemTable, FGameplayTag FilterTag);

    // === SLOT OPERATIONS ===
    
    /** Check if slot can accept item (stack compatibility) */
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static bool CanStackItems(const FInventorySlot& ExistingSlot, FName NewItemID, int32 Quantity, UDataTable* ItemTable);
    
    /** Calculate how many items can fit in slot */
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static int32 GetRemainingStackSpace(const FInventorySlot& Slot, UDataTable* ItemTable);
    
    /** Create new slot from item data */
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static FInventorySlot CreateSlotFromItem(FName ItemID, int32 Quantity);

    // === VALUE CALCULATIONS ===
    
    /** Calculate total value of slot (BaseValue * Quantity * QualityModifier) */
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static int32 CalculateSlotValue(const FInventorySlot& Slot, UDataTable* ItemTable);
    
    /** Calculate total weight of slot */
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static float CalculateSlotWeight(const FInventorySlot& Slot, UDataTable* ItemTable);
    
    //Value Per Item Calculations
    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static int32 GetItemValue(const FName& ItemID, UDataTable* ItemTable);

    UFUNCTION(BlueprintPure, Category = "Item Helpers")
    static float GetItemWeight(const FName& ItemID, UDataTable* ItemTable);


};