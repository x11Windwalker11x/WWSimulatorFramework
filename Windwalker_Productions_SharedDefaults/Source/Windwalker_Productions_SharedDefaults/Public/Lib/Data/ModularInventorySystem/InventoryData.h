// InventoryData.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/Core/WorldObjectData.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Lib/Data/ModularCraftingData/CraftingData.h"
#include "InventoryData.generated.h"
// ============================================================
// WEAPON STATS
// ============================================================

USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FRangedWeaponStats
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Damage = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float RateOfFire = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accuracy", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Accuracy = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accuracy")
    float Range = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
    int32 MagSize = 30;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
    float ReloadTime = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
    float RecoilVertical = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
    float RecoilHorizontal = 0.0f;
    
    float GetDPS() const { return Damage * RateOfFire; }
    float GetEffectiveDPS() const { return GetDPS() * Accuracy; }
    bool IsValid() const { return Damage > 0.0f; }
};

USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FMeleeWeaponStats
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Damage = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float AttackSpeed = 1.0f; // Attacks per second
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
    float Range = 100.0f; // Melee reach in units
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float StaminaCost = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block")
    float BlockEfficiency = 0.0f; // 0 = can't block, 1 = perfect block
    
    float GetDPS() const { return Damage * AttackSpeed; }
    bool IsValid() const { return Damage > 0.0f; }
};

// ============================================================
// THROWABLE STATS
// ============================================================

USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FThrowableWeaponStats
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Damage = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bCanCauseDamage = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
    float ImpactRadius = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
    float FuseTime = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
    float ThrowForce = 1000.0f;
    
    bool IsLethal() const { return bCanCauseDamage && Damage > 0.0f; }
    bool IsValid() const { return ImpactRadius > 0.0f; }
};

// ============================================================
// ARMOR STATS
// ============================================================

USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FArmorStats
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protection")
    int32 ArmorPoints = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protection", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DamageReduction = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobility")
    float MovementModifier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobility")
    float NoiseModifier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
    float ExplosiveResist = 0.0f;
    
    bool IsValid() const { return ArmorPoints > 0 || DamageReduction > 0.0f; }
};

// ============================================================
// CONSUMABLE STATS
// ============================================================

USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FConsumableStats
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float HealthRestore = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float StaminaRestore = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float Duration = 0.0f; // 0 = instant
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage")
    float UseTime = 1.0f; // Seconds to consume
    
    bool IsInstant() const { return Duration <= 0.0f; }
    bool IsValid() const { return HealthRestore > 0.0f || StaminaRestore > 0.0f; }
};

