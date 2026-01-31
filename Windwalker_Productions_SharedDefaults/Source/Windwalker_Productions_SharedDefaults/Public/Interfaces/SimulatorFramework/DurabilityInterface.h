#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PhysicalInteractionInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDurabilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors/components that have durability.
 * Implemented by UDurabilityComponent in ModularSystemsBase.
 * Actors do not implement it. Use InteractableInteface Instead..
 */

class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API IDurabilityInterface
{
	GENERATED_BODY()

public:
	// ============================================================================
	// OPERATIONS
	// ============================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	void ApplyDurabilityLoss(float Amount, AActor* Causer);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	void ApplyUseDamage(AActor* Causer);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	void Repair(float Amount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	void RepairFull();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	void SetDurability(float NewDurability);

	//Components
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable | Components")
	UActorComponent* GetDurabilityComponentAsActorComponent();

	// ============================================================================
	// QUERIES
	// ============================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	float GetDurability() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	float GetDurabilityPercent() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	bool IsBroken() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	bool IsLowDurability() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	bool HasDurability() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	bool IsUsable() const;

};