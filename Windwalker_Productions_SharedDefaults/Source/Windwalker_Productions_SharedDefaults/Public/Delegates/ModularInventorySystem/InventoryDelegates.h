#pragma once
#include "CoreMinimal.h"
#include "InventoryDelegates.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnItemAdded,
	int32, SlotIndex,
	FName, ItemID,
	int32, Quantity);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnItemRemoved,
	int32, SlotIndex,
	FName, ItemID,
	int32, ItemQuantity);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSlotChanged,
	int32, SlotIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnInventoryCleared);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnItemDropped,
	FName, ItemID,
	const FInventorySlot&, DroppedSlot,
	int32, ItemQuantity,
	FVector, Location);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnItemConsumed,
	FName, ItemID,
	int32, ItemQuantity);
