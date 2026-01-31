// JsonReaderBase.cpp
// Location: ModularSystemsBase/Private/Utilities/JsonReader/JsonReaderBase.cpp

#include "Utilities/JsonReader/JsonReaderBase.h"

#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IPluginManager.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "UObject/Package.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogJsonReader, Log, All);

// ============================================================
// STATIC UTILITY FUNCTIONS
// ============================================================

bool UJsonReaderBase::IsPluginLoaded(const FString& PluginName)
{
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
    return Plugin.IsValid() && Plugin->IsEnabled();
}

FString UJsonReaderBase::GetPluginContentDir(const FString& PluginName)
{
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
    if (!Plugin.IsValid())
    {
        UE_LOG(LogJsonReader, Error, TEXT("Plugin '%s' not found!"), *PluginName);
        return TEXT("");
    }
    
    return Plugin->GetContentDir();
}

FString UJsonReaderBase::GetJsonFilePath(const FString& PluginName, const FString& FileName)
{
    // Get plugin content directory
    FString ContentDir = GetPluginContentDir(PluginName);
    if (ContentDir.IsEmpty())
    {
        return TEXT("");
    }
    
    // Build: [PluginContentDir]/Data/[FileName]
    FString FullPath = FPaths::Combine(ContentDir, TEXT("Data"), FileName);
    
    // Verify file exists
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullPath))
    {
        UE_LOG(LogJsonReader, Warning, TEXT("JSON file not found: %s"), *FullPath);
    }
    
    return FullPath;
}

FString UJsonReaderBase::GetDataTableAssetPath(const FString& PluginName, const FString& DataTableName)
{
    // Build: /[PluginName]/Data/DT_[DataTableName]
    // This is the UE asset path format (not filesystem)
    return FString::Printf(TEXT("/%s/Data/DT_%s.DT_%s"), *PluginName, *DataTableName, *DataTableName);
}

bool UJsonReaderBase::LoadJsonFile(const FString& FilePath, TSharedPtr<FJsonObject>& OutJsonObject)
{
    // Check file exists
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
    {
        UE_LOG(LogJsonReader, Error, TEXT("JSON file not found: %s"), *FilePath);
        return false;
    }
    
    // Load file to string
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogJsonReader, Error, TEXT("Failed to read JSON file: %s"), *FilePath);
        return false;
    }
    
    // Parse JSON
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
    if (!FJsonSerializer::Deserialize(JsonReader, OutJsonObject) || !OutJsonObject.IsValid())
    {
        UE_LOG(LogJsonReader, Error, TEXT("Failed to parse JSON: %s"), *FilePath);
        return false;
    }
    
    UE_LOG(LogJsonReader, Log, TEXT("Successfully loaded JSON: %s"), *FilePath);
    return true;
}

UDataTable* UJsonReaderBase::LoadOrCreateDataTable(const FString& AssetPath, UScriptStruct* RowStruct)
{
    // Try to load existing DataTable
    UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *AssetPath);
    
    if (DataTable)
    {
        UE_LOG(LogJsonReader, Log, TEXT("Loaded existing DataTable: %s"), *AssetPath);
        return DataTable;
    }
    
#if WITH_EDITOR
    // Create new DataTable in editor
    // Extract package name from asset path (remove .AssetName suffix)
    FString PackageName = FPackageName::ObjectPathToPackageName(AssetPath);
    FString AssetName = FPackageName::GetShortName(AssetPath);
    
    // Remove duplicate suffix if present (DT_Name.DT_Name -> DT_Name)
    int32 DotIndex;
    if (AssetName.FindChar('.', DotIndex))
    {
        AssetName = AssetName.Left(DotIndex);
    }
    
    UPackage* Package = CreatePackage(*PackageName);
    if (!Package)
    {
        UE_LOG(LogJsonReader, Error, TEXT("Failed to create package: %s"), *PackageName);
        return nullptr;
    }
    
    DataTable = NewObject<UDataTable>(Package, *AssetName, RF_Public | RF_Standalone);
    if (!DataTable)
    {
        UE_LOG(LogJsonReader, Error, TEXT("Failed to create DataTable: %s"), *AssetPath);
        return nullptr;
    }
    
    DataTable->RowStruct = RowStruct;
    
    UE_LOG(LogJsonReader, Log, TEXT("Created new DataTable: %s"), *AssetPath);
    return DataTable;
