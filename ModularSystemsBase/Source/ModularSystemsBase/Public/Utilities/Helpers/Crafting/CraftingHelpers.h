#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTags.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "Utilities/Helpers/MSB_BlueprintFunctionLibrary_Base.h"
#include "CraftingHelpers.generated.h"

class UDataTable;
class UInventoryComponent;

/**
 * Static utilities for crafting operations.
 */
UCLASS()
class MODULARSYSTEMSBASE_API UCraftingHelpers : public UMSB_BlueprintFunctionLibrary_Base
{
    GENERATED_BODY()

public:
    
    // === RECIPE LOOKUP ===
    
    /** Get recipe data from DataTable by ID */
    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static bool GetRecipeData(const UDataTable* RecipeTable, const FName& RecipeID, FRecipeData& OutData);
    
    /** Get all recipes that produce a specific item */
    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static TArray<FName> GetRecipesForOutput(UDataTable* RecipeTable, const FName& OutputItemID);
    
    /** Get all recipes that use a specific input */
    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static TArray<FName> GetRecipesUsingInput(UDataTable* RecipeTable, const FName InputItemID);

    // === VALIDATION ===
    
    /** Check if inventory has all required inputs for recipe */
    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static bool CanCraftRecipe(UActorComponent* Inventory, const FRecipeData& Recipe);
    
    /** Check if actor has required tags for recipe */
    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static bool MeetsRecipeRequirements(AActor* Crafter, const FRecipeData& Recipe);
    
    /** Get missing inputs for recipe */
    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static TMap<FName, int32> GetMissingInputs(UActorComponent* Inventory, const FRecipeData& Recipe);

    // === FILTERING ===
    
    /** Get all craftable recipes given current inventory */
    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static TArray<FName> GetCraftableRecipes(UActorComponent* Inventory, UDataTable* RecipeTable);
    
    /** Filter recipes by station type */
    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static TArray<FName> FilterRecipesByStation(UDataTable* RecipeTable, FGameplayTag StationType);

    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static bool HasAllInputs(UInventoryComponent* Inventory);

    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static bool MeetsRequirements(AActor* Crafter);

    UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
    static TMap<FName, int32> GetMissingInputs(UInventoryComponent* Inventory);

    UFUNCTION(BlueprintPure, Category = "Crafting| Capability | Helpers")
    static bool DoesSatisfy(const FCraftingCapability& TargetCraftingCapability, const FCraftingCapability& Requirement);
    
    
    // === EXECUTION (NEW) ===

    /** Execute full craft: validate → consume inputs → produce outputs */
    UFUNCTION(BlueprintCallable, Category = "Crafting|Helpers")
    static bool ExecuteCraft(UActorComponent* Inventory, AActor* Crafter, const FRecipeData& Recipe, int32 CraftCount = 1);

    /** Consume recipe inputs from inventory */
    UFUNCTION(BlueprintCallable, Category = "Crafting|Helpers")
    static bool ConsumeInputs(UActorComponent* Inventory, const FRecipeData& Recipe, int32 CraftCount = 1);

    /** Produce recipe outputs to inventory */
    UFUNCTION(BlueprintCallable, Category = "Crafting|Helpers")
    static bool ProduceOutputs(UActorComponent* Inventory, const FRecipeData& Recipe, float OutputQuality, float OutputDurability, int32 CraftCount = 1);

    /** Calculate output quality based on crafter skill and station */
    UFUNCTION(BlueprintPure, Category = "Crafting|Helpers")
    static float CalculateOutputQuality(AActor* Crafter, AActor* Station, const FRecipeData& Recipe);

    /** Calculate output durability based on crafter skill */
    UFUNCTION(BlueprintPure, Category = "Crafting|Helpers")
    static float CalculateOutputDurability(AActor* Crafter, const FRecipeData& Recipe);
    
    UFUNCTION(BlueprintPure, Category = "Crafting|Helpers")
    static int32 UCraftingHelpers::GetCapabilityLevel(const FCraftingCapabilitySet& Capabilities, const FGameplayTag& Tag);
    
    UFUNCTION(BlueprintPure, Category = "Crafting | Capability| Helpers")
    static int32 GetLevel(const FCraftingCapability& Capabilities, const FGameplayTag& Tag); 
    
    UFUNCTION(BlueprintPure, Category = "Crafting | CapabilitySet | Helpers")
    static bool DoesSatisfyAll(const FCraftingCapabilitySet& TargetCraftingCapabilities,
        const TArray<FCraftingCapability>& Requirements);
    
    UFUNCTION(BlueprintPure, Category = "Crafting | CapabilitySet | Helpers")
    static void AddOrUpdate(FCraftingCapabilitySet& CapabilitySet, FGameplayTag Tag, int32 Level);
    
    UFUNCTION(BlueprintPure, Category = "Crafting | CapabilitySet | Helpers")
    static void Merge(FCraftingCapabilitySet& Target, const FCraftingCapabilitySet& Other);
    
   
};



