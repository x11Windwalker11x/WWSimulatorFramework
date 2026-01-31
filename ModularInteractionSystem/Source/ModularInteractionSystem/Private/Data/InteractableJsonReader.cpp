// InteractableJsonReader.cpp
// Location: ModularSystemsBase/Private/Utilities/JsonReader/InteractableJsonReader.cpp

#include "Utilities/JsonReader/InteractableJsonReader.h"
#include "Lib/Data/ModularInteractionSystem/InteractionData.h"

DEFINE_LOG_CATEGORY_STATIC(LogInteractableJsonReader, Log, All);

// Static member initialization
TWeakObjectPtr<UDataTable> UInteractableJsonReader::CachedDataTable = nullptr;

UScriptStruct* UInteractableJsonReader::GetRowStructType() const
{
    return FInteractableData::StaticStruct();
}

FString UInteractableJsonReader::GetInteractableJsonPath()
{
    return GetJsonFilePath(PLUGIN_NAME, JSON_FILE_NAME);
}

FString UInteractableJsonReader::GetInteractableDataTablePath()
{
    return GetDataTableAssetPath(PLUGIN_NAME, DATATABLE_NAME);
}

UDataTable* UInteractableJsonReader::GetInteractableDataTable()
{
    // Return cached if valid
    if (CachedDataTable.IsValid())
    {
        return CachedDataTable.Get();
    }
    
    // Load DataTable
    FString AssetPath = GetInteractableDataTablePath();
    UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *AssetPath);
    
    if (DataTable)
    {
        CachedDataTable = DataTable;
    }
    
    return DataTable;
}

bool UInteractableJsonReader::ReloadInteractables(UDataTable*& OutDataTable)
{
    // Get paths
    FString JsonPath = GetInteractableJsonPath();
    FString DataTablePath = GetInteractableDataTablePath();
    
    if (JsonPath.IsEmpty())
    {
        UE_LOG(LogInteractableJsonReader, Error, TEXT("Failed to get JSON path - is plugin '%s' loaded?"), PLUGIN_NAME);
        return false;
    }
    
    // Load JSON
    TSharedPtr<FJsonObject> JsonObject;
    if (!LoadJsonFile(JsonPath, JsonObject))
    {
        return false;
    }
    
    // Get Interactables array
    const TArray<TSharedPtr<FJsonValue>>* InteractablesArray;
    if (!JsonObject->TryGetArrayField(JSON_ARRAY_KEY, InteractablesArray))
    {
        UE_LOG(LogInteractableJsonReader, Error, TEXT("JSON missing '%s' array"), JSON_ARRAY_KEY);
        return false;
    }
    
    // Load or create DataTable
    UDataTable* DataTable = LoadOrCreateDataTable(DataTablePath, FInteractableData::StaticStruct());
    if (!DataTable)
    {
        return false;
    }
    
    // Populate from JSON
    bool bSuccess = PopulateDataTableFromJson(
        DataTable,
        *InteractablesArray,
        ROW_NAME_FIELD,
        TEXT("InteractableJsonReader")
    );
    
    if (bSuccess)
    {
        CachedDataTable = DataTable;
        OutDataTable = DataTable;
        UE_LOG(LogInteractableJsonReader, Log, TEXT("Successfully reloaded %d interactables"), DataTable->GetRowNames().Num());
    }
    
    return bSuccess;
}

bool UInteractableJsonReader::ReloadInteractablesInEditor()
{
#if WITH_EDITOR
    UDataTable* DataTable = nullptr;
    if (!ReloadInteractables(DataTable))
    {
        return false;
    }
    
    return SaveDataTableAsset(DataTable);
#else
    UE_LOG(LogInteractableJsonReader, Warning, TEXT("ReloadInteractablesInEditor called at runtime"));
    return false;
#endif
}