#else
    UE_LOG(LogJsonReader, Error, TEXT("DataTable not found and cannot create at runtime: %s"), *AssetPath);
    return nullptr;
#endif
}

bool UJsonReaderBase::PopulateDataTableFromJson(
    UDataTable* DataTable,
    const TArray<TSharedPtr<FJsonValue>>& JsonArray,
    const FString& RowNameField,
    const FString& ContextString)
{
    if (!DataTable)
    {
        UE_LOG(LogJsonReader, Error, TEXT("[%s] DataTable is null"), *ContextString);
        return false;
    }
    
    if (!DataTable->RowStruct)
    {
        UE_LOG(LogJsonReader, Error, TEXT("[%s] DataTable has no RowStruct"), *ContextString);
        return false;
    }
    
    // Clear existing rows
    DataTable->EmptyTable();
    
    int32 SuccessCount = 0;
    int32 FailCount = 0;
    
    for (int32 i = 0; i < JsonArray.Num(); ++i)
    {
        const TSharedPtr<FJsonObject>* JsonObject;
        if (!JsonArray[i]->TryGetObject(JsonObject))
        {
            UE_LOG(LogJsonReader, Warning, TEXT("[%s] Invalid JSON object at index %d"), *ContextString, i);
            FailCount++;
            continue;
        }
        
        // Get row name from the specified field
        FString RowName;
        if (!(*JsonObject)->TryGetStringField(RowNameField, RowName))
        {
            UE_LOG(LogJsonReader, Warning, TEXT("[%s] Missing '%s' field at index %d"), *ContextString, *RowNameField, i);
            FailCount++;
            continue;
        }
        
        // Convert JSON object to string for DataTable import
        FString JsonRowString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonRowString);
        FJsonSerializer::Serialize(JsonObject->ToSharedRef(), Writer);
        
        // Create row data
        FTableRowBase* NewRow = (FTableRowBase*)FMemory::Malloc(DataTable->RowStruct->GetStructureSize());
        DataTable->RowStruct->InitializeStruct(NewRow);

        
        // Import from JSON string
        // New (correct for UE5.5+)
        FJsonObjectConverter::JsonObjectToUStruct(
            JsonObject->ToSharedRef(),
            DataTable->RowStruct.Get(),
            NewRow
        );
        
        // Add row to DataTable
        DataTable->AddRow(FName(*RowName), *NewRow);
        
        // Cleanup temp allocation
        DataTable->RowStruct->DestroyStruct(NewRow);
        FMemory::Free(NewRow);
        
        SuccessCount++;
    }
    
    UE_LOG(LogJsonReader, Log, TEXT("[%s] Populated %d rows (%d failed)"), *ContextString, SuccessCount, FailCount);
    return SuccessCount > 0;
}

bool UJsonReaderBase::SaveDataTableAsset(UDataTable* DataTable)
{
#if WITH_EDITOR
    if (!DataTable)
    {
        UE_LOG(LogJsonReader, Error, TEXT("Cannot save null DataTable"));
        return false;
    }
    
    DataTable->MarkPackageDirty();
    
    UPackage* Package = DataTable->GetOutermost();
    FString PackageFileName = FPackageName::LongPackageNameToFilename(
        Package->GetName(),
        FPackageName::GetAssetPackageExtension()
    );
    
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.bForceByteSwapping = true;
    SaveArgs.bWarnOfLongFilename = true;
    
    if (!UPackage::SavePackage(Package, DataTable, *PackageFileName, SaveArgs))
    {
        UE_LOG(LogJsonReader, Error, TEXT("Failed to save DataTable: %s"), *PackageFileName);
        return false;
    }
    
    // Notify asset registry
    FAssetRegistryModule::AssetCreated(DataTable);
    
    UE_LOG(LogJsonReader, Log, TEXT("Saved DataTable: %s"), *PackageFileName);
    return true;
#else
    UE_LOG(LogJsonReader, Warning, TEXT("SaveDataTableAsset called at runtime - skipping"));
    return false;
#endif
}