#pragma once

#include "CoreMinimal.h"
#include "MasterSaveGame.h"
#include "Actors/Interactables/Item/Data/ItemData.h"
#include "Actors/Interactables/Item/Data/ItemData_Dynamic.h"
#include "InventorySaveModule.generated.h"

/**
 * Represents a single saved item instance with quantity
 */
USTRUCT(BlueprintType)
struct FItemSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 SlotIndex = -1;

	// Optional: Store additional runtime data (durability, modifications, etc.)
	UPROPERTY(SaveGame, BlueprintReadWrite)
	TMap<FName, float> CustomFloatData;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	TMap<FName, FString> CustomStringData;

	FItemSaveData()
		: ItemID(NAME_None)
		, Quantity(1)
		, SlotIndex(-1)
	{}

	FItemSaveData(FName InItemID, int32 InQuantity, int32 InSlotIndex)
		: ItemID(InItemID)
		, Quantity(InQuantity)
		, SlotIndex(InSlotIndex)
	{}
};

/**
 * Container for a player's inventory data
 */
USTRUCT(BlueprintType)
struct FPlayerInventoryData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	TArray<FItemSaveData> InventoryItems;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float CurrentWeight = 0.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float MaxWeight = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 MaxSlots = 20;

	FPlayerInventoryData()
		: PlayerID(TEXT(""))
		, CurrentWeight(0.0f)
		, MaxWeight(100.0f)
		, MaxSlots(20)
	{}
};

/**
 * Modular save data component for inventory system
 * This gets embedded in the MasterSaveGame object
 */
UCLASS(BlueprintType)
class MODULARSAVEGAMESYSTEM_API UInventorySaveModule : public UModularSaveData
{
	GENERATED_BODY()

public:
	UInventorySaveModule();

	// ========== Inventory Data ==========
	
	// Store inventory data for each player (supports local multiplayer)
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Inventory")
	TMap<FString, FPlayerInventoryData> PlayerInventories;

	// Optional: Global data (shared across all players)
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Inventory")
	TArray<FItemSaveData> SharedStorage;

	// ========== Utility Functions ==========
	
	/**
	 * Get or create player inventory
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Save")
	FPlayerInventoryData& GetOrCreatePlayerInventory(const FString& PlayerID);

	/**
	 * Check if player data exists
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory Save")
	bool HasPlayerData(const FString& PlayerID) const;

	/**
	 * Remove specific player data
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Save")
	bool RemovePlayerData(const FString& PlayerID);

	/**
	 * Get total item count across all players
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory Save")
	int32 GetTotalItemCount() const;

	/**
	 * Get all player IDs
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory Save")
	TArray<FString> GetAllPlayerIDs() const;

	// ========== UModularSaveData Overrides ==========
	
	virtual void ClearData() override;
	virtual bool ValidateData_Implementation() override;
	virtual void MigrateData_Implementation(int32 FromVersion, int32 ToVersion) override;

private:
	static const int32 CURRENT_MODULE_VERSION = 1;
};