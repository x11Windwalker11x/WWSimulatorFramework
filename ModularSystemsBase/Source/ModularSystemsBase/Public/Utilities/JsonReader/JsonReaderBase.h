// JsonReaderBase.h
// Location: ModularSystemsBase/Public/Utilities/JsonReader/JsonReaderBase.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "JsonReaderBase.generated.h"

/**
 * Base class for JSON to DataTable conversion
 * 
 * Architecture: Each plugin owns its data
 * - JSON files live in: Plugins/[PluginName]/Content/Data/[FileName].json
 * - DataTables live in: /[PluginName]/Data/DT_[Name]
 * 
 * Derived classes define:
 * - GetPluginName() - which plugin owns this data
 * - GetJsonFileName() - the .json file name
 * - GetDataTableName() - the DataTable asset name (without DT_ prefix)
 * - GetRowStructType() - the UScriptStruct for the row type
 * - GetJsonArrayKey() - the key in JSON that holds the array ("Items", "Recipes", etc.)
 * - GetRowNameField() - the field in JSON used as row name ("ItemID", "RecipeID", etc.)
 */
UCLASS(Abstract)
class MODULARSYSTEMSBASE_API UJsonReaderBase : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ============================================================
    // STATIC UTILITY FUNCTIONS (Used by all derived classes)
    // ============================================================

    /**
     * Get the full filesystem path for a JSON file in a plugin
     * @param PluginName - Plugin that owns the data (e.g., "ModularInventorySystem")
     * @param FileName - JSON file name (e.g., "InventoryItems.json")
     * @return Full filesystem path to the JSON file
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader")
    static FString GetJsonFilePath(const FString& PluginName, const FString& FileName);

    /**
     * Get the UE asset path for a DataTable in a plugin
     * @param PluginName - Plugin that owns the data
     * @param DataTableName - Name without DT_ prefix (e.g., "InventoryItems" -> "/Plugin/Data/DT_InventoryItems")
     * @return UE asset path (e.g., "/ModularInventorySystem/Data/DT_InventoryItems")
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader")
    static FString GetDataTableAssetPath(const FString& PluginName, const FString& DataTableName);

    /**
     * Load and parse a JSON file
     * @param FilePath - Full filesystem path to JSON file
     * @param OutJsonObject - Parsed JSON object
     * @return true if successful
     */
    static bool LoadJsonFile(const FString& FilePath, TSharedPtr<FJsonObject>& OutJsonObject);

    /**
     * Load or create a DataTable asset
     * @param AssetPath - UE asset path
     * @param RowStruct - The struct type for rows
     * @return Loaded or newly created DataTable (nullptr on failure)
     */
    static UDataTable* LoadOrCreateDataTable(const FString& AssetPath, UScriptStruct* RowStruct);

    /**
     * Populate a DataTable from a JSON array
     * @param DataTable - Target DataTable
     * @param JsonArray - Array of JSON objects
     * @param RowNameField - Field in JSON to use as row name (e.g., "ItemID")
     * @param ContextString - For error logging
     * @return true if successful
     */
    static bool PopulateDataTableFromJson(
        UDataTable* DataTable,
        const TArray<TSharedPtr<FJsonValue>>& JsonArray,
        const FString& RowNameField,
        const FString& ContextString
    );

    /**
     * Save a DataTable asset to disk (Editor only)
     * @param DataTable - DataTable to save
     * @return true if successful
     */
    static bool SaveDataTableAsset(UDataTable* DataTable);

protected:
    // ============================================================
    // VIRTUAL FUNCTIONS (Override in derived classes)
    // ============================================================

    /** Plugin that owns this data type */
    virtual FString GetPluginName() const PURE_VIRTUAL(UJsonReaderBase::GetPluginName, return TEXT(""););

    /** JSON file name (e.g., "InventoryItems.json") */
    virtual FString GetJsonFileName() const PURE_VIRTUAL(UJsonReaderBase::GetJsonFileName, return TEXT(""););

    /** DataTable name without DT_ prefix */
    virtual FString GetDataTableName() const PURE_VIRTUAL(UJsonReaderBase::GetDataTableName, return TEXT(""););

    /** The UScriptStruct for this DataTable's rows */
    virtual UScriptStruct* GetRowStructType() const PURE_VIRTUAL(UJsonReaderBase::GetRowStructType, return nullptr;);

    /** Key in JSON that holds the data array (e.g., "Items", "Recipes") */
    virtual FString GetJsonArrayKey() const PURE_VIRTUAL(UJsonReaderBase::GetJsonArrayKey, return TEXT(""););

    /** Field in JSON objects used as row name (e.g., "ItemID", "RecipeID") */
    virtual FString GetRowNameField() const PURE_VIRTUAL(UJsonReaderBase::GetRowNameField, return TEXT(""););

    // ============================================================
    // HELPER FUNCTIONS
    // ============================================================

    /** Check if plugin exists and is loaded */
    static bool IsPluginLoaded(const FString& PluginName);

    /** Get plugin content directory (filesystem path) */
    static FString GetPluginContentDir(const FString& PluginName);
};