#include "CharacterSaveModule.h"

UCharacterSaveModule::UCharacterSaveModule()
{
    ModuleName = TEXT("CharacterSaveModule");
    ModuleVersion = CURRENT_MODULE_VERSION;
}

FPlayerCharacterData& UCharacterSaveModule::GetOrCreatePlayerCharacter(const FString& PlayerID)
{
    if (!PlayerCharacters.Contains(PlayerID))
    {
        FPlayerCharacterData NewCharacter;
        NewCharacter.PlayerID = PlayerID;
        PlayerCharacters.Add(PlayerID, NewCharacter);
    }

    return PlayerCharacters[PlayerID];
}

bool UCharacterSaveModule::HasPlayerData(const FString& PlayerID) const
{
    return PlayerCharacters.Contains(PlayerID);
}

bool UCharacterSaveModule::RemovePlayerData(const FString& PlayerID)
{
    return PlayerCharacters.Remove(PlayerID) > 0;
}

TArray<FString> UCharacterSaveModule::GetAllPlayerIDs() const
{
    TArray<FString> PlayerIDs;
    PlayerCharacters.GetKeys(PlayerIDs);
    return PlayerIDs;
}

bool UCharacterSaveModule::SaveCharacterTransform(const FString& PlayerID, const FTransform& Transform, const FString& LevelName)
{
    FPlayerCharacterData& CharData = GetOrCreatePlayerCharacter(PlayerID);
    CharData.Transform.FromTransform(Transform);
    CharData.Transform.CurrentLevelName = LevelName;
    UpdateTimestamp();
    return true;
}

bool UCharacterSaveModule::LoadCharacterTransform(const FString& PlayerID, FTransform& OutTransform, FString& OutLevelName)
{
    if (!HasPlayerData(PlayerID))
    {
        return false;
    }

    const FPlayerCharacterData& CharData = PlayerCharacters[PlayerID];
    OutTransform = CharData.Transform.ToTransform();
    OutLevelName = CharData.Transform.CurrentLevelName;
    return true;
}

bool UCharacterSaveModule::SaveCharacterStats(const FString& PlayerID, const FCharacterStatsData& Stats)
{
    FPlayerCharacterData& CharData = GetOrCreatePlayerCharacter(PlayerID);
    CharData.Stats = Stats;
    UpdateTimestamp();
    return true;
}

bool UCharacterSaveModule::LoadCharacterStats(const FString& PlayerID, FCharacterStatsData& OutStats)
{
    if (!HasPlayerData(PlayerID))
    {
        return false;
    }

    OutStats = PlayerCharacters[PlayerID].Stats;
    return true;
}

void UCharacterSaveModule::ClearData()
{
    PlayerCharacters.Empty();
}

bool UCharacterSaveModule::ValidateData_Implementation()
{
    for (auto& Pair : PlayerCharacters)
    {
        FPlayerCharacterData& CharData = Pair.Value;
        
        // Clamp health/stats
        CharData.Stats.Health = FMath::Clamp(CharData.Stats.Health, 0.0f, CharData.Stats.MaxHealth);
        CharData.Stats.Stamina = FMath::Clamp(CharData.Stats.Stamina, 0.0f, CharData.Stats.MaxStamina);
        CharData.Stats.Mana = FMath::Clamp(CharData.Stats.Mana, 0.0f, CharData.Stats.MaxMana);
        CharData.Stats.Level = FMath::Max(1, CharData.Stats.Level);
        CharData.Stats.Experience = FMath::Max(0, CharData.Stats.Experience);
        CharData.Stats.Currency = FMath::Max(0, CharData.Stats.Currency);
    }

    return true;
}

void UCharacterSaveModule::MigrateData_Implementation(int32 FromVersion, int32 ToVersion)
{
    // Handle version migrations here if needed
}