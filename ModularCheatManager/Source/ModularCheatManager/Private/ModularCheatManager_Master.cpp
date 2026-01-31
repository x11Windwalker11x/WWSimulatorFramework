#include "ModularCheatManager_Master.h"
#include "Subsystems/DataReloadSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogModularCheatManager);

void UModularCheatManager_Master::InitCheatManager()
{
    Super::InitCheatManager();
    UE_LOG(LogModularCheatManager, Log, TEXT("ModularCheatManager initialized"));
}

ECheatPermissionLevel UModularCheatManager_Master::GetPermissionLevelForPlayer(const FString& DeveloperName) const
{
    if (!DeveloperWhitelistAsset)
    {
        return ECheatPermissionLevel::GodMode;
    }

    for (const FDeveloperAccount& Account : DeveloperWhitelistAsset->DeveloperAccounts)
    {
        if (Account.DeveloperName.Equals(DeveloperName, ESearchCase::IgnoreCase))
        {
            return Account.PermissionLevel;
        }
    }

    return ECheatPermissionLevel::GodMode;
}

void UModularCheatManager_Master::SetCheatPermission(ECheatPermissionLevel NewPermission)
{
    CheatPermissionLevel = NewPermission;
}

void UModularCheatManager_Master::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UModularCheatManager_Master, CheatPermissionLevel);
}

void UModularCheatManager_Master::ReloadItemData()
{
    UDataReloadSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataReloadSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LogModularCheatManager, Error, TEXT("DataReloadSubsystem not found"));
        return;
    }
    
    UDataTable* DT = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Items"));
    if (DT)
    {
        Subsystem->ReloadItems(DT, TEXT("Items.json"), true);
        UE_LOG(LogModularCheatManager, Log, TEXT("Items reloaded"));
    }
    else
    {
        UE_LOG(LogModularCheatManager, Error, TEXT("Failed to load DT_Items"));
    }
}

void UModularCheatManager_Master::ReloadRecipeItemData()
{
    UDataReloadSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataReloadSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LogModularCheatManager, Error, TEXT("DataReloadSubsystem not found"));
        return;
    }
    
    UDataTable* DT = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_RecipeItems"));
    if (DT)
    {
        Subsystem->ReloadRecipeItems(DT, TEXT("RecipeItems.json"), true);
        UE_LOG(LogModularCheatManager, Log, TEXT("Recipe items reloaded"));
    }
    else
    {
        UE_LOG(LogModularCheatManager, Error, TEXT("Failed to load DT_RecipeItems"));
    }
}

void UModularCheatManager_Master::ReloadRecipeData()
{
    UDataReloadSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataReloadSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LogModularCheatManager, Error, TEXT("DataReloadSubsystem not found"));
        return;
    }
    
    UDataTable* DT = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Recipes"));
    if (DT)
    {
        Subsystem->ReloadRecipes(DT, TEXT("Recipes.json"), true);
        UE_LOG(LogModularCheatManager, Log, TEXT("Recipes reloaded"));
    }
    else
    {
        UE_LOG(LogModularCheatManager, Error, TEXT("Failed to load DT_Recipes"));
    }
}

void UModularCheatManager_Master::ReloadAllData()
{
    UDataReloadSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataReloadSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LogModularCheatManager, Error, TEXT("DataReloadSubsystem not found"));
        return;
    }
    
#if WITH_EDITOR
    Subsystem->ReloadAllDataInEditor();
    UE_LOG(LogModularCheatManager, Log, TEXT("All data reloaded"));
#else
    ReloadItemData();
    ReloadRecipeItemData();
    ReloadRecipeData();
    UE_LOG(LogModularCheatManager, Log, TEXT("All data reloaded (runtime mode)"));
#endif
}

void UModularCheatManager_Master::DebugPrintItems()
{
    UDataTable* DT = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Items"));
    if (!DT)
    {
        UE_LOG(LogModularCheatManager, Error, TEXT("Failed to load DT_Items"));
        return;
    }
    
    TArray<FName> RowNames = DT->GetRowNames();
    UE_LOG(LogModularCheatManager, Warning, TEXT("=== DEBUG: Loaded Items (%d total) ==="), RowNames.Num());
    
    for (const FName& RowName : RowNames)
    {
        const FItemData* Row = DT->FindRow<FItemData>(RowName, TEXT("Debug"));
        if (Row)
        {
            UE_LOG(LogModularCheatManager, Warning, TEXT("  %s: %s (Weight: %.2f)"),
                *RowName.ToString(),
                *Row->DisplayName.ToString(),
                Row->Weight);
        }
    }
}