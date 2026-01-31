// RecipeJsonReader.cpp
// Location: ModularSystemsBase/Private/Utilities/JsonReader/RecipeJsonReader.cpp

#include "Utilities/JsonReader/RecipeJsonReader.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"

DEFINE_LOG_CATEGORY_STATIC(LogRecipeJsonReader, Log, All);

// Static member initialization
TWeakObjectPtr<UDataTable> URecipeJsonReader::CachedDataTable = nullptr;

UScriptStruct* URecipeJsonReader::GetRowStructType() const
{
    return FRecipeData::StaticStruct();
}

FString URecipeJsonReader::GetRecipeJsonPath()
{
    return GetJsonFilePath(PLUGIN_NAME, JSON_FILE_NAME);
}

FString URecipeJsonReader::GetRecipeDataTablePath()
{
    return GetDataTableAssetPath(PLUGIN_NAME, DATATABLE_NAME);
}

UDataTable* URecipeJsonReader::GetRecipeDataTable()
{
    // Return cached if valid
    if (CachedDataTable.IsValid())
    {
        return CachedDataTable.Get();
    }
    
    // Load DataTable
    FString AssetPath = GetRecipeDataTablePath();
    UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *AssetPath);
    
    if (DataTable)
    {
        CachedDataTable = DataTable;
    }
    
    return DataTable;
}

bool URecipeJsonReader::ReloadRecipes(UDataTable*& OutDataTable)
{
    // Get paths
    FString JsonPath = GetRecipeJsonPath();
    FString DataTablePath = GetRecipeDataTablePath();
    
    if (JsonPath.IsEmpty())
    {
        UE_LOG(LogRecipeJsonReader, Error, TEXT("Failed to get JSON path - is plugin '%s' loaded?"), PLUGIN_NAME);
        return false;
    }
    
    // Load JSON
    TSharedPtr<FJsonObject> JsonObject;
    if (!LoadJsonFile(JsonPath, JsonObject))
    {
        return false;
    }
    
    // Get Recipes array
    const TArray<TSharedPtr<FJsonValue>>* RecipesArray;
    if (!JsonObject->TryGetArrayField(JSON_ARRAY_KEY, RecipesArray))
    {
        UE_LOG(LogRecipeJsonReader, Error, TEXT("JSON missing '%s' array"), JSON_ARRAY_KEY);
        return false;
    }
    
    // Load or create DataTable
    UDataTable* DataTable = LoadOrCreateDataTable(DataTablePath, FRecipeData::StaticStruct());
    if (!DataTable)
    {
        return false;
    }
    
    // Populate from JSON
    bool bSuccess = PopulateDataTableFromJson(
        DataTable,
        *RecipesArray,
        ROW_NAME_FIELD,
        TEXT("RecipeJsonReader")
    );
    
    if (bSuccess)
    {
        CachedDataTable = DataTable;
        OutDataTable = DataTable;
        UE_LOG(LogRecipeJsonReader, Log, TEXT("Successfully reloaded %d recipes"), DataTable->GetRowNames().Num());
    }
    
    return bSuccess;
}

bool URecipeJsonReader::ReloadRecipesInEditor()
{
#if WITH_EDITOR
    UDataTable* DataTable = nullptr;
    if (!ReloadRecipes(DataTable))
    {
        return false;
    }
    
    return SaveDataTableAsset(DataTable);
#else
    UE_LOG(LogRecipeJsonReader, Warning, TEXT("ReloadRecipesInEditor called at runtime"));
    return false;
#endif
}