#include "InventorySaveModule.h"

UInventorySaveModule::UInventorySaveModule()
{
	ModuleName = TEXT("InventorySaveModule");
	ModuleVersion = CURRENT_MODULE_VERSION;
}

FPlayerInventoryData& UInventorySaveModule::GetOrCreatePlayerInventory(const FString& PlayerID)
{
	if (!PlayerInventories.Contains(PlayerID))
	{
		FPlayerInventoryData NewInventory;
		NewInventory.PlayerID = PlayerID;
		PlayerInventories.Add(PlayerID, NewInventory);
	}

	return PlayerInventories[PlayerID];
}

bool UInventorySaveModule::HasPlayerData(const FString& PlayerID) const
{
	return PlayerInventories.Contains(PlayerID);
}

bool UInventorySaveModule::RemovePlayerData(const FString& PlayerID)
{
	return PlayerInventories.Remove(PlayerID) > 0;
}

int32 UInventorySaveModule::GetTotalItemCount() const
{
	int32 TotalCount = 0;
    
	for (const auto& Pair : PlayerInventories)
	{
		TotalCount += Pair.Value.InventoryItems.Num();
	}
    
	TotalCount += SharedStorage.Num();
    
	return TotalCount;
}

TArray<FString> UInventorySaveModule::GetAllPlayerIDs() const
{
	TArray<FString> PlayerIDs;
	PlayerInventories.GetKeys(PlayerIDs);
	return PlayerIDs;
}

void UInventorySaveModule::ClearData()
{
	PlayerInventories.Empty();
	SharedStorage.Empty();
}

bool UInventorySaveModule::ValidateData_Implementation()
{
	for (auto& Pair : PlayerInventories)
	{
		FPlayerInventoryData& InvData = Pair.Value;
        
		InvData.InventoryItems.RemoveAll([](const FItemSaveData& Item) {
			return Item.ItemID == NAME_None || Item.Quantity <= 0;
		});
	}

	return true;
}

void UInventorySaveModule::MigrateData_Implementation(int32 FromVersion, int32 ToVersion)
{
}