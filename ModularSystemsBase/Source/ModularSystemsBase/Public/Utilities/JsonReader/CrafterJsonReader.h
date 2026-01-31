
#pragma once

#include "CoreMinimal.h"
#include "Windwalker_Productions_SharedDefaults.h"
#include "Utilities/JsonReader/JsonReaderBase.h"
#include "CrafterJsonReader.generated.h"

/**
 * JSON Reader for Crafting Crafters
 * 
 * File: Plugins/ModularInventorySystem/Content/Data/CrafterItems.json
 * DataTable: /ModularInventorySystem/Data/DT_CrafterItems
 * Row Struct: FCrafterData
 * JSON Array Key: "Crafters"
 * Row Name Field: "CrafterID"
 */
UCLASS()
class MODULARSYSTEMSBASE_API UCrafterJsonReader : public UJsonReaderBase
{
    GENERATED_BODY()

public:

    // ============================================================
    // PUBLIC API
    // ============================================================

    /**
     * Reload Crafters from JSON into DataTable (runtime)
     * @param OutDataTable - Optional: returns the populated DataTable
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "JsonReader|Crafters")
    static bool ReloadCrafters(UDataTable*& OutDataTable);

    /**
     * Reload Crafters and save DataTable asset (editor only)
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "JsonReader|Crafters", meta = (DevelopmentOnly))
    static bool ReloadCraftersInEditor();

    /**
     * Get the DataTable (loads if needed)
     * @return DataTable or nullptr
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Crafters")
    static UDataTable* GetCrafterDataTable();

    /**
     * Get the JSON file path
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Crafters")
    static FString GetCrafterJsonPath();

    /**
     * Get the DataTable asset path
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Crafters")
    static FString GetCrafterDataTablePath();

protected:
    // ============================================================
    // OVERRIDES
    // ============================================================

    virtual FString GetPluginName() const override { return WWPluginDirectories::PLUGIN_NAME_MODULARINVENTORYSYSTEM; }
    virtual FString GetJsonFileName() const override { return WWPluginDirectories::JSON_FILE_NAME_CRAFTERS; }
    virtual FString GetDataTableName() const override { return WWPluginDirectories::DATATABLE_NAME_CRAFTERS; }
    virtual UScriptStruct* GetRowStructType() const override;
    virtual FString GetJsonArrayKey() const override { return WWPluginDirectories::JSON_ARRAY_KEY_CRAFTERS; }
    virtual FString GetRowNameField() const override { return WWPluginDirectories::ROW_NAME_FIELD_CRAFTERID; }

private:
    /** Cached DataTable reference */
    static TWeakObjectPtr<UDataTable> CachedDataTable;
};