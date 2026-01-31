// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTableOverwritableComponent.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "Interfaces/ModularInventorySystem/CraftingInterface.h"
#include "Lib/Data/ModularCraftingData/CraftingData.h"
#include "CrafterComponent_Master.generated.h"


UCLASS(Abstract, ClassGroup=(Components))
class MODULARSYSTEMSBASE_API UCrafterComponent_Master : public UDataTableOverwritableComponent, ICraftingInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCrafterComponent_Master();

	UFUNCTION(BlueprintPure, Category = "Crafter|Getters")
	FName GetCrafterID() const;


	//--GETTERS---//
	UFUNCTION(BlueprintPure, Category = "Getters")
	FGameplayTagContainer GetCrafterTagsPure() const;

	UFUNCTION(BlueprintPure, Category = "Getters")
	FCraftingCapabilitySet GetCapabilities() const;

	UFUNCTION(BlueprintPure, Category = "Getters")
	int32 GetCapabilityLevel(FGameplayTag Tag) const;

	UFUNCTION(BlueprintPure, Category = "Getters")
	bool HasCapability(FGameplayTag Tag, int32 MinLevel = 1) const;

	//---MODIFIERS---//
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void AddCapability(FGameplayTag Tag, int32 Level = 1);
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void RemoveCapability(FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void SetCapabilityLevel(FGameplayTag Tag, int32 NewLevel);
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void MergeCapabilities(const FCraftingCapabilitySet& Other);
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void ClearCapabilities();
	
	//Interface Functions
	virtual UActorComponent* GetCraftingComponent_Implementation() override;
	virtual FName GetCraftableID_Implementation(AActor* Crafter ) const override;
	virtual FGameplayTagContainer GetCrafterTags_Implementation() const override;
	virtual bool HasCraftingCapability_Implementation(FGameplayTag CapabilityTag, int32 MinLevel) override;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafter Variables")
	FCrafterInstanceData* CrafterInstanceData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafter Variables")
	FGameplayTagContainer BaseCrafterTags;
	
	/** Runtime capabilities (skills, station bonuses, tool bonuses combined) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafter|Runtime")
	FCraftingCapabilitySet Capabilities;

	virtual void MapCustomData(FTableRowBase* DataTableRow, FTableRowBase* DataTableRowInstance) override;



	
};
