#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ProgressionHelpers.generated.h"

UCLASS()
class MODULARSYSTEMSBASE_API UProgressionHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // === XP & LEVELING - GENERIC FUNCTIONS ===

    /** Total XP required to reach a level (Level 2 = 50, Level 3 = 150, etc.) */
    UFUNCTION(BlueprintPure, Category = "Progression")
    static int32 GetXPRequiredForLevel(int32 TargetLevel);

    /** XP cost to level up from current level, scaled by tier */
    UFUNCTION(BlueprintPure, Category = "Progression")
    static int32 GetLevelUpCost(int32 CurrentLevel, int32 Tier, float TierMultiplier = 1.5f);

    /** Calculate XP gained from crafting an item */
    UFUNCTION(BlueprintPure, Category = "Progression")
    static int32 CalculateCraftingXP(int32 RecipeComplexity, int32 Quantity, float QualityRatio, float MiniGameScore, int32 ItemRarity);
    
    
    // === STAT MULTIPLIERS ===
    
    /** Get efficiency multiplier (affects quality) */
    UFUNCTION(BlueprintPure, Category = "Progression|Stats")
    static float GetEfficiencyMultiplier(int32 Tier, int32 Level);
    
    /** Get speed multiplier (lower = faster, 1.0 = baseline) */
    UFUNCTION(BlueprintPure, Category = "Progression|Stats")
    static float GetSpeedMultiplier(int32 Tier, int32 Level);
    
    /** Get capacity (slots, storage, etc.) */
    UFUNCTION(BlueprintPure, Category = "Progression|Stats")
    static int32 GetCapacity(int32 Tier, int32 Level);
    
    /** Get quality bonus multiplier */
    UFUNCTION(BlueprintPure, Category = "Progression|Stats")
    static float GetQualityBonus(int32 Tier, int32 Level);
    
    /** Get speed bonus multiplier */
    UFUNCTION(BlueprintPure, Category = "Progression|Stats")
    static float GetSpeedBonus(int32 Tier, int32 Level);
    
    // === DURABILITY ===
    
    /** Calculate max durability from tier/level */
    UFUNCTION(BlueprintPure, Category = "Progression|Durability")
    static float GetMaxDurability(int32 Tier, int32 Level, float BaseDurability);
    
    /** Calculate durability loss per use */
    UFUNCTION(BlueprintPure, Category = "Progression|Durability")
    static float GetDurabilityLossPerUse(int32 Tier, int32 Level, float BaseWear);
        
    /** Calculate durability as percentage (0-100) */
    UFUNCTION(BlueprintPure, Category = "Progression|Durability")
    static float GetDurabilityPercent(float CurrentDurability, int32 Tier, int32 Level, float BaseDurability);
    
    // === XP PROGRESSION ===
    
    /** Check if can level up with current XP */
    UFUNCTION(BlueprintPure, Category = "Progression|XP")
    static bool CanLevelUp(int32 CurrentXP, int32 CurrentLevel, int32 Tier, float TierMultiplier = 1.5f);
    
    /** Calculate XP progress as percentage (0.0 - 1.0) */
    UFUNCTION(BlueprintPure, Category = "Progression|XP")
    static float GetXPProgressPercent(int32 CurrentXP, int32 CurrentLevel, int32 Tier, float TierMultiplier = 1.5f);



    // === STAT MULTIPLIERS ===
    
    /** Speed multiplier (lower = faster crafting) */




    // === DURABILITY ===
    
    /** Calculate uses until broken */
    UFUNCTION(BlueprintPure, Category = "Progression")
    static int32 GetUsesUntilBroken(int32 Tier, int32 Level, float BaseDurability = 100.0f, float BaseWear = 1.0f);

    /*--- FSTATIONINSTANCEDATA HELPERS ---*/
    UFUNCTION(BlueprintPure, Category = "Progression")
    static void ApplyWear(FStationInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static void Repair(FStationInstanceData& Data, float Amount);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static void RepairFull(FStationInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static bool IsBroken(const FStationInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static float GetDurabiltiyPercent(const FStationInstanceData& Data);

        //---OPERATIONS---//

    //adds exp, on level up: returns 1.0f if true 0.0f if false
    UFUNCTION(BlueprintPure, Category = "Progression")
    static float AddXP( FStationInstanceData& Data, const int32& Amount);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static float RemoveXP( FStationInstanceData& Data, const int32& Amount);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static float SetXP(FStationInstanceData& Data, const int32 NewXPValue);

    /*--- Comes frome the old struct functions ---*/
    
    // Stat getters - use UProgressionHelpers formulas
    UFUNCTION(BlueprintPure, Category = "Progression")
    static float GetMaxDurability(const FStationInstanceData& Data);
    
    UFUNCTION(BlueprintPure, Category = "Progression")
    static float GetDurabilityLossPerUse(const FStationInstanceData& Data); 
    
    UFUNCTION(BlueprintPure, Category = "Progression")
    static float GetEfficiencyMultiplier(const FStationInstanceData& Data);
    
    UFUNCTION(BlueprintPure, Category = "Progression")
    static float GetSpeedMultiplier(const FStationInstanceData& Data);
   
    UFUNCTION(BlueprintPure, Category = "Progression")
    static int32 GetCapacity(const FStationInstanceData& Data); 

    UFUNCTION(BlueprintPure, Category = "Progression")
    static int32 GetLevelUpCost(const FStationInstanceData& Data);
    
    UFUNCTION(BlueprintPure, Category = "Progression")
    static float GetDurabilityPercent(const FStationInstanceData& Data);
    
    UFUNCTION(BlueprintPure, Category = "Progression")
    static bool TryLevelUp(int32 Level = 1 , int32 Tier =1, int32 TierMultiplier =1.5,  int32 CurrentXP);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static void AddXP(FStationXPPool& StationXPPool,int32 Amount);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static bool SpendXP(FStationXPPool& StationXPPool, int32 Amount);

    /*--- FTOOLINSTANCEDATA HELPERS ---*/

    UFUNCTION(BlueprintPure, Category = "Progression")
    static float GetMaxDurability(const FToolInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static float GetDurabilityLossPerUse(const FToolInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static float GetDurabilityPercent(const FToolInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static int32 GetLevelUpCost(const FToolInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static void ApplyWear(FToolInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static void Repair(FToolInstanceData& Data, float Amount);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static void RepairFull(FToolInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static bool IsBroken(const FToolInstanceData& Data);

    UFUNCTION(BlueprintPure, Category = "Progression")
    static float AddXP(FToolInstanceData& Data, const int32& Amount);

};