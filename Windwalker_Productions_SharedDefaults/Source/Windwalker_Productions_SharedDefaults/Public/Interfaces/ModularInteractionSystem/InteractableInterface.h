// InteractableInterface.h
/*
* Interactable responsible transitioning of info between an interactable (item, npc, door, weapon, dialouge etc)
*  and an interactor (player, ai, event systems etc).
*/
#pragma once

#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "InteractableInterface.generated.h"

class UInventoryComponent;
UINTERFACE(MinimalAPI, BlueprintType)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API  IInteractableInterface
{
	GENERATED_BODY()

public:
	//TODO: Make a uobject called interactable, and implement those events there and make other interactables inherit fromt that.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void OnInteract(AController* InstigatorController);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	FItemData_Dynamic GetPickupData() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void SetPickupData(FItemData_Dynamic& InPickupData);

	//Components
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable | Components")
	UActorComponent* GetInventoryComponentAsActorComponent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable | Components")
	UActorComponent* GetDurabilityComponentAsActorComponent();
	
	//For Initializing the item
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item | Initialization")
	void SetItemID(FName ItemID);
	
	//For UI
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void OnPreInteractionEntered(APawn* OtherPawn);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void OnFullInteractionEntered(APawn* OtherPawn);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void OnPreInteractionExited(APawn* OtherPawn);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void OnFullInteractionExited(APawn* OtherPawn);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	UActorComponent* GetOutlineComponent();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void SetOutlineActive(bool bActivate);
	
	//For Gameplay
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	bool IsCurrentlyInteractable();

	//For Optimisation
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void SetInteractionEnabled(bool bIsEnabled);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	bool GetInteractionEnabled();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable", meta = (ToolTip = "Used for mulitcast delegates on state changes especially for loading and unloading"))
	void SetInteractionNotifications();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable", meta = (ToolTip = "Disables InteractionSubsystem Handling for optimisation and ui."))
	bool GetbInteractionNotificationsAllowManualHandling();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void LoadAssets();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void UnloadAssets();
	//End of Optimisation
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	FGameplayTagContainer GetGameplayTags();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void OnPlayerNearbyInit(APawn* PlayerPawn, const bool bIsNearby); // just a notifer from sphere component from pawn who has active player controller.

	// In your InteractableInterface.h

	/** Called when interaction is predicted (client-side cosmetic effects only) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|Prediction")
	void OnInteractPredicted(AController* InstigatorController);
	virtual void OnInteractPredicted_Implementation(AController* InstigatorController) {}

	/** Cache state for rollback */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|Prediction")
	void CachePredictionState(FInteractableState& OutState);
	virtual void CachePredictionState_Implementation(FInteractableState& OutState) {}

	/** Restore state after rollback */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|Prediction")
	void RestorePredictionState(const FInteractableState& State);
	virtual void RestorePredictionState_Implementation(const FInteractableState& State) {}
	
};
