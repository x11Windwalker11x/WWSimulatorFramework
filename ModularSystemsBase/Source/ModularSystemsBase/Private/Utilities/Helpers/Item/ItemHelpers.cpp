#include "Utilities/Helpers/Item/ItemHelpers.h"

#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "Debug/DebugSubsystem.h"
#include "Engine/DataTable.h"

bool UItemHelpers::GetItemData(UDataTable* ItemTable, FName ItemID, FItemData& OutData)
{
	if (!ItemTable || ItemID.IsNone())
	{
		return false;
	}

	FItemData* FoundRow = ItemTable->FindRow<FItemData>(ItemID, TEXT("GetItemData"));
	if (!FoundRow)
	{
		return false;
	}

	OutData = *FoundRow;
	return true;
}


TArray<FName> UItemHelpers::FilterItemsByTag(UDataTable* ItemTable, FGameplayTag FilterTag)
{
	TArray<FName> Result;

	if (!ItemTable || FilterTag.IsValid()) return Result;

	//Get all row names
	TArray<FName> RowNames = ItemTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FItemData* Row = ItemTable->FindRow<FItemData>(RowName, TEXT("FilterItemsByTArray"));
		if (Row && Row->ObjectTags.HasTag(FilterTag))
		{
			Result.Add(RowName);
		}
	}

	return Result;
}

bool UItemHelpers::DoesItemExist(UDataTable* ItemTable, FName ItemID)
{
	if (!ItemTable || ItemID.IsNone()) return false;

	return ItemTable->FindRow<FItemData>(ItemID, TEXT("GetItemData")) != nullptr;
}

TArray<FName> UItemHelpers::GetAllItemIDs(UDataTable* ItemTable)
{
	if (!ItemTable) return TArray<FName>();
	return ItemTable->GetRowNames();	
}

bool UItemHelpers::CanStackItems(const FInventorySlot& ExistingSlot, FName NewItemID, int32 Quantity,
	UDataTable* ItemTable)
{
	if (!ItemTable || NewItemID.IsNone()) return false;

	if (ExistingSlot.ItemID != NewItemID) return false;
	
	if (ExistingSlot.Quantity <=0 || Quantity <= 0) return false;

	FItemData* NewItemData = ItemTable->FindRow<FItemData>(NewItemID, TEXT("Getting Item Data to check if stackable"));

	if (NewItemData->GetIsStackable()) return false;

	int32 SpaceRemaining = ExistingSlot.MaxStackSize - ExistingSlot.Quantity;
	return SpaceRemaining >= Quantity;
	
}

int32 UItemHelpers::GetRemainingStackSpace(const FInventorySlot& Slot, UDataTable* ItemTable)
{
	if (!ItemTable || Slot.ItemID.IsNone())
	{
		UE_LOG(LogDebugSystem, Log, TEXT("GetRemainingStackSpace: ItemTable or Slot is null"));
		return 0;
	}
	FItemData* ItemtoCheckAganist = ItemTable->FindRow<FItemData>(Slot.ItemID, TEXT("GetRemainingStackSpace"));
	if (!ItemtoCheckAganist)
	{
		UE_LOG(LogDebugSystem, Log, TEXT("No valid item to check: GetRemainingStackSpace"));
		return 0;
	}
	return Slot.MaxStackSize - Slot.Quantity;	
}

FInventorySlot UItemHelpers::CreateSlotFromItem(FName ItemID, int32 Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0)
	{
		UE_LOG(LogDebugSystem, Log, TEXT("CreateSlotFromItem: ItemID is invalid or invalid quantity"));
		return FInventorySlot();
	}
	FInventorySlot NewSlot;
	NewSlot.ItemID = ItemID;
	NewSlot.Quantity = Quantity;
	NewSlot.InstanceID = FGuid::NewGuid();
	return NewSlot;
}

int32 UItemHelpers::CalculateSlotValue(const FInventorySlot& Slot, UDataTable* ItemTable)
{
	if (Slot.ItemID.IsNone() || !ItemTable) return 0;
	FItemData* ItemData = ItemTable->FindRow<FItemData>(Slot.ItemID, TEXT("GetItemData"));
	//TODO: Implement the modifications from the economy module later.
	return ItemData->BaseValue * Slot.Quantity;
	
}

float UItemHelpers::CalculateSlotWeight(const FInventorySlot& Slot, UDataTable* ItemTable)
{
	if (!Slot.ItemID.IsValid() || !ItemTable) return 0;
	FItemData* ItemData = ItemTable->FindRow<FItemData>(Slot.ItemID, TEXT("GetItemData"));
	//TODO: implement weight calculation logic here.
	return ItemData->Weight * Slot.Quantity;
		
}

int32 UItemHelpers::GetItemValue(const FName& ItemID, UDataTable* ItemTable)
{
	if (!ItemID.IsValid() || !ItemTable) return 0;
	FItemData* ItemData = ItemTable->FindRow<FItemData>(ItemID, TEXT("GetItemData"));
	return ItemData->BaseValue;
}

float UItemHelpers::GetItemWeight(const FName& ItemID, UDataTable* ItemTable)
{
	if (!ItemID.IsValid() || !ItemTable) return 0;
	FItemData* ItemData = ItemTable->FindRow<FItemData>(ItemID, TEXT("GetItemData"));
	return ItemData->Weight;
}