/**
 * Item definition data.
 * Extends FWorldObjectData with inventory-specific properties.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FItemData : public FPickupableDataTableData
{
    GENERATED_BODY()

    //SpawnData

    //PickUp Object Class to show something in the world.
    TSoftClassPtr<AActor> PickupActorClass;

    //Texture Data
    TSoftObjectPtr<UTexture2D> Soft_Icon = nullptr;

    // === STACKING ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stacking", meta = (ClampMin = "1"))
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stacking")
    bool bIsStackable = true;

    // === QUALITY/RARITY ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    FGameplayTag Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality", meta = (ClampMin = "0", ClampMax = "100"))
    float BaseQuality = 100.0f;

    // === ECONOMICS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economics", meta = (ClampMin = "0"))
    int32 BaseValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economics")
    float Weight = 0.0f;

    // === CATEGORIZATION ===
    
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
    // FGameplayTag ItemType;
    //
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
    // FGameplayTag SlotType;

    // === USAGE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsRangedWeapon = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsMeleeWeapon = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsThrowableWeapon = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsArmor = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage")
    bool bIsConsumable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage")
    bool bDestroyOnUse = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage", meta = (ClampMin = "0.0"))
    float UseCooldown = 0.0f;

    // === DURABILITY (from old ItemData.h) ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability")
    bool bHasDurability = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability", meta = (EditCondition = "bHasDurability"))
    float DurabilityDrainPerUse = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability", meta = (EditCondition = "bHasDurability"))
    float Durability = 1.0f;
    
    // === TYPE-SPECIFIC STATS (Tag-Driven) ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Ranged")
    FRangedWeaponStats RangedStats;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Melee")
    FMeleeWeaponStats MeleeStats;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Throwable")
    FThrowableWeaponStats ThrowableStats;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Armor")
    FArmorStats ArmorStats;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Consumable")
    FConsumableStats ConsumableStats;


    // === HELPERS ===    
    
    FORCEINLINE bool GetIsStackable() const
    {
        if (bIsStackable) return true;
        
        if (BaseTags.HasTagExact(FWWTagLibrary::Inventory_Item_Flags_QuestItem())
            || MaxStackSize <= 1)
            return false;
        return true;
    }

    FORCEINLINE FLinearColor GetRarityColor() const
    {
        if (Rarity.MatchesTag(FWWTagLibrary::Item_Rarity_Legendary()))
            return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);  // Orange
        if (Rarity.MatchesTag(FWWTagLibrary::Item_Rarity_Epic()))
            return FLinearColor(0.58f, 0.0f, 0.83f, 1.0f);  // Purple
        if (Rarity.MatchesTag(FWWTagLibrary::Item_Rarity_Rare()))
            return FLinearColor(0.0f, 0.44f, 0.87f, 1.0f);  // Blue
        if (Rarity.MatchesTag(FWWTagLibrary::Item_Rarity_Uncommon()))
            return FLinearColor(0.12f, 0.56f, 0.1f, 1.0f);  // Green
        if (Rarity.MatchesTag(FWWTagLibrary::Item_Rarity_Common()))
            return FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);  // Gray

        return FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);  // Default
    }
    FORCEINLINE bool IsRangedWeapon() const
    {
        return BaseTags.HasTag(FWWTagLibrary::Item_Type_Weapon_Ranged());
    }

    FORCEINLINE bool IsMeleeWeapon() const
    {
        return BaseTags.HasTag(FWWTagLibrary::Item_Type_Weapon_Melee());
    }

    FORCEINLINE bool IsThrowable() const
    {
        return BaseTags.HasTag(FWWTagLibrary::Item_Type_Throwable());
    }

    FORCEINLINE bool IsArmor() const
    {
        return BaseTags.HasTag(FWWTagLibrary::Item_Type_Armor());
    }

    FORCEINLINE bool IsConsumable() const
    {
        return BaseTags.HasTag(FWWTagLibrary::Item_Type_Consumable());
    }
    
    FORCEINLINE bool IsWeapon() const { return IsRangedWeapon() || IsMeleeWeapon(); }
    
    FORCEINLINE float GetDPS() const
    {
        if (IsRangedWeapon() && RangedStats.IsValid()) return RangedStats.GetDPS();
        if (IsMeleeWeapon() && MeleeStats.IsValid()) return MeleeStats.GetDPS();
        return 0.0f;
    }

};

/*
 *ItemData_Dynamic: used when the itemdata is actually created:
 *like creating in game with visuals and everything on item dropped
 *or created when the game initialized in current map that the game 
 * initially loaded for, for example.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FItemData_Dynamic : public FWorldObjectData
{
    GENERATED_BODY()
        
    
    UPROPERTY(BlueprintReadOnly, Category = "Item Info")
    float Weight = 0.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Item Info")
    int32 MaxStackSize = 1;

    // Soft References to Animation Montages
    UPROPERTY(BlueprintReadOnly, Category = "Animations")
    TSoftObjectPtr<UAnimMontage> PickupMontage;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animations")
    TSoftObjectPtr<UAnimMontage> DropMontage;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animations")
    TSoftObjectPtr<UAnimMontage> UseMontage;

    // Soft References to Sounds
    UPROPERTY(BlueprintReadOnly, Category = "Sounds")
    TSoftObjectPtr<USoundBase> PickupSound;
    
    UPROPERTY(BlueprintReadOnly, Category = "Sounds")
    TSoftObjectPtr<USoundBase> DropSound;
    
    UPROPERTY(BlueprintReadOnly, Category = "Sounds")
    TSoftObjectPtr<USoundBase> UseSound;

    // Loading State - Do not touch it it is determined internally!
    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    bool bIsLoaded = false;

    // === PICKUP INSTANCE DATA (NEW) ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    float Quality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    float Durability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    bool bIsStolen = false;
    
    void Reset()
    {
        ID = FName();
        DisplayName = FText::GetEmpty();
        Description = FText::GetEmpty();
        Weight = 0.0f;
        MaxStackSize = 1;
        Soft_SM_Mesh.Reset();
        Soft_SK_Mesh.Reset();
        PickupMontage.Reset();
        DropMontage.Reset();
        UseMontage.Reset();
        PickupSound.Reset();
        DropSound.Reset();
        UseSound.Reset();
        bIsLoaded = false;
    }
};

/**
 * Runtime inventory slot instance.
 * Contains both item reference and per-instance state.
 * 
 * NOTE: This struct holds DATA ONLY. All game logic queries
 * are delegated to UInventoryComponent.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FInventorySlot : public FTableRowBase
{
    GENERATED_BODY()

    // === CORE ITEM DATA ===
    
    /** Unique instance ID for this specific item instance */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FGuid InstanceID;
    
    /** Item ID (row name in ItemDataTable) */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FName ItemID = NAME_None;
    
    /** Current stack quantity */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot", meta = (ClampMin = "0"))
    int32 Quantity = 0;
    
    /** Maximum stack size for this item */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot", meta = (ClampMin = "1"))
    int32 MaxStackSize = 1;

    // === PER-INSTANCE STATE ===
    
    /** Item quality/purity (0.0 - 1.0) */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    float Quality = 1.0f;
    
    /** Item durability (0.0 - 1.0) */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    float Durability = 1.0f;
    
    /** Item rarity level (0 = common, 1 = uncommon, 2 = rare, etc.) */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    int32 Rarity = 0;
    
    /** Is item favorited (prevents accidental drop/combine) */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    bool bIsFavorite = false;
    
    /** Is item stolen (affects shop prices, police interaction) */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    bool bIsStolen = false;
    
    /** Item-specific gameplay tags */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FGameplayTagContainer ItemTags;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Tool", meta = (EditCondition = "bIsTool"))
    FToolInstanceData ToolData;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Tool")
    bool bIsTool = false;

    /** Extensible custom data for item-specific needs */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    TMap<FName, float> CustomData;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Slot")
    bool bIsEquipped = false;

    /*SlotType: equippable, consumable, throwable, wearable
    *all types can go into quickslot
    *but only wearables can go into wearable slot
    *if no tag selected, will be treaded as inventory slot
    */    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FGameplayTag SlotType;

    // === CONSTRUCTOR ===
    
    FInventorySlot()
        : InstanceID(FGuid::NewGuid())
        , ItemID(NAME_None)
        , Quantity(0)
        , MaxStackSize(1)
        , Quality(1.0f)
        , Durability(1.0f)
        , Rarity(0)
        , bIsFavorite(false)
        , bIsStolen(false)
    {
    }

    // === HELPERS ===

    bool HasToolData() const{return bIsTool && ToolData.IsValid();}    
    bool IsEmpty() const { return ItemID.IsNone() || Quantity <= 0; }
    bool IsValid() const { return !ItemID.IsNone() && Quantity > 0; }
    
    int32 GetRemainingStackSpace() const
    {
        return FMath::Max(0, MaxStackSize - Quantity);
    }
    
    bool CanStackWith(const FInventorySlot& Other) const
    {
        if (IsEmpty() || Other.IsEmpty()) return false;
        if (ItemID != Other.ItemID) return false;
        if (Quantity >= MaxStackSize) return false;
        if (FMath::Abs(Quality - Other.Quality) > 0.01f) return false;
        return true;
    }

    void Clear()
    {
        InstanceID = FGuid::NewGuid();
        ItemID = NAME_None;
        Quantity = 0;
        MaxStackSize = 1;
        Quality = 1.0f;
        Durability = 1.0f;
        Rarity = 0;
        bIsFavorite = false;
        bIsStolen = false;
        ItemTags.Reset();
        CustomData.Empty();
        bIsEquipped = false;
        SlotType = FGameplayTag();
    }

    bool operator==(const FInventorySlot& Other) const
    {
        return InstanceID == Other.InstanceID;
    }
    
    bool operator!=(const FInventorySlot& Other) const
    {
        return !(*this == Other);
    }
};

