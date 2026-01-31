// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "ECheatPermissionLevel.h"
#include "ModularCheatManager.generated.h"

class UDeveloperWhitelistAsset;

UCLASS()
class MODULARCHEATMANAGER_API UModularCheatManager : public UCheatManager
{
    GENERATED_BODY()

public:
    // Whitelist asset reference (set via PlayerController or server config) 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat Manager")
    UDeveloperWhitelistAsset* DeveloperWhitelist;

    // Current player permission level (assigned on login) 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cheat Manager")
    ECheatPermissionLevel CurrentPermission = ECheatPermissionLevel::GodMode;

public:
    //Assign permission based on whitelist 
    UFUNCTION(Server, Reliable)
    void ServerAssignPermission();

    //Check if the player has at least the required permission 
    UFUNCTION(BlueprintCallable, Category = "Cheat Manager")
    bool HasCheatPermission(ECheatPermissionLevel RequiredLevel) const;

    //xample Dev Command 
    UFUNCTION(exec)
    void Cheat_GiveItem(FName ItemID);
};
