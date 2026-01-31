#include "DeveloperWhitelistAsset.h"

#include "DeveloperWhitelistAsset.h"

bool UDeveloperWhitelistAsset::HasDeveloper(const FString& PlayerName) const
{
    // Get prefix before dash for input - DECLARE as local variable
    FString InputPrefix = PlayerName.Split(TEXT("-"), nullptr, nullptr)
                         ? PlayerName.Left(PlayerName.Find(TEXT("-")))
                         : PlayerName;

    for (const FDeveloperAccount& Account : DeveloperAccounts)
    {
        // Get prefix before dash for account
        FString AccountPrefix = Account.DeveloperName.Split(TEXT("-"), nullptr, nullptr) 
                                 ? Account.DeveloperName.Left(Account.DeveloperName.Find(TEXT("-"))) 
                                 : Account.DeveloperName;
        UE_LOG(LogTemp, Log, TEXT("Trimmed DeveloperName: %s"), *InputPrefix);

        if (AccountPrefix.TrimStartAndEnd().Equals(InputPrefix.TrimStartAndEnd(), ESearchCase::IgnoreCase))
        {
            return true;
        }
        else return false;
    }

    return false;
}

// In DeveloperWhitelistAsset.cpp
ECheatPermissionLevel UDeveloperWhitelistAsset::GetPermissionForPlayer(const FString& PlayerName) const
{
    // Get prefix before dash for input
    FString InputPrefix = PlayerName.Split(TEXT("-"), nullptr, nullptr) 
                         ? PlayerName.Left(PlayerName.Find(TEXT("-"))) 
                         : PlayerName;

    // ADD THESE DEBUG LOGS HERE:
    UE_LOG(LogTemp, Warning, TEXT("Looking for player prefix: '%s' from full name: '%s'"), *InputPrefix, *PlayerName);
    UE_LOG(LogTemp, Warning, TEXT("Total accounts in whitelist: %d"), DeveloperAccounts.Num());

    for (const FDeveloperAccount& Account : DeveloperAccounts)
    {
        FString AccountPrefix = Account.DeveloperName.Split(TEXT("-"), nullptr, nullptr) 
                               ? Account.DeveloperName.Left(Account.DeveloperName.Find(TEXT("-"))) 
                               : Account.DeveloperName;

        // ADD MORE DEBUG HERE IF YOU WANT:
        UE_LOG(LogTemp, Warning, TEXT("Checking account: '%s' (prefix: '%s')"), *Account.DeveloperName, *AccountPrefix);

        if (AccountPrefix.TrimStartAndEnd().Equals(InputPrefix.TrimStartAndEnd(), ESearchCase::IgnoreCase))
        {
            UE_LOG(LogTemp, Warning, TEXT("MATCH! Returning: %s"), *UEnum::GetValueAsString(Account.PermissionLevel));
            return Account.PermissionLevel;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("No match found, returning None"));
    return ECheatPermissionLevel::GodMode;
}