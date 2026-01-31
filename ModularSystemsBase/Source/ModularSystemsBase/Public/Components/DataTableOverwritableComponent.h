// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "Lib/Data/Core/WorldObjectData.h"
#include "Lib/Data/ModularCraftingData/CraftingData.h"
#include "DataTableOverwritableComponent.generated.h"


UCLASS(Abstract, ClassGroup=(Components))
class MODULARSYSTEMSBASE_API UDataTableOverwritableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDataTableOverwritableComponent();

	//--GETTERS---//
	UFUNCTION(BlueprintPure, Category = "Getters")
	FGameplayTagContainer GetTagsPure() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName ID = FName("");


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafter Variables")
	FWorldObjectData InstanceData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafter Variables")
	FGameplayTagContainer DataTagContainer;

	//TFunction: function to call: GetDataTable function in each module
	//TFunction already requires the specific JsonReader object.
	//So, no need to use as template object.
	//TRowStruct: UStruct to configure the component
	template <auto TFunction, typename TRowStruct, typename TRowInstanceStruct>
	void SetupConfig(TRowInstanceStruct& DestInstance, FName ID);

	/*
	 *Gotta be very careful here,using typeless pointer (void* FStruct)
	 *if we point to a wrong struct that is outside of hierachy,
	 *the either get crashed or bluescreen may come up
	 */
	virtual void MapCustomData(FTableRowBase* DataTableRow, FTableRowBase* DataTableRowInstance);
	
};