/**
 * Inventory Slot Reference
 * Used for multi-selection and batch operations
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FInventorySlotReference : public FTableRowBase
{
    GENERATED_BODY()
    
    /** Which inventory type this slot belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reference")
    FGameplayTag InventoryType;
    
    /** Slot index within that inventory */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reference")
    int32 SlotIndex = -1;
    
    FInventorySlotReference()
        : SlotIndex(-1)
    {
    }
    
    FInventorySlotReference(FGameplayTag InInventoryType, int32 InSlotIndex)
        : InventoryType(InInventoryType)
        , SlotIndex(InSlotIndex)
    {
    }
    
    bool IsValid() const { return InventoryType.IsValid() && SlotIndex >= 0; }
    
    bool operator==(const FInventorySlotReference& Other) const
    {
        return InventoryType == Other.InventoryType && SlotIndex == Other.SlotIndex;
    }
    
    bool operator!=(const FInventorySlotReference& Other) const
    {
        return !(*this == Other);
    }
};


/**
 * Drop table entry for loot generation.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FDropTableEntry
{
    GENERATED_BODY()

    /** Item to drop */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
    FName ItemID = NAME_None;

    /** Drop chance (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DropChance = 1.0f;

    /** Min quantity if dropped */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "1"))
    int32 MinQuantity = 1;

    /** Max quantity if dropped */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "1"))
    int32 MaxQuantity = 1;

    /** Required tags on looter to drop */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
    FGameplayTagContainer RequiredLooterTags;

    int32 RollQuantity() const
    {
        return FMath::RandRange(MinQuantity, FMath::Max(MinQuantity, MaxQuantity));
    }

    bool RollDrop() const
    {
        return FMath::FRand() <= DropChance;
    }
};

