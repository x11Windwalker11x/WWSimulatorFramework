#include "Utilities/Helpers/Crafting/CraftingHelpers.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "Interfaces/ModularInventorySystem/InventoryInterface.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Interfaces/ModularInventorySystem/CraftingInterface.h"
#include "Logging/InteractableInventoryLogging.h"


bool UCraftingHelpers::GetRecipeData(const UDataTable* RecipeTable , const FName& RecipeID, FRecipeData& OutData)
{
	if (!RecipeTable || !RecipeID.IsValid()) return false;
	FRecipeData* RecipeData = RecipeTable->FindRow<FRecipeData>(RecipeID, TEXT("Finding Recipe Data"));
	if (!RecipeData || !RecipeData->IsValid()) return false;
    OutData = *RecipeData; 
	return true;
}

TArray<FName> UCraftingHelpers::GetRecipesForOutput(UDataTable* RecipeTable, const FName& OutputItemID)
{
	TArray<FName> Result;
	Result.Empty();
	if (!RecipeTable || !OutputItemID.IsValid()) return Result;

	
	for (const FName& RowName : RecipeTable->GetRowNames())
	{
		FRecipeData* RecipeData = RecipeTable->FindRow<FRecipeData>(RowName, TEXT("Finding Recipe Data"));
		if (!RecipeData || !RecipeData->IsValid()) return Result;
		if (RecipeData->Outputs.Contains(OutputItemID)) Result.AddUnique(RowName);
	}
	return Result;
}

TArray<FName> UCraftingHelpers::GetRecipesUsingInput(UDataTable* RecipeTable, const FName InputItemID)
{
	TArray<FName> Result;
	Result.Empty();
	
	if (!RecipeTable || !InputItemID.IsValid()) return Result;
	for (const FName& RowName : RecipeTable->GetRowNames())
	{
		FRecipeData* RecipeData = RecipeTable->FindRow<FRecipeData>(InputItemID, TEXT("Find Recipe Data"));
		if (!RecipeData) return Result;
		if (RecipeData->Inputs.Contains(RowName)) Result.AddUnique(RowName);
	}
	return Result;
	
}

bool UCraftingHelpers::CanCraftRecipe(UActorComponent* Inventory, const FRecipeData& Recipe)
{
	if (!Inventory || !Recipe.IsValid()) return false;
	if (UActorComponent* InventoryComponent = GetInventoryComponent(Inventory))
	for (const auto& Input : Recipe.Inputs)
	{
		int32 CurrentCount = IInventoryInterface::Execute_GetItemCount(InventoryComponent,Input.Key);
		if (CurrentCount < Input.Value)
			return false;
	}
	return true;
}

bool UCraftingHelpers::MeetsRecipeRequirements(AActor* Crafter, const FRecipeData& Recipe)
{
	if (!Crafter || !Recipe.IsValid()) return false;
	if (Recipe.RequiredTags.IsEmpty()) return true;
	
	FGameplayTagContainer CrafterTags;
	if (Crafter->GetClass()->ImplementsInterface(UCraftingInterface::StaticClass()))
	{
		if (UActorComponent* CrafterComponent = ICraftingInterface::Execute_GetCraftingComponent(Crafter))
		{
			CrafterTags = ICraftingInterface::Execute_GetCrafterTags(Crafter);
		}
	}
	else UE_LOG(LogDebugSystem, Warning, TEXT("UCraftingHelpers::MeetsRecipeRequirements, crafter actor does not implement interface."));
	
	return CrafterTags.HasAll(Recipe.RequiredTags);
}

TMap<FName, int32> UCraftingHelpers::GetMissingInputs(UActorComponent* Inventory, const FRecipeData& Recipe)
{
	TMap<FName, int32> MissingInputs;
	if (!Recipe.IsValid() || !Inventory) return MissingInputs;

	if (UActorComponent* InventoryComponent = GetInventoryComponent(Inventory))
	{
		for (const auto Input : Recipe.Inputs)
		{
			int32 CurrentCount = IInventoryInterface::Execute_GetItemCount(InventoryComponent, Input.Key);
			int32 Needed = Input.Value - CurrentCount;
			if (Needed >0)
			{
				MissingInputs.Add(Input.Key, Needed);
			}
		}
	}
	return MissingInputs;
}

TArray<FName> UCraftingHelpers::GetCraftableRecipes(UActorComponent* Inventory, UDataTable* RecipeTable)
{
	TArray<FName> Result;
	if (!RecipeTable || !Inventory) return Result;
	if (UActorComponent* InventoryComponent = GetInventoryComponent(Inventory))
	{
		for (const FName& RowName : RecipeTable->GetRowNames())
		{
			FRecipeData* RecipeRow = RecipeTable->FindRow<FRecipeData>(RowName, TEXT("Finding Recipe Data"));
			if (!RecipeRow || !RecipeRow->IsValid()) return Result;
			if (CanCraftRecipe(Inventory, *RecipeRow))
			{
				Result.AddUnique(RowName);
			}
			return Result;
		}
	}
	return Result;
}

TArray<FName> UCraftingHelpers::FilterRecipesByStation(UDataTable* RecipeTable, FGameplayTag StationType)
{
	TArray<FName> Result;
	if (!RecipeTable || !StationType.IsValid()) return Result;
	for (const FName& RowName : RecipeTable->GetRowNames())
	{
		FRecipeData* RecipeRow = RecipeTable->FindRow<FRecipeData>(RowName, TEXT("FindingRecipes"));
		if (!RecipeRow || !RecipeRow->IsValid()) return Result;
		if (StationType.MatchesTagExact(RecipeRow->StationType))
		{
			Result.AddUnique(RecipeRow->StationType.GetTagName());
		}
	}
	return Result;
}

bool UCraftingHelpers::HasAllInputs(UInventoryComponent* Inventory)
{
	if (!Inventory) return false;
	
}

bool UCraftingHelpers::MeetsRequirements(AActor* Crafter)
{
	if (!Crafter) return false;
	if (Crafter->GetClass()->ImplementsInterface(UCraftingInterface::StaticClass()))
	{
		ICraftingInterface::Execute_GetCrafterTags(Crafter);
	}
}


bool UCraftingHelpers::DoesSatisfy(const FCraftingCapability& TargetCraftingCapability, const FCraftingCapability& Requirement)
{
	return TargetCraftingCapability.CapabilityTag.MatchesTag(Requirement.CapabilityTag) 
		&& TargetCraftingCapability.Level >= Requirement.Level;
}


/*---CRAFTINGCAPABILITYSET USTRUCT HELPERS ---*/

int32 UCraftingHelpers::GetLevel(const FCraftingCapability& Capabilities, const FGameplayTag& Tag)
{
	for (const FCraftingCapability& Cap : Capabilities)
	{
		if (Cap.CapabilityTag.MatchesTag(Tag))
		{
			return Cap.Level;
		}
	}
	return 0;
}

bool UCraftingHelpers::DoesSatisfyAll(const FCraftingCapabilitySet& TargetCraftingCapabilities,
	const TArray<FCraftingCapability>& Requirements)
{
	if (TargetCraftingCapabilities.Capabilities.Num() == 0 || Requirements.IsEmpty()) return false;
	
	/*Strat: create gameplaytag container tags for each and make a querry to
	 *get rid of complicated for each loop checks.
	*/
	FGameplayTagContainer TargetRequirementAttributes = FGameplayTagContainer();
	FGameplayTagContainer RequirementContainer = FGameplayTagContainer();
	
	for (const FCraftingCapability& TargetCapability : TargetCraftingCapabilities.Capabilities)
	{
		TargetRequirementAttributes.AddTag(TargetCapability.CapabilityTag);
	}

	for (const FCraftingCapability& RequiredCapability : Requirements)
	{
		RequirementContainer.AddTag(RequiredCapability.CapabilityTag);
	}

	if (TargetRequirementAttributes.HasAllExact(RequirementContainer))
	{
		return true;
	}
	return false;
}

void UCraftingHelpers::AddOrUpdate(FCraftingCapabilitySet& CapabilitySet, FGameplayTag Tag, int32 Level)
{
	for (FCraftingCapability& Cap : CapabilitySet)
	{
		if (Cap.CapabilityTag.MatchesTagExact(Tag))
		{
			Cap.Level = Level;
			return;
		}
	}
	CapabilitySet.Capabilities.Add(FCraftingCapability(Tag, Level));
}

void UCraftingHelpers::Merge(FCraftingCapabilitySet& Target, const FCraftingCapabilitySet& Other)
{
	if (Target.Capabilities.Num()==0 || Other.Capabilities.Num()==0) return;
		
	for (const FCraftingCapability& TargetCap : Target.Capabilities)
	{
		for (const FCraftingCapability& OtherCap : Other.Capabilities)
		{
			int32 CurrentLevel = GetLevel(TargetCap, OtherCap.CapabilityTag);
			if (OtherCap.Level > CurrentLevel)
			{
				AddOrUpdate(Target, OtherCap.CapabilityTag, OtherCap.Level);
			}
		}

	}
}


bool UCraftingHelpers::ExecuteCraft(UActorComponent* Inventory, AActor* Crafter, const FRecipeData& Recipe, int32 CraftCount)
{
	if (!Inventory || !Recipe.IsValid() || CraftCount <= 0) return false;

	// Validate can craft
	if (!CanCraftRecipe(Inventory, Recipe)) return false;
	if (!MeetsRecipeRequirements(Crafter, Recipe)) return false;

	// Calculate quality/durability
	float OutputQuality = CalculateOutputQuality(Crafter, nullptr, Recipe);
	float OutputDurability = CalculateOutputDurability(Crafter, Recipe);

	// Consume inputs
	if (!ConsumeInputs(Inventory, Recipe, CraftCount)) return false;

	// Produce outputs
	if (!ProduceOutputs(Inventory, Recipe, OutputQuality, OutputDurability, CraftCount)) return false;

	return true;
}

bool UCraftingHelpers::ConsumeInputs(UActorComponent* Inventory, const FRecipeData& Recipe, int32 CraftCount)
{
	if (!Inventory || !Recipe.IsValid()) return false;

	UActorComponent* InvComp = GetInventoryComponent(Inventory);
	if (!InvComp) return false;

	// Consume each input
	for (const auto& Input : Recipe.Inputs)
	{
		int32 TotalToConsume = Input.Value * CraftCount;
		bool bConsumed = IInventoryInterface::Execute_TryConsumeItem(InvComp, Input.Key, TotalToConsume);
		if (!bConsumed)
		{
			UE_LOG(LogTemp, Warning, TEXT("ExecuteCraft: Failed to consume %s x%d"), *Input.Key.ToString(), TotalToConsume);
			return false;
		}
	}

	return true;
}
bool UCraftingHelpers::ProduceOutputs(UActorComponent* Inventory, const FRecipeData& Recipe, float OutputQuality, float OutputDurability, int32 CraftCount)
{
	if (!Inventory || !Recipe.IsValid()) return false;

	UActorComponent* InvComp = GetInventoryComponent(Inventory);
	if (!InvComp) return false;

	// Add each output
	for (const auto& Output : Recipe.Outputs)
	{
		int32 TotalToProduce = Output.Value * CraftCount;
		bool bAdded = IInventoryInterface::Execute_AddItem(InvComp, Output.Key, TotalToProduce, OutputQuality, OutputDurability, false);
		if (!bAdded)
		{
			UE_LOG(LogTemp, Warning, TEXT("ExecuteCraft: Failed to add %s x%d"), *Output.Key.ToString(), TotalToProduce);
			// Note: Inputs already consumed - may need rollback in production
			return false;
		}
	}

	return true;
}

float UCraftingHelpers::CalculateOutputQuality(AActor* Crafter, AActor* Station, const FRecipeData& Recipe)
{
	float BaseQuality = 1.0f;

	// Get crafter skill bonus
	if (Crafter && Crafter->GetClass()->ImplementsInterface(UCraftingInterface::StaticClass()))
	{
		// Future: Query crafter level for this recipe type
		// int32 CrafterLevel = ICraftingInterface::Execute_GetCapabilityLevel(Crafter, Recipe.StationType);
		// BaseQuality += CrafterLevel * 0.05f;  // 5% per level
	}

	// Get station bonus (future)
	// if (Station) { ... }

	return FMath::Clamp(BaseQuality, 0.0f, 1.0f);
}

float UCraftingHelpers::CalculateOutputDurability(AActor* Crafter, const FRecipeData& Recipe)
{
	// Crafted items start at full durability
	// Future: Crafter skill could affect this
	return 1.0f;
}

int32 UCraftingHelpers::GetCapabilityLevel(const FCraftingCapabilitySet& Capabilities, const FGameplayTag& Tag)
{
	for (const FCraftingCapability& Cap : Capabilities.Capabilities)
	{
		if (Cap.CapabilityTag.MatchesTag(Tag))
		{
			return Cap.Level;
		}
	}
	return 0;
}
