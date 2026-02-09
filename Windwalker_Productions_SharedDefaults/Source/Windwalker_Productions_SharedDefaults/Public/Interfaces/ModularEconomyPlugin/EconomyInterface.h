// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "EconomyInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEconomyInterface : public UInterface
{
	GENERATED_BODY()
};

class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API IEconomyInterface
{
	GENERATED_BODY()

public:
	// === RESOURCE TYPE ===

	/** Returns the resource type tag (Economy.Resource.*) this consumer uses */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Economy")
	FGameplayTag GetResourceType() const;

	/** Returns the current consumption rate in units per hour */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Economy")
	float GetResourceConsumptionRate() const;

	/** Returns true if this consumer is actively consuming resources */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Economy")
	bool IsConsuming() const;

	/** Returns the cost per hour based on rate and cost-per-unit */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Economy")
	float GetCostPerHour() const;

	// === CONTROL ===

	/** Enable or disable consumption */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Economy")
	void SetConsumptionActive(bool bActive);

	// === COMPONENT ACCESS ===

	/** Returns the economy component on this actor (if any) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Economy")
	UActorComponent* GetEconomyComponent();
};
