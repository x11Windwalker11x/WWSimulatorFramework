// DataReloadSubsystem.h
// Location: ModularSystemsBase/Public/Subsystems/DataReloadSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "DataReloadSubsystem.generated.h"

/**
 * Subsystem that orchestrates JSON to DataTable reload operations
 * 
 * Provides:
 * - Runtime reload of all data types
 * - Editor reload with asset saving
 * - Console commands for debugging
 * 
 * Console Commands:
 * - WW.Data.ReloadItems       - Reload inventory items
 * - WW.Data.ReloadRecipes     - Reload crafting recipes
 * - WW.Data.ReloadInteractables - Reload interactable objects
 * - WW.Data.ReloadAll         - Reload all data types
 * - WW.Data.Status            - Show loaded data status
 */
UCLASS()
class MODULARSYSTEMSBASE_API UDataReloadSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    // ============================================================
    // LIFECYCLE
    // ============================================================

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ============================================================
    // PUBLIC API
    // ============================================================

    /**
     * Reload inventory items from JSON
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "DataReload")
    bool ReloadItems();

    /**
     * Reload crafting recipes from JSON
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "DataReload")
    bool ReloadRecipes();

    /**
     * Reload interactable objects from JSON
     * @return true if successful
     */
    UFUNCTION(BlueprintCallable, Category = "DataReload")
    bool ReloadInteractables();

    /**
     * Reload all data types
     * @return true if all succeeded
     */
    UFUNCTION(BlueprintCallable, Category = "DataReload")
    bool ReloadAll();

    /**
     * Reload all and save assets (editor only)
     * @return true if all succeeded
     */
    UFUNCTION(BlueprintCallable, Category = "DataReload", meta = (DevelopmentOnly))
    bool ReloadAllInEditor();

    /**
     * Print status of all loaded data
     */
    UFUNCTION(BlueprintCallable, Category = "DataReload")
    void PrintStatus();

    // ============================================================
    // GETTERS
    // ============================================================

    /** Get item DataTable */
    UFUNCTION(BlueprintPure, Category = "DataReload")
    UDataTable* GetItemDataTable() const;

    /** Get recipe DataTable */
    UFUNCTION(BlueprintPure, Category = "DataReload")
    UDataTable* GetRecipeDataTable() const;

    /** Get interactable DataTable */
    UFUNCTION(BlueprintPure, Category = "DataReload")
    UDataTable* GetInteractableDataTable() const;

private:
    // ============================================================
    // CONSOLE COMMANDS
    // ============================================================

    void RegisterConsoleCommands();
    void UnregisterConsoleCommands();

    // Command handlers
    void Cmd_ReloadItems();
    void Cmd_ReloadRecipes();
    void Cmd_ReloadInteractables();
    void Cmd_ReloadAll();
    void Cmd_Status();

    // Console command handles
    TArray<IConsoleObject*> ConsoleCommands;
};