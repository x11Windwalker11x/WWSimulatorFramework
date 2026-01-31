// ItemJsonReader.cpp
// Location: ModularSystemsBase/Private/Utilities/JsonReader/ItemJsonReader.cpp

#include "Utilities/JsonReader/ItemJsonReader.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"

DEFINE_LOG_CATEGORY_STATIC(LogItemJsonReader, Log, All);

// Static member initialization
TWeakObjectPtr<UDataTable> UItemJsonReader::CachedDataTable = nullptr;

UScriptStruct* UItemJsonReader::GetRowStructType() const
{
    return FItemData::StaticStruct();
}

FString UItemJsonReader::GetItemJsonPath()
{
    return GetJsonFilePath(PLUGIN_NAME, JSON_FILE_NAME);
}

FString UItemJsonReader::GetItemDataTablePath()
{
    return GetDataTableAssetPath(PLUGIN_NAME, DATATABLE_NAME);
}

UDataTable* UItemJsonReader::GetItemDataTable()
{
    // Return cached if valid
    if (CachedDataTable.IsValid())
    {
        return CachedDataTable.Get();
    }
    
    // Load DataTable
    FString AssetPath = GetItemDataTablePath();
    UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *AssetPath);
    
    if (DataTable)
    {
        CachedDataTable = DataTable;
    }
    
    return DataTable;
}

bool UItemJsonReader::ReloadItems(UDataTable*& OutDataTable)
{
    // Get paths
    FString JsonPath = GetItemJsonPath();
    FString DataTablePath = GetItemDataTablePath();
    
    if (JsonPath.IsEmpty())
    {
        UE_LOG(LogItemJsonReader, Error, TEXT("Failed to get JSON path - is plugin '%s' loaded?"), PLUGIN_NAME);
        return false;
    }
    
    // Load JSON
    TSharedPtr<FJsonObject> JsonObject;
    if (!LoadJsonFile(JsonPath, JsonObject))
    {
        return false;
    }
    
    // Get Items array
    const TArray<TSharedPtr<FJsonValue>>* ItemsArray;
    if (!JsonObject->TryGetArrayField(JSON_ARRAY_KEY, ItemsArray))
    {
        UE_LOG(LogItemJsonReader, Error, TEXT("JSON missing '%s' array"), JSON_ARRAY_KEY);
        return false;
    }
    
    // Load or create DataTable
    UDataTable* DataTable = LoadOrCreateDataTable(DataTablePath, FItemData::StaticStruct());
    if (!DataTable)
    {
        return false;
    }
    
    // Populate from JSON
    bool bSuccess = PopulateDataTableFromJson(
        DataTable,
        *ItemsArray,
        ROW_NAME_FIELD,
        TEXT("ItemJsonReader")
    );
    
    if (bSuccess)
    {
        CachedDataTable = DataTable;
        OutDataTable = DataTable;
        UE_LOG(LogItemJsonReader, Log, TEXT("Successfully reloaded %d items"), DataTable->GetRowNames().Num());
    }
    
    return bSuccess;
}

bool UItemJsonReader::ReloadItemsInEditor()
{
#if WITH_EDITOR
    UDataTable* DataTable = nullptr;
    if (!ReloadItems(DataTable))
    {
        return false;
    }
    
    return SaveDataTableAsset(DataTable);
#else
    UE_LOG(LogItemJsonReader, Warning, TEXT("ReloadItemsInEditor called at runtime"));
    return false;
#endif
}