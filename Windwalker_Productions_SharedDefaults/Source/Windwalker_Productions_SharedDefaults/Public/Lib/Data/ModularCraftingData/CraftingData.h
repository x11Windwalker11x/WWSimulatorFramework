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