#include "Components/DataTableOverwritableComponent.h"
#include "Debug/DebugSubsystem.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Utilities/Helpers/Crafting/CraftingHelpers.h"



/*TODO: abstarct one layer to make it applicable all components
 * using data table to get their configuration
 */
template <auto TFunction, typename TRowStruct, typename TRowInstanceStruct>
void UDataTableOverwritableComponent::SetupConfig(TRowInstanceStruct& DestInstance, FName ID)
{
	if (!ID.IsValid()) return;
	
	//Get DataTable
	UDataTable* DataTable = TFunction();
	if (!DataTable) return;

	//Find row
	TRowStruct DataRow = DataTable->FindRow<TRowStruct>(ID, TEXT("Finding DataRow ID:%s..."), *ID.ToString());
	if (!DataRow)
	{
		FString ErrorMessage = FString::Printf(TEXT("CrafterComponent: could not find the data row for item id %s in owner: %s"), *ID.ToString(), *GetOwner()->GetName());
		UDebugSubsystem::PrintDebug(this, FWWTagLibrary::Debug_InventoryInteraction(), ErrorMessage, false, EDebugVerbosity::Critical);
	}
	/*
	 *Important: TRowStruct and the derived struct, (FWorldObjectData for now)
	 *must inherit from a generic FTableRowBase struct for that to work.
	 *that custom struct (FWorldObjectData as placeholder), has to be
	 *replaced by another custom ustruct that has all the common members of
	 *item data struct, crafter data struct, recipes data struct, 
	 *interactable data struct and character data struct
	 *gotta think about the dynamic and static data as well. 
	 */
	FWorldObjectData* SourceBase = static_cast<FWorldObjectData*>(&DataRow);
	FWorldObjectData* TargetBase = static_cast<FWorldObjectData*>(&DestInstance);

	//Map generic data
	TargetBase->ID = SourceBase->ID;
	TargetBase->DisplayName = SourceBase->DisplayName;
	TargetBase->Description = SourceBase->Description;

	// Call the mapper for specific data
	MapCustomData(DataRow, &DestInstance);
	/*
	 *Gotta implement this method to crafter component master
	 *Too specific to implement here
	 */
	// for (const FCraftingCapabilitySet& CapabilitySet : DataRow->StartingCapabilitySet)
	// {
	// 	for (const FCraftingCapability& Capability : CapabilitySet)
	// 	{
	// 		UCraftingHelpers::AddOrUpdate(CrafterInstanceData.CurrentCapabilities, Capability.CapabilityTag, Capability.Level);
	// 	}
	// }

	//Already set in this func.
	// CrafterInstanceData.CrafterDisplayName = DataRow->CrafterDisplayName;
	// CrafterInstanceData.CrafterID = DataRow->CrafterID;
}

UDataTableOverwritableComponent::UDataTableOverwritableComponent()
{
}

void UDataTableOverwritableComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDataTableOverwritableComponent::MapCustomData(FTableRowBase* DataTableRow, FTableRowBase* DataTableRowInstance)
{
	// Nothing here, it is designed for children.
}