/**
 * Item preview data for UI tooltips.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FItemPreviewData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    FName ID = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    FText DisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    FText Description;

    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    FGameplayTag Rarity;

    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    FGameplayTag ItemType;

    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    float CurrentQuality = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    int32 Value = 0;

    /** Stat key-value pairs for display */
    UPROPERTY(BlueprintReadOnly, Category = "Preview")
    TMap<FName, float> Stats;
};

/**
 * Stat comparison entry for item comparison UI.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FStatComparisonEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    FName StatName = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    FText StatDisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    float CurrentValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    float CompareValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    bool bHigherIsBetter = false;

    UPROPERTY(BlueprintReadOnly, Category = "Comparison")
    FString UnitSuffix = FString();

    /** Positive = better, Negative = worse, Zero = same */
    float GetDifference() const { return CompareValue - CurrentValue; }
    bool IsBetter() const { return GetDifference() > 0.0f; }
    bool IsWorse() const { return GetDifference() < 0.0f; }
};

/**
 * Recipe data for crafting systems.
 * Lives here because multiple plugins may need it.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FRecipeData : public FTableRowBase
{
    GENERATED_BODY()

    /** Unique recipe identifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FName RecipeID = NAME_None;

    /** Display name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FText DisplayName;

    /** Required input items (ItemID -> Quantity) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TMap<FName, int32> Inputs;

    /** Output items (ItemID -> Quantity) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TMap<FName, int32> Outputs;

    /** Required tags on crafter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FGameplayTagContainer RequiredTags;

    /** Crafting time in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe", meta = (ClampMin = "0.0"))
    float CraftTime = 0.0f;

    /** Station type tag required (e.g., Station.Workbench, Station.Furnace) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FGameplayTag StationType;

    bool IsValid() const { return !RecipeID.IsNone() && Inputs.Num() > 0 && Outputs.Num() > 0; }
};

