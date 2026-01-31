#include "Utilities/Helpers/Progression/ProgressionHelpers.h"

#include <Programs/UnrealBuildAccelerator/Core/Public/UbaBase.h>

#include "Channels/MovieSceneChannel.h"
#include "Lib/Data/ModularCraftingData/CraftingData.h"

// ===== XP & LEVELING =====

int32 UProgressionHelpers::GetXPRequiredForLevel(int32 TargetLevel)
{
    if (TargetLevel <= 1) return 0;
    
    // Formula: 25 * (Level-1) * Level
    // Level 2: 50, Level 3: 150, Level 4: 300, Level 5: 500
    return 25 * (TargetLevel - 1) * TargetLevel;
}

int32 UProgressionHelpers::GetLevelUpCost(int32 CurrentLevel, int32 Tier, float TierMultiplier)
{
    if (CurrentLevel < 1) CurrentLevel = 1;
    if (Tier < 1) Tier = 1;
    
    // Cost to go from CurrentLevel to CurrentLevel+1
    int32 BaseCost = GetXPRequiredForLevel(CurrentLevel + 1) - GetXPRequiredForLevel(CurrentLevel);
    
    // Tier 1: 1.0, Tier 2: 1.5, Tier 3: 2.25
    float TierFactor = FMath::Pow(TierMultiplier, Tier - 1);
    
    return FMath::RoundToInt(BaseCost * TierFactor);
}

int32 UProgressionHelpers::CalculateCraftingXP(int32 RecipeComplexity, int32 Quantity, float QualityRatio, float MiniGameScore, int32 ItemRarity)
{
    // Clamp inputs
    RecipeComplexity = FMath::Clamp(RecipeComplexity, 1, 10);
    Quantity = FMath::Max(1, Quantity);
    QualityRatio = FMath::Clamp(QualityRatio, 0.0f, 1.0f);
    MiniGameScore = FMath::Clamp(MiniGameScore, 0.0f, 1.0f);
    ItemRarity = FMath::Clamp(ItemRarity, 0, 4);
    
    // Base XP from complexity
    float Base = RecipeComplexity * 10.0f;
    
    // Multipliers
    float QuantityMult = FMath::Sqrt(static_cast<float>(Quantity));  // Diminishing returns
    float QualityMult = 0.5f + QualityRatio * 0.5f;                  // 0.5 - 1.0
    float PerfectionMult = 0.5f + MiniGameScore * 0.5f;              // 0.5 - 1.0
    float RarityMult = 1.0f + ItemRarity * 0.25f;                    // 1.0 - 2.0
    
    return FMath::RoundToInt(Base * QuantityMult * QualityMult * PerfectionMult * RarityMult);
}

// ===== STAT MULTIPLIERS =====

float UProgressionHelpers::GetEfficiencyMultiplier(int32 Tier, int32 Level)
{
    Tier = FMath::Clamp(Tier, 1, 3);
    Level = FMath::Clamp(Level, 1, 3);
    
    // Tier: 1.0, 1.25, 1.5
    float TierBase = 1.0f + (Tier - 1) * 0.25f;
    
    // Level: 1.0, 1.1, 1.2
    float LevelBonus = 1.0f + (Level - 1) * 0.1f;
    
    return TierBase * LevelBonus;
}

float UProgressionHelpers::GetSpeedMultiplier(int32 Tier, int32 Level)
{
    Tier = FMath::Clamp(Tier, 1, 3);
    Level = FMath::Clamp(Level, 1, 3);
    
    // Lower = faster
    // Tier 1: 1.0, Tier 2: 0.83, Tier 3: 0.69
    float TierFactor = 1.0f / FMath::Pow(1.2f, Tier - 1);
    
    // Level 1: 1.0, Level 2: 0.87, Level 3: 0.76
    float LevelFactor = 1.0f / FMath::Pow(1.15f, Level - 1);
    
    return TierFactor * LevelFactor;
}

int32 UProgressionHelpers::GetCapacity(int32 Tier, int32 Level)
{
    Tier = FMath::Clamp(Tier, 1, 3);
    Level = FMath::Clamp(Level, 1, 3);
    
    // Tier: 1, 2, 4
    int32 TierBase = FMath::RoundToInt(FMath::Pow(2.0f, Tier - 1));
    
    // Level adds: 0, 1, 2
    int32 LevelBonus = Level - 1;
    
    return TierBase + LevelBonus;
}

float UProgressionHelpers::GetQualityBonus(int32 Tier, int32 Level)
{
    Tier = FMath::Clamp(Tier, 1, 3);
    Level = FMath::Clamp(Level, 1, 3);
    
    // Same as GetEfficiencyMultiplier
    float TierBase = 1.0f + (Tier - 1) * 0.25f;
    float LevelBonus = 1.0f + (Level - 1) * 0.1f;
    
    return TierBase * LevelBonus;

}

float UProgressionHelpers::GetSpeedBonus(int32 Tier, int32 Level)
{
    Tier = FMath::Clamp(Tier, 1, 3);
    Level = FMath::Clamp(Level, 1, 3);
    
    // Same as GetSpeedMultiplier
    float TierFactor = 1.0f / FMath::Pow(1.2f, Tier - 1);
    float LevelFactor = 1.0f / FMath::Pow(1.15f, Level - 1);
    
    return TierFactor * LevelFactor;

}

// ===== DURABILITY =====

float UProgressionHelpers::GetMaxDurability(int32 Tier, int32 Level, float BaseDurability)
{
    Tier = FMath::Clamp(Tier, 1, 3);
    Level = FMath::Clamp(Level, 1, 3);
    
    // Tier: 1.0, 1.5, 2.25
    float TierMult = FMath::Pow(1.5f, Tier - 1);
    
    // Level: 1.0, 1.2, 1.44
    float LevelMult = FMath::Pow(1.2f, Level - 1);
    
    return BaseDurability * TierMult * LevelMult;
}

float UProgressionHelpers::GetDurabilityLossPerUse(int32 Tier, int32 Level, float BaseWear)
{
    Tier = FMath::Clamp(Tier, 1, 3);
    Level = FMath::Clamp(Level, 1, 3);
    
    // Higher tier = less wear (1.0, 0.5, 0.33)
    float TierReduction = 1.0f / Tier;
    
    // Higher level = less wear (1.0, 0.9, 0.8)
    float LevelReduction = 1.0f - (Level - 1) * 0.1f;
    
    return BaseWear * TierReduction * LevelReduction;
}

float UProgressionHelpers::GetDurabilityPercent(float CurrentDurability, int32 Tier, int32 Level, float BaseDurability)
{
    float Max = GetMaxDurability(Tier, Level, BaseDurability);
    return Max > 0.0f ? (CurrentDurability / Max) * 100.0f : 0.0f;
}

bool UProgressionHelpers::CanLevelUp(int32 CurrentXP, int32 CurrentLevel, int32 Tier, float TierMultiplier)
{
    if (CurrentLevel >= 3) return false;
    
    int32 Cost = GetLevelUpCost(CurrentLevel, Tier, TierMultiplier);
    return CurrentXP >= Cost;

}

float UProgressionHelpers::GetXPProgressPercent(int32 CurrentXP, int32 CurrentLevel, int32 Tier, float TierMultiplier)
{
    int32 Cost = GetLevelUpCost(CurrentLevel, Tier, TierMultiplier);
    return Cost > 0 ? FMath::Clamp((float)CurrentXP / (float)Cost, 0.0f, 1.0f) : 0.0f;

}

int32 UProgressionHelpers::GetUsesUntilBroken(int32 Tier, int32 Level, float BaseDurability, float BaseWear)
{
    float MaxDur = GetMaxDurability(Tier, Level, BaseDurability);
    float WearPerUse = GetDurabilityLossPerUse(Tier, Level, BaseWear);
    
    if (WearPerUse <= 0.0f) return INT32_MAX;
    
    return FMath::FloorToInt(MaxDur / WearPerUse);
}


/*--- FSTATIONINSTANCEDATA HELPERS ---*/

void UProgressionHelpers::ApplyWear(FStationInstanceData& Data)
{
    float Wear = GetDurabilityLossPerUse(Data.Tier, Data.Level, Data.BaseWear);
    Data.CurrentDurability = FMath::Max(0.0f, Data.CurrentDurability - Wear);
}

void UProgressionHelpers::Repair(FStationInstanceData& Data, float Amount)
{
    float Max = GetMaxDurability(Data.Tier, Data.Level, Data.BaseWear);
    Data.CurrentDurability = FMath::Min(Max, Data.CurrentDurability + Amount);
}

void UProgressionHelpers::RepairFull(FStationInstanceData& Data)
{
    Data.CurrentDurability = GetMaxDurability(Data.Tier, Data.Level, Data.BaseWear);
}

bool UProgressionHelpers::IsBroken(const FStationInstanceData& Data)
{
    return Data.CurrentDurability <= 0.00f;
}

float UProgressionHelpers::GetDurabiltiyPercent(const FStationInstanceData& Data)
{
    float Max = GetMaxDurability(Data.Tier, Data.Level, Data.BaseWear);
    //Note to self: ternary expression : expression ? return_value_true : return_value_false
    return Max > 0.0f ? Data.CurrentDurability / Max : 0.0f;
}

float UProgressionHelpers::AddXP(FStationInstanceData& Data, const int32& Amount)
{
    Data.CurrentXP += Amount;
    
    bool LeveledUP = false;
    int32 XPNeeded = GetXPRequiredForLevel(Data.Level);
    for (; Data.CurrentXP >= XPNeeded && Data.Level < 3; XPNeeded = GetXPRequiredForLevel(Data.Level+1)/* requeried xp for next level*/);
    {
        Data.CurrentXP -= XPNeeded;
        Data.Level++;
        LeveledUP = true;
        XPNeeded = GetXPRequiredForLevel(Data.Level+1) /*Next Level*/;
    }
    return LeveledUP;
}

/*Removes exp, on level down: returns 1.0f if true 0.0f if false
 *Primarily for debugging
 */
float UProgressionHelpers::RemoveXP(FStationInstanceData& Data, const int32& Amount)
{
    Data.CurrentXP -= Amount;
    bool bLeveledDown = false;

    for (int32 i = 0; i < 3; ++i)
    {
        // If XP is negative, we need to drop a level to "refill" it
        if (Data.Level > 0 && Data.CurrentXP < 0)
        {
            Data.Level--;
            int32 XPFromPrevLevel = GetXPRequiredForLevel(Data.Level);
            Data.CurrentXP += XPFromPrevLevel;
            bLeveledDown = true;
        }
        else
        {
            break;
        }
    }
    
    // Clamp at 0 so we don't have negative XP at Level 0
    if (Data.Level == 0 && Data.CurrentXP < 0) Data.CurrentXP = 0;

    return bLeveledDown;
}

//Immediately sets xp and updates the level, primarily used for ui 
float UProgressionHelpers::SetXP(FStationInstanceData& Data, const int32 NewXPValue)
{
    // 1. Logic Check: You cannot pass 'const' Data if you intend to modify it.
    // I have removed 'const' from the signature.

    int32 Delta = NewXPValue - Data.CurrentXP;

    if (Delta > 0)
    {
        AddXP(Data, Delta);
    }
    else if (Delta < 0)
    {
        // Note: FMath::Abs converts negative to positive for the function call
        RemoveXP(Data, FMath::Abs(Delta));
    }

    // 2. Return Value: You defined the return as 'float'. 
    // Usually, this should return the "Progress Percentage" (0.0f - 1.0f) for the UI.
    int32 MaxXP = GetXPRequiredForLevel(Data.Level);
    
    return MaxXP > 0 ? (float)Data.CurrentXP / (float)MaxXP : 0.0f;
}

float UProgressionHelpers::GetMaxDurability(const FStationInstanceData& Data)
{
    float TierMult = FMath::Pow(1.5f, Data.Tier - 1);
    float LevelMult = FMath::Pow(1.2f, Data.Level - 1);
    return 100.0f * TierMult * LevelMult;
}

float UProgressionHelpers::GetDurabilityLossPerUse(const FStationInstanceData& Data)
{
    float TierReduction = 1.0f / Data.Tier;
    float LevelReduction = 1.0f - (Data.Level - 1) * 0.1f;
    return 1.0f * TierReduction * LevelReduction;
}

float UProgressionHelpers::GetEfficiencyMultiplier(const FStationInstanceData& Data)
{
    float TierBase = 1.0f + (Data.Tier - 1) * 0.25f;
    float LevelBonus = 1.0f + (Data.Level - 1) * 0.1f;
    return TierBase * LevelBonus;
}

float UProgressionHelpers::GetSpeedMultiplier(const FStationInstanceData& Data)
{
    float TierFactor = 1.0f / FMath::Pow(1.2f, Data.Tier - 1);
    float LevelFactor = 1.0f / FMath::Pow(1.15f, Data.Level - 1);
    return TierFactor * LevelFactor;
}

int32 UProgressionHelpers::GetCapacity(const FStationInstanceData& Data)
{
    //lesson pow accepts float, use static casts to turn into args into float and result into int
    //when necessary.
    float PowBasedFormulaResult = FMath::Pow( 2.0f , static_cast<float>(Data.Tier - 1));
    int32 TierBase = static_cast<int>(PowBasedFormulaResult);
    int32 LevelBonus = Data.Level - 1;
    return TierBase + LevelBonus;
}

int32 UProgressionHelpers::GetLevelUpCost(const FStationInstanceData& Data)
{
    if (Data.Level >= 3) return 0;
    int32 NextLevel = Data.Level + 1;
    int32 BaseCost = 25 * NextLevel * (NextLevel - 1) - 25 * Data.Level * (Data.Level - 1);
    float TierFactor = FMath::Pow(1.5f, Data.Tier - 1);
    return FMath::RoundToInt(BaseCost * TierFactor);
}

float UProgressionHelpers::GetDurabilityPercent(const FStationInstanceData& Data)
{
    float Max = GetMaxDurability(Data);
    return Max > 0.0f ? (Data.CurrentDurability / Max) * 100.0f : 0.0f;
}

bool UProgressionHelpers::TryLevelUp(int32 Level, int32 Tier, int32 TierMultiplier,  int32 CurrentXP)
{
    int32 Cost = GetLevelUpCost(Level, Tier, TierMultiplier);
    if (Level < 3 && CurrentXP >= Cost)
    {
        CurrentXP -= Cost;
        Level++;
        return true;
    }
    return false;
}

void UProgressionHelpers::AddXP(FStationXPPool& StationXPPool, int32 Amount)
{
    if (Amount > 0)
    {
        StationXPPool.AvailableXP += Amount;
        StationXPPool.TotalXPEarned += Amount;
    }
}

bool UProgressionHelpers::SpendXP(FStationXPPool& StationXPPool, int32 Amount)
{
    if (Amount > 0 && StationXPPool.AvailableXP >= Amount)
    {
        StationXPPool.AvailableXP -= Amount;
        return true;
    }
    return false;
}