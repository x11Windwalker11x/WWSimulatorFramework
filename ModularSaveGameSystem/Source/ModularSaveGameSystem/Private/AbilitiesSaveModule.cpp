#include "AbilitiesSaveModule.h"

UAbilitiesSaveModule::UAbilitiesSaveModule()
{
    ModuleName = TEXT("AbilitiesSaveModule");
    ModuleVersion = CURRENT_MODULE_VERSION;
}

FPlayerAbilitiesData& UAbilitiesSaveModule::GetOrCreatePlayerAbilities(const FString& PlayerID)
{
    if (!PlayerAbilities.Contains(PlayerID))
    {
        FPlayerAbilitiesData NewAbilities;
        NewAbilities.PlayerID = PlayerID;
        PlayerAbilities.Add(PlayerID, NewAbilities);
    }

    return PlayerAbilities[PlayerID];
}

bool UAbilitiesSaveModule::HasPlayerData(const FString& PlayerID) const
{
    return PlayerAbilities.Contains(PlayerID);
}

bool UAbilitiesSaveModule::RemovePlayerData(const FString& PlayerID)
{
    return PlayerAbilities.Remove(PlayerID) > 0;
}

int32 UAbilitiesSaveModule::GetTotalUnlockedAbilities() const
{
    int32 TotalCount = 0;
    
    for (const auto& Pair : PlayerAbilities)
    {
        for (const FAbilitySaveData& Ability : Pair.Value.Abilities)
        {
            if (Ability.bIsUnlocked)
            {
                TotalCount++;
            }
        }
    }
    
    return TotalCount;
}

TArray<FString> UAbilitiesSaveModule::GetAllPlayerIDs() const
{
    TArray<FString> PlayerIDs;
    PlayerAbilities.GetKeys(PlayerIDs);
    return PlayerIDs;
}

void UAbilitiesSaveModule::ClearData()
{
    PlayerAbilities.Empty();
}

bool UAbilitiesSaveModule::ValidateData_Implementation()
{
    for (auto& Pair : PlayerAbilities)
    {
        FPlayerAbilitiesData& AbilityData = Pair.Value;
        
        AbilityData.Abilities.RemoveAll([](const FAbilitySaveData& Ability) {
            return Ability.AbilityID == NAME_None;
        });
        
        for (FAbilitySaveData& Ability : AbilityData.Abilities)
        {
            Ability.Level = FMath::Max(1, Ability.Level);
            Ability.Experience = FMath::Max(0.0f, Ability.Experience);
        }
    }

    return true;
}

void UAbilitiesSaveModule::MigrateData_Implementation(int32 FromVersion, int32 ToVersion)
{
}