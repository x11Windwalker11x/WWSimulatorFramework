#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "CraftingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCraftingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MODULARINVENTORYSYSTEM_API ICraftingInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	// UActorComponent* GetInventoryComponentAsActorComponent(TSubclassOf<UActorComponent> ComponentClass);

	//TODO: Crafting Component not implemented at the moment
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	UActorComponent* GetCraftingComponent();

	//Gets the crafter tags from its inventorycomponent.
	//TODO: later it will be changed to crafting component. 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	FGameplayTagContainer GetCrafterTags() const;

	//Returns the Craftable Recipe ID
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	FName GetCraftableID(AActor* Crafter ) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool HasCraftingCapability(FGameplayTag CapabilityTag, int32 MinLevel);


	// Use BlueprintNativeEvent for both C++ and BP implementation
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor States")
	// bool GetbCanInteract();
	

};