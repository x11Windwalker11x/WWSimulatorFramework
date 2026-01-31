// ItemJsonReader.h
// Location: ModularSystemsBase/Public/Utilities/JsonReader/ItemJsonReader.h

#pragma once

#include "CoreMinimal.h"
#include "Windwalker_Productions_SharedDefaults.h"
#include "Utilities/JsonReader/JsonReaderBase.h"
#include "ItemJsonReader.generated.h"

/**
 * JSON Reader for Inventory Items
 * 
 * File: Plugins/ModularInventorySystem/Content/Data/InventoryItems.json
 * DataTable: /ModularInventorySystem/Data/DT_InventoryItems
 * Row Struct: FItemData
 * JSON Array Key: "Items"
 * Row Name Field: "ItemID"
 */
UCLASS()
class MODULARSYSTEMSBASE_API UItemJsonReader : public UJsonReaderBase
{
    GENERATED_BODY()

public:

    // ============================================================
    // PUBLIC API
    // ============================================================

    /**
     * Reload items from JSON into DataTable (runtime)
     * @param OutDataTable - Optional: returns the populated DataTable
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "JsonReader|Items")
    static bool ReloadItems(UDataTable*& OutDataTable);

    /**
     * Reload items and save DataTable asset (editor only)
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "JsonReader|Items", meta = (DevelopmentOnly))
    static bool ReloadItemsInEditor();

    /**
     * Get the DataTable (loads if needed)
     * @return DataTable or nullptr
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Items")
    static UDataTable* GetItemDataTable();

    /**
     * Get the JSON file path
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Items")
    static FString GetItemJsonPath();

    /**
     * Get the DataTable asset path
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Items")
    static FString GetItemDataTablePath();

protected:
    // ============================================================
    // OVERRIDES
    // ============================================================

    virtual FString GetPluginName() const override { return WWPluginDirectories::PLUGIN_NAME_MODULARINVENTORYSYSTEM; }
    virtual FString GetJsonFileName() const override { return WWPluginDirectories::JSON_FILE_NAME_INVENTORYITEMS; }
    virtual FString GetDataTableName() const override { return WWPluginDirectories::DATATABLE_NAME_INVENTORYITEMS; }
    virtual UScriptStruct* GetRowStructType() const override;
    virtual FString GetJsonArrayKey() const override { return WWPluginDirectories::JSON_ARRAY_KEY_ITEMS; }
    virtual FString GetRowNameField() const override { return WWPluginDirectories::ROW_NAME_FIELD_ITEMID; }


private:
    /** Cached DataTable reference */
    static TWeakObjectPtr<UDataTable> CachedDataTable;
};