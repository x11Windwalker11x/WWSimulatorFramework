// RecipeJsonReader.h
// Location: ModularSystemsBase/Public/Utilities/JsonReader/RecipeJsonReader.h

#pragma once

#include "CoreMinimal.h"
#include "Windwalker_Productions_SharedDefaults.h"
#include "Utilities/JsonReader/JsonReaderBase.h"
#include "RecipeJsonReader.generated.h"

/**
 * JSON Reader for Crafting Recipes
 * 
 * File: Plugins/ModularInventorySystem/Content/Data/RecipeItems.json
 * DataTable: /ModularInventorySystem/Data/DT_RecipeItems
 * Row Struct: FRecipeData
 * JSON Array Key: "Recipes"
 * Row Name Field: "RecipeID"
 */
UCLASS()
class MODULARSYSTEMSBASE_API URecipeJsonReader : public UJsonReaderBase
{
    GENERATED_BODY()

public:

    // ============================================================
    // PUBLIC API
    // ============================================================

    /**
     * Reload recipes from JSON into DataTable (runtime)
     * @param OutDataTable - Optional: returns the populated DataTable
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "JsonReader|Recipes")
    static bool ReloadRecipes(UDataTable*& OutDataTable);

    /**
     * Reload recipes and save DataTable asset (editor only)
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "JsonReader|Recipes", meta = (DevelopmentOnly))
    static bool ReloadRecipesInEditor();

    /**
     * Get the DataTable (loads if needed)
     * @return DataTable or nullptr
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Recipes")
    static UDataTable* GetRecipeDataTable();

    /**
     * Get the JSON file path
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Recipes")
    static FString GetRecipeJsonPath();

    /**
     * Get the DataTable asset path
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Recipes")
    static FString GetRecipeDataTablePath();

protected:
    // ============================================================
    // OVERRIDES
    // ============================================================

    virtual FString GetPluginName() const override { return WWPluginDirectories::PLUGIN_NAME_MODULARINVENTORYSYSTEM; }
    virtual FString GetJsonFileName() const override { return WWPluginDirectories::JSON_FILE_NAME_RECIPEITEMS; }
    virtual FString GetDataTableName() const override { return WWPluginDirectories::DATATABLE_NAME_RECIPEITEMS; }
    virtual UScriptStruct* GetRowStructType() const override;
    virtual FString GetJsonArrayKey() const override { return WWPluginDirectories::JSON_ARRAY_KEY_RECIPES; }
    virtual FString GetRowNameField() const override { return WWPluginDirectories::ROW_NAME_FIELD_RECIPE; }

private:
    /** Cached DataTable reference */
    static TWeakObjectPtr<UDataTable> CachedDataTable;
};