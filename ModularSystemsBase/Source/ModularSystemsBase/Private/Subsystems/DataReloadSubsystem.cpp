// DataReloadSubsystem.cpp
// Location: ModularSystemsBase/Private/Subsystems/DataReloadSubsystem.cpp

#include "Subsystems/DataReloadSubsystem.h"
#include "Utilities/JsonReader/ItemJsonReader.h"
#include "Utilities/JsonReader/RecipeJsonReader.h"
#include "Utilities/JsonReader/InteractableJsonReader.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogDataReload, Log, All);

// ============================================================
// LIFECYCLE
// ============================================================

void UDataReloadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    RegisterConsoleCommands();
    
    UE_LOG(LogDataReload, Log, TEXT("DataReloadSubsystem initialized"));
}

void UDataReloadSubsystem::Deinitialize()
{
    UnregisterConsoleCommands();
    
    Super::Deinitialize();
}

// ============================================================
// PUBLIC API
// ============================================================

bool UDataReloadSubsystem::ReloadItems()
{
    UDataTable* DataTable = nullptr;
    bool bSuccess = UItemJsonReader::ReloadItems(DataTable);
    
    if (bSuccess)
    {
        UE_LOG(LogDataReload, Log, TEXT("Items reloaded: %d rows"), DataTable ? DataTable->GetRowNames().Num() : 0);
    }
    else
    {
        UE_LOG(LogDataReload, Error, TEXT("Failed to reload items"));
    }
    
    return bSuccess;
}

bool UDataReloadSubsystem::ReloadRecipes()
{
    UDataTable* DataTable = nullptr;
    bool bSuccess = URecipeJsonReader::ReloadRecipes(DataTable);
    
    if (bSuccess)
    {
        UE_LOG(LogDataReload, Log, TEXT("Recipes reloaded: %d rows"), DataTable ? DataTable->GetRowNames().Num() : 0);
    }
    else
    {
        UE_LOG(LogDataReload, Error, TEXT("Failed to reload recipes"));
    }
    
    return bSuccess;
}

bool UDataReloadSubsystem::ReloadInteractables()
{
    UDataTable* DataTable = nullptr;
    bool bSuccess = UInteractableJsonReader::ReloadInteractables(DataTable);
    
    if (bSuccess)
    {
        UE_LOG(LogDataReload, Log, TEXT("Interactables reloaded: %d rows"), DataTable ? DataTable->GetRowNames().Num() : 0);
    }
    else
    {
        UE_LOG(LogDataReload, Error, TEXT("Failed to reload interactables"));
    }
    
    return bSuccess;
}

bool UDataReloadSubsystem::ReloadAll()
{
    UE_LOG(LogDataReload, Log, TEXT("=== Reloading all data ==="));
    
    bool bAllSuccess = true;
    
    bAllSuccess &= ReloadItems();
    bAllSuccess &= ReloadRecipes();
    bAllSuccess &= ReloadInteractables();
    
    if (bAllSuccess)
    {
        UE_LOG(LogDataReload, Log, TEXT("=== All data reloaded successfully ==="));
    }
    else
    {
        UE_LOG(LogDataReload, Warning, TEXT("=== Some data failed to reload ==="));
    }
    
    return bAllSuccess;
}

bool UDataReloadSubsystem::ReloadAllInEditor()
{
#if WITH_EDITOR
    UE_LOG(LogDataReload, Log, TEXT("=== Reloading all data (Editor) ==="));
    
    bool bAllSuccess = true;
    
    bAllSuccess &= UItemJsonReader::ReloadItemsInEditor();
    bAllSuccess &= URecipeJsonReader::ReloadRecipesInEditor();
    bAllSuccess &= UInteractableJsonReader::ReloadInteractablesInEditor();
    
    if (bAllSuccess)
    {
        UE_LOG(LogDataReload, Log, TEXT("=== All data reloaded and saved ==="));
    }
    else
    {
        UE_LOG(LogDataReload, Warning, TEXT("=== Some data failed to reload/save ==="));
    }
    
    return bAllSuccess;
#else
    UE_LOG(LogDataReload, Warning, TEXT("ReloadAllInEditor called at runtime"));
    return false;
#endif
}

void UDataReloadSubsystem::PrintStatus()
{
    UE_LOG(LogDataReload, Log, TEXT(""));
    UE_LOG(LogDataReload, Log, TEXT("========================================"));
    UE_LOG(LogDataReload, Log, TEXT("       DATA RELOAD SUBSYSTEM STATUS"));
    UE_LOG(LogDataReload, Log, TEXT("========================================"));
    
    // Items
    UDataTable* ItemDT = GetItemDataTable();
    UE_LOG(LogDataReload, Log, TEXT(""));
    UE_LOG(LogDataReload, Log, TEXT("[ITEMS]"));
    UE_LOG(LogDataReload, Log, TEXT("  JSON Path: %s"), *UItemJsonReader::GetItemJsonPath());
    UE_LOG(LogDataReload, Log, TEXT("  DataTable: %s"), *UItemJsonReader::GetItemDataTablePath());
    UE_LOG(LogDataReload, Log, TEXT("  Loaded: %s"), ItemDT ? TEXT("YES") : TEXT("NO"));
    if (ItemDT)
    {
        UE_LOG(LogDataReload, Log, TEXT("  Row Count: %d"), ItemDT->GetRowNames().Num());
    }
    
    // Recipes
    UDataTable* RecipeDT = GetRecipeDataTable();
    UE_LOG(LogDataReload, Log, TEXT(""));
    UE_LOG(LogDataReload, Log, TEXT("[RECIPES]"));
    UE_LOG(LogDataReload, Log, TEXT("  JSON Path: %s"), *URecipeJsonReader::GetRecipeJsonPath());
    UE_LOG(LogDataReload, Log, TEXT("  DataTable: %s"), *URecipeJsonReader::GetRecipeDataTablePath());
    UE_LOG(LogDataReload, Log, TEXT("  Loaded: %s"), RecipeDT ? TEXT("YES") : TEXT("NO"));
    if (RecipeDT)
    {
        UE_LOG(LogDataReload, Log, TEXT("  Row Count: %d"), RecipeDT->GetRowNames().Num());
    }
    
    // Interactables
    UDataTable* InteractableDT = GetInteractableDataTable();
    UE_LOG(LogDataReload, Log, TEXT(""));
    UE_LOG(LogDataReload, Log, TEXT("[INTERACTABLES]"));
    UE_LOG(LogDataReload, Log, TEXT("  JSON Path: %s"), *UInteractableJsonReader::GetInteractableJsonPath());
    UE_LOG(LogDataReload, Log, TEXT("  DataTable: %s"), *UInteractableJsonReader::GetInteractableDataTablePath());
    UE_LOG(LogDataReload, Log, TEXT("  Loaded: %s"), InteractableDT ? TEXT("YES") : TEXT("NO"));
    if (InteractableDT)
    {
        UE_LOG(LogDataReload, Log, TEXT("  Row Count: %d"), InteractableDT->GetRowNames().Num());
    }
    
    UE_LOG(LogDataReload, Log, TEXT(""));
    UE_LOG(LogDataReload, Log, TEXT("========================================"));
}

// ============================================================
// GETTERS
// ============================================================

UDataTable* UDataReloadSubsystem::GetItemDataTable() const
{
    return UItemJsonReader::GetItemDataTable();
}

UDataTable* UDataReloadSubsystem::GetRecipeDataTable() const
{
    return URecipeJsonReader::GetRecipeDataTable();
}

UDataTable* UDataReloadSubsystem::GetInteractableDataTable() const
{
    return UInteractableJsonReader::GetInteractableDataTable();
}

// ============================================================
// CONSOLE COMMANDS
// ============================================================

void UDataReloadSubsystem::RegisterConsoleCommands()
{
    IConsoleManager& ConsoleManager = IConsoleManager::Get();
    
    // WW.Data.ReloadItems
    ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
        TEXT("WW.Data.ReloadItems"),
        TEXT("Reload inventory items from JSON"),
        FConsoleCommandDelegate::CreateUObject(this, &UDataReloadSubsystem::Cmd_ReloadItems),
        ECVF_Cheat
    ));
    
    // WW.Data.ReloadRecipes
    ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
        TEXT("WW.Data.ReloadRecipes"),
        TEXT("Reload crafting recipes from JSON"),
        FConsoleCommandDelegate::CreateUObject(this, &UDataReloadSubsystem::Cmd_ReloadRecipes),
        ECVF_Cheat
    ));
    
    // WW.Data.ReloadInteractables
    ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
        TEXT("WW.Data.ReloadInteractables"),
        TEXT("Reload interactable objects from JSON"),
        FConsoleCommandDelegate::CreateUObject(this, &UDataReloadSubsystem::Cmd_ReloadInteractables),
        ECVF_Cheat
    ));
    
    // WW.Data.ReloadAll
    ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
        TEXT("WW.Data.ReloadAll"),
        TEXT("Reload all data from JSON files"),
        FConsoleCommandDelegate::CreateUObject(this, &UDataReloadSubsystem::Cmd_ReloadAll),
        ECVF_Cheat
    ));
    
    // WW.Data.Status
    ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
        TEXT("WW.Data.Status"),
        TEXT("Print status of loaded data"),
        FConsoleCommandDelegate::CreateUObject(this, &UDataReloadSubsystem::Cmd_Status),
        ECVF_Cheat
    ));
    
    UE_LOG(LogDataReload, Log, TEXT("Registered %d console commands"), ConsoleCommands.Num());
}

void UDataReloadSubsystem::UnregisterConsoleCommands()
{
    IConsoleManager& ConsoleManager = IConsoleManager::Get();
    
    for (IConsoleObject* Command : ConsoleCommands)
    {
        if (Command)
        {
            ConsoleManager.UnregisterConsoleObject(Command);
        }
    }
    
    ConsoleCommands.Empty();
}

void UDataReloadSubsystem::Cmd_ReloadItems()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Reloading items..."));
    }
    
    if (ReloadItems())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Items reloaded successfully!"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to reload items!"));
        }
    }
}

void UDataReloadSubsystem::Cmd_ReloadRecipes()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Reloading recipes..."));
    }
    
    if (ReloadRecipes())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Recipes reloaded successfully!"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to reload recipes!"));
        }
    }
}

void UDataReloadSubsystem::Cmd_ReloadInteractables()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Reloading interactables..."));
    }
    
    if (ReloadInteractables())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Interactables reloaded successfully!"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to reload interactables!"));
        }
    }
}

void UDataReloadSubsystem::Cmd_ReloadAll()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Reloading all data..."));
    }
    
#if WITH_EDITOR
    if (GIsEditor && !GIsPlayInEditorWorld)
    {
        // In editor without PIE - save assets too
        if (ReloadAllInEditor())
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("All data reloaded and saved!"));
            }
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Some data failed to reload!"));
            }
        }
        return;
    }
#endif
    
    // Runtime or PIE
    if (ReloadAll())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("All data reloaded successfully!"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Some data failed to reload!"));
        }
    }
}

void UDataReloadSubsystem::Cmd_Status()
{
    PrintStatus();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Data status printed to Output Log"));
    }
}