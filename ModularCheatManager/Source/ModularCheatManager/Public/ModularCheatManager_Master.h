#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "DeveloperWhitelistAsset.h"
#include "ModularCheatManager_Master.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogModularCheatManager, Log, All);

/**
 * Master cheat manager with data reload commands
 */
UCLASS(Blueprintable)
class MODULARCHEATMANAGER_API UModularCheatManager_Master : public UCheatManager
{
    GENERATED_BODY()

public:
    // === WHITELIST ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cheat")
    UDeveloperWhitelistAsset* DeveloperWhitelistAsset;

    UFUNCTION(BlueprintCallable, Category = "Cheat")
    ECheatPermissionLevel GetPermissionLevelForPlayer(const FString& DeveloperName) const;

    void SetCheatPermission(ECheatPermissionLevel NewPermission);

    // === DATA RELOAD COMMANDS ===
    
    /** Reload items from JSON */
    UFUNCTION(Exec)
    void ReloadItemData();
    
    /** Reload recipe items from JSON */
    UFUNCTION(Exec)
    void ReloadRecipeItemData();
    
    /** Reload recipes from JSON */
    UFUNCTION(Exec)
    void ReloadRecipeData();
    
    /** Reload all data from JSON */
    UFUNCTION(Exec)
    void ReloadAllData();
    
    /** Debug: print loaded items */
    UFUNCTION(Exec)
    void DebugPrintItems();

protected:
    virtual void InitCheatManager() override;

private:
    UPROPERTY(Replicated)
    ECheatPermissionLevel CheatPermissionLevel = ECheatPermissionLevel::GodMode;
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};