#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "CraftingData.generated.h"

// ===== FCraftingCapability =====

USTRUCT(BlueprintType)
struct FCraftingCapability : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FGameplayTag CapabilityTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Level = 1;

    FCraftingCapability() {}
    
    FCraftingCapability(FGameplayTag InTag, int32 InLevel)
        : CapabilityTag(InTag), Level(InLevel) {}

    bool IsValid() const
    {
        return CapabilityTag.IsValid() && Level > 0;
    }

    bool operator == (const FGameplayTag& OtherTag) const
    {
        return CapabilityTag == OtherTag;
    }

};

// ===== FCraftingCapabilitySet =====

USTRUCT(BlueprintType)
struct FCraftingCapabilitySet : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FCraftingCapability> Capabilities;
    

    FORCEINLINE bool IsValid() const
    {
        return Capabilities.Num() > 0;
    }
    
    void Clear()

    {
        Capabilities.Empty();
    }

    bool IsEmpty() const
    {
        return Capabilities.Num() == 0;
    }
};

// ===== FStationXPPool =====

USTRUCT(BlueprintType)
struct FStationXPPool : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Progression")
    FGameplayTag StationTag;

    UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
    int32 AvailableXP = 0;

    UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
    int32 TotalXPEarned = 0;

    FStationXPPool() {}
    
    FStationXPPool(FGameplayTag InTag)
        : StationTag(InTag) {}

    

    bool IsValid() const
    {
        return StationTag.IsValid();
    }
};

// ===== FStationInstanceData =====

USTRUCT(BlueprintType)
struct FStationInstanceData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Station")
    FName StationID;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Station")
    FGameplayTag StationTag;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Station", meta = (ClampMin = "1", ClampMax = "3"))
    int32 Tier = 1;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Station", meta = (ClampMin = "1", ClampMax = "3"))
    int32 Level = 1;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Station")
    float CurrentDurability = 100.0f;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Station")
    int32 CurrentXP = 0;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Station")
    float BaseWear = 0.0f;

    bool IsValid() const
    {
        return !StationID.IsNone() && StationTag.IsValid();
    }
    
};

// ===== FToolInstanceData =====

USTRUCT(BlueprintType) 
struct FToolInstanceData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Tool", meta = (ClampMin = "1", ClampMax = "3"))
    int32 Tier = 1;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Tool", meta = (ClampMin = "1", ClampMax = "3"))
    int32 Level = 1;

    UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Tool")
    int32 CurrentXP = 0;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Tool")
    float CurrentDurability = 100.0f;

    // Stat getters - same formulas as station
    /*DEPRECIATED! USE PROGRESSIONHELPERS INSTEAD.*/
    // float GetMaxDurability() const
    // {
    //     float TierMult = FMath::Pow(1.5f, Tier - 1);
    //     float LevelMult = FMath::Pow(1.2f, Level - 1);
    //     return 100.0f * TierMult * LevelMult;
    // }

    // float GetDurabilityLossPerUse() const
    // {
    //     float TierReduction = 1.0f / Tier;
    //     float LevelReduction = 1.0f - (Level - 1) * 0.1f;
    //     return 1.0f * TierReduction * LevelReduction;
    // }
 // int32 GetLevelUpCost() const
 //    {
 //        if (Level >= 3) return 0;
 //        int32 NextLevel = Level + 1;
 //        int32 BaseCost = 25 * NextLevel * (NextLevel - 1) - 25 * Level * (Level - 1);
 //        float TierFactor = FMath::Pow(1.5f, Tier - 1);
 //        return FMath::RoundToInt(BaseCost * TierFactor);
 //    }
 //    void AddXP(int32 Amount)
         //    {
         //        if (Amount > 0)
         //        {
         //            CurrentXP += Amount;
         //        }
         //    }
 //    float GetDurabilityPercent() const
                 //    {
                 //        float Max = GetMaxDurability();
                 //        return Max > 0.0f ? (CurrentDurability / Max) * 100.0f : 0.0f;
                 //    }
    // void ApplyWear()
    //     {
    //         CurrentDurability = FMath::Max(0.0f, CurrentDurability - GetDurabilityLossPerUse());
    //     }
    //
    //     bool IsBroken() const
    //     {
    //         return CurrentDurability <= 0.0f;
    //     }
    //
    //     void Repair(float Amount)
    //     {
    //         CurrentDurability = FMath::Min(GetMaxDurability(), CurrentDurability + Amount);
    //     }
    //
    //     void RepairFull()
    //     {
    //         CurrentDurability = GetMaxDurability();
    //     }
    
   
    bool IsValid() const
    {
        return Tier >= 1 && Tier <= 3 && Level >= 1 && Level <= 3;
    }
};

// ===== FToolTierUpgradeData =====

USTRUCT(BlueprintType)
struct FToolTierUpgradeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    FGameplayTag ToolTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade", meta = (ClampMin = "2", ClampMax = "3"))
    int32 TargetTier = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    FGameplayTag RequiredStationTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade", meta = (ClampMin = "1", ClampMax = "3"))
    int32 RequiredStationTier = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    TMap<FName, int32> ResourceCosts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    int32 MoneyCost = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    int32 RequiredRank = 0;

    bool IsValid() const
    {
        return ToolTag.IsValid() && TargetTier >= 2 && TargetTier <= 3;
    }
};

USTRUCT(BlueprintType)
struct FCrafterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FName ID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText DisplayName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer BaseTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCraftingCapabilitySet StartingCapabilitySet;

    
};

USTRUCT(BlueprintType)
struct FCrafterInstanceData : FCrafterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCraftingCapabilitySet CurrentCapabilities;

    
};