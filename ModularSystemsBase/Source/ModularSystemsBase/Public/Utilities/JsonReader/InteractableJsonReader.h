// InteractableJsonReader.h
// Location: ModularSystemsBase/Public/Utilities/JsonReader/InteractableJsonReader.h

#pragma once

#include "CoreMinimal.h"
#include "Windwalker_Productions_SharedDefaults.h"
#include "Utilities/JsonReader/JsonReaderBase.h"
#include "InteractableJsonReader.generated.h"

/**
 * JSON Reader for Interactable Objects
 * 
 * File: Plugins/ModularInteractionSystem/Content/Data/Interactables.json
 * DataTable: /ModularInteractionSystem/Data/DT_Interactables
 * Row Struct: FInteractableData
 * JSON Array Key: "Interactables"
 * Row Name Field: "InteractableID"
 */
UCLASS()
class MODULARSYSTEMSBASE_API UInteractableJsonReader : public UJsonReaderBase
{
    GENERATED_BODY()

public:

    // ============================================================
    // PUBLIC API
    // ============================================================

    /**
     * Reload interactables from JSON into DataTable (runtime)
     * @param OutDataTable - Optional: returns the populated DataTable
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "JsonReader|Interactables")
    static bool ReloadInteractables(UDataTable*& OutDataTable);

    /**
     * Reload interactables and save DataTable asset (editor only)
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "JsonReader|Interactables", meta = (DevelopmentOnly))
    static bool ReloadInteractablesInEditor();

    /**
     * Get the DataTable (loads if needed)
     * @return DataTable or nullptr
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Interactables")
    static UDataTable* GetInteractableDataTable();

    /**
     * Get the JSON file path
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Interactables")
    static FString GetInteractableJsonPath();

    /**
     * Get the DataTable asset path
     */
    UFUNCTION(BlueprintPure, Category = "JsonReader|Interactables")
    static FString GetInteractableDataTablePath();

protected:
    // ============================================================
    // OVERRIDES
    // ============================================================

    virtual FString GetPluginName() const override { return WWPluginDirectories::PLUGIN_NAME_MODULARINTERACTIONSYSTEM; }
    virtual FString GetJsonFileName() const override { return WWPluginDirectories::JSON_FILE_NAME_INTERACTABLES; }
    virtual FString GetDataTableName() const override { return WWPluginDirectories::DATATABLE_NAME_INTERACTABLES; }
    virtual UScriptStruct* GetRowStructType() const override;
    virtual FString GetJsonArrayKey() const override { return WWPluginDirectories::JSON_ARRAY_KEY_INTERACTABLES; }
    virtual FString GetRowNameField() const override { return WWPluginDirectories::ROW_NAME_FIELD_INTERACTABLE; }

private:
    /** Cached DataTable reference */
    static TWeakObjectPtr<UDataTable> CachedDataTable;
};