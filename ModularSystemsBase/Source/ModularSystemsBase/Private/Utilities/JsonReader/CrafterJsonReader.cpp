// CrafterJsonReader.cpp
// Location: ModularSystemsBase/Private/Utilities/JsonReader/CrafterJsonReader.cpp

#include "Utilities/JsonReader/CrafterJsonReader.h"
#include "Lib/Data/ModularCraftingData/CraftingData.h"

DEFINE_LOG_CATEGORY_STATIC(LogCrafterJsonReader, Log, All);

// Static member initialization
TWeakObjectPtr<UDataTable> UCrafterJsonReader::CachedDataTable = nullptr;

UScriptStruct* UCrafterJsonReader::GetRowStructType() const
{
    return FCrafterData::StaticStruct();
}

FString UCrafterJsonReader::GetCrafterJsonPath()
{
    return GetJsonFilePath(WWPluginDirectories::PLUGIN_NAME_MODULARINVENTORYSYSTEM, WWPluginDirectories::JSON_FILE_NAME_CRAFTER);
}

FString UCrafterJsonReader::GetCrafterDataTablePath()
{
    return GetDataTableAssetPath(WWPluginDirectories::PLUGIN_NAME_MODULARINVENTORYSYSTEM, WWPluginDirectories::DATATABLE_NAME_CRAFTER);
}

UDataTable* UCrafterJsonReader::GetCrafterDataTable()
{
    // Return cached if valid
    if (CachedDataTable.IsValid())
    {
        return CachedDataTable.Get();
    }
    
    // Load DataTable
    FString AssetPath = GetCrafterDataTablePath();
    UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *AssetPath);
    
    if (DataTable)
    {
        CachedDataTable = DataTable;
    }
    
    return DataTable;
}

bool UCrafterJsonReader::ReloadCrafters(UDataTable*& OutDataTable)
{
    // Get paths
    FString JsonPath = GetCrafterJsonPath();
    FString DataTablePath = GetCrafterDataTablePath();
    
    if (JsonPath.IsEmpty())
    {
        UE_LOG(LogCrafterJsonReader, Error, TEXT("Failed to get JSON path - is plugin '%s' loaded?"), WWPluginDirectories::PLUGIN_NAME_MODULARINVENTORYSYSTEM);
        return false;
    }
    
    // Load JSON
    TSharedPtr<FJsonObject> JsonObject;
    if (!LoadJsonFile(JsonPath, JsonObject))
    {
        return false;
    }
    
    // Get Crafters array
    const TArray<TSharedPtr<FJsonValue>>* CraftersArray;
    if (!JsonObject->TryGetArrayField(WWPluginDirectories::JSON_ARRAY_KEY_CRAFTERS, CraftersArray))
    {
        UE_LOG(LogCrafterJsonReader, Error, TEXT("JSON missing '%s' array"), WWPluginDirectories::JSON_ARRAY_KEY_CRAFTERS);
        return false;
    }
    
    // Load or create DataTable
    UDataTable* DataTable = LoadOrCreateDataTable(DataTablePath, FCrafterData::StaticStruct());
    if (!DataTable)
    {
        return false;
    }
    
    // Populate from JSON
    bool bSuccess = PopulateDataTableFromJson(
        DataTable,
        *CraftersArray,
        WWPluginDirectories::ROW_NAME_FIELD_CRAFTERID,
        TEXT("CrafterJsonReader")
    );
    
    if (bSuccess)
    {
        CachedDataTable = DataTable;
        OutDataTable = DataTable;
        UE_LOG(LogCrafterJsonReader, Log, TEXT("Successfully reloaded %d Crafters"), DataTable->GetRowNames().Num());
    }
    
    return bSuccess;
}

bool UCrafterJsonReader::ReloadCraftersInEditor()
{
#if WITH_EDITOR
    UDataTable* DataTable = nullptr;
    if (!ReloadCrafters(DataTable))
    {
        return false;
    }
    
    return SaveDataTableAsset(DataTable);
#else
    UE_LOG(LogCrafterJsonReader, Warning, TEXT("ReloadCraftersInEditor called at runtime"));
    return false;
#endif
}