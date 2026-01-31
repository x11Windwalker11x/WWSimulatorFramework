// Fill out your copyright notice in the Description page of Project Settings.

/*
 *This interface was once in modular interaction system but since it is tied close with controller and pawns
 *it was better to put it here for logistical reasons
 * Handles all info that should be carried from pawn to interactable object. 
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MODULARINVENTORYSYSTEM_API IInventoryInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	// UActorComponent* GetInventoryComponentAsActorComponent(TSubclassOf<UActorComponent> ComponentClass);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	UActorComponent* GetInventoryComponent();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;
	
	// Use BlueprintNativeEvent for both C++ and BP implementation
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor States")
	// bool GetbCanInteract();

	// Add these to IInventoryInterface public section:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool TryConsumeItem(FName ItemID, int32 Quantity);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool HasItem(FName ItemID, int32 MinQuantity);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemID, int32 Quantity, float Quality, float Durability, bool bIsStolen);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool RemoveItemByInstance(FGuid InstanceID, int32 Quantity);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool DropItem(FGameplayTag InventoryType, int32 SlotIndex, int32 Quantity);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool UseItem(FGameplayTag InventoryType, int32 SlotIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool EquipItem(FGameplayTag InventoryType, int32 SlotIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool UnequipItem(FGameplayTag InventoryType, int32 SlotIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	int32 FindEmptySlot(FGameplayTag InventoryType) const;


};
