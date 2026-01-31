// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CrafterComponent_Master.h"
#include "Debug/DebugSubsystem.h"
#include "Subsystems/DataReloadSubsystem.h"
#include "Utilities/Helpers/Crafting/CraftingHelpers.h"
#include "Utilities/Helpers/Progression/ProgressionHelpers.h"
#include "Utilities/JsonReader/CrafterJsonReader.h"


// Sets default values for this component's properties
UCrafterComponent_Master::UCrafterComponent_Master()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

FName UCrafterComponent_Master::GetCrafterID() const
{
	return CrafterInstanceData->ID;
}


// Called when the game starts
void UCrafterComponent_Master::BeginPlay()
{
	Super::BeginPlay();

	CrafterInstanceData->ID = ID;

	SetupConfig<&UCrafterJsonReader::GetCrafterDataTable, FCrafterData, FCrafterInstanceData>(*CrafterInstanceData, CrafterInstanceData->ID);
	// Initialize capabilities from instance data
	Capabilities = CrafterInstanceData->CurrentCapabilities;
	
}

void UCrafterComponent_Master::MapCustomData(FTableRowBase* DataTableRow, FTableRowBase* DataTableRowInstance)
{
	//No need to call super. the function is already empty
	//Super::MapCustomData(DataTableRow, DataTableRowInstance);

	FCrafterData* TableRow = static_cast<FCrafterData*>(DataTableRow);
	CrafterInstanceData = static_cast<FCrafterInstanceData*>(DataTableRowInstance);

	if (!TableRow || !CrafterInstanceData) return;

	CrafterInstanceData->ID = TableRow->ID;
	CrafterInstanceData->DisplayName = TableRow->DisplayName;
	CrafterInstanceData->BaseTags = TableRow->BaseTags;
	CrafterInstanceData->StartingCapabilitySet = TableRow->StartingCapabilitySet;

}


FGameplayTagContainer UCrafterComponent_Master::GetCrafterTagsPure() const
{
	FGameplayTagContainer Tags;
	for (const FCraftingCapability& Cap : CrafterInstanceData->CurrentCapabilities)
	{
		Tags.AddTag(Cap.CapabilityTag);
	}
	return Tags;
}

FCraftingCapabilitySet UCrafterComponent_Master::GetCapabilities() const
{
	return Capabilities;
}

int32 UCrafterComponent_Master::GetCapabilityLevel(FGameplayTag LevelTag) const
{
	// for (const FCraftingCapability& Cap : Capabilities.Capabilities)
	// {
	// 	if (Cap.CapabilityTag == LevelTag)
	// 	{
	// 		return Cap.Level;
	// 	}
	// }
	return UCraftingHelpers::GetCapabilityLevel(Capabilities, LevelTag);
}

bool UCrafterComponent_Master::HasCapability(FGameplayTag Tag, int32 MinLevel) const
{
	const FCraftingCapability* FoundCap = Capabilities.Capabilities.FindByPredicate([&](const FCraftingCapability& Item) {
		return Item.CapabilityTag == Tag;
	});

	// 2. Check if it exists AND if its level meets the requirement
	// If FoundCap is null, the player doesn't have the capability at all
	return FoundCap && FoundCap->Level >= MinLevel;}

void UCrafterComponent_Master::AddCapability(FGameplayTag Tag, int32 Level)
{
	UCraftingHelpers::AddOrUpdate(Capabilities, Tag, Level);
}

void UCrafterComponent_Master::RemoveCapability(FGameplayTag Tag)
{
	Capabilities.Capabilities.RemoveAll([Tag](const FCraftingCapability& Cap)
	{
		return Cap.CapabilityTag.MatchesTagExact(Tag);
	});
}

void UCrafterComponent_Master::SetCapabilityLevel(FGameplayTag Tag, int32 NewLevel)
{
	UCraftingHelpers::AddOrUpdate(Capabilities, Tag, NewLevel);
}

void UCrafterComponent_Master::MergeCapabilities(const FCraftingCapabilitySet& Other)
{
	UCraftingHelpers::Merge(Capabilities,Other);
}

void UCrafterComponent_Master::ClearCapabilities()
{
	Capabilities.Capabilities.Empty();
}

UActorComponent* UCrafterComponent_Master::GetCraftingComponent_Implementation()
{
	return this;
}

FName UCrafterComponent_Master::GetCraftableID_Implementation(AActor* Crafter) const
{
	return GetCrafterID();
}


FGameplayTagContainer UCrafterComponent_Master::GetCrafterTags_Implementation() const
{
	return GetCrafterTagsPure();
}

bool UCrafterComponent_Master::HasCraftingCapability_Implementation(FGameplayTag CapabilityTag, int32 MinLevel)
{
    return HasCapability(CapabilityTag, MinLevel);
}
