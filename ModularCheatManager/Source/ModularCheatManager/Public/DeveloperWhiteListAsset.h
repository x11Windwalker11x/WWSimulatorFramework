// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ECheatPermissionLevel.h"
#include "DeveloperWhitelistAsset.generated.h"

USTRUCT(BlueprintType)
struct FDeveloperAccount
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString DeveloperName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ECheatPermissionLevel PermissionLevel = ECheatPermissionLevel::GodMode;

    FDeveloperAccount()
    {
        DeveloperName = TEXT("");
        PermissionLevel = ECheatPermissionLevel::GodMode;
    }
};

UCLASS(BlueprintType)
class MODULARCHEATMANAGER_API UDeveloperWhitelistAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FDeveloperAccount> DeveloperAccounts;

    UFUNCTION(BlueprintCallable, Category = "Whitelist")
    bool HasDeveloper(const FString& DeveloperName) const;

    UFUNCTION(BlueprintCallable, Category = "Whitelist")
    ECheatPermissionLevel GetPermissionForPlayer(const FString& DeveloperName) const;
};
