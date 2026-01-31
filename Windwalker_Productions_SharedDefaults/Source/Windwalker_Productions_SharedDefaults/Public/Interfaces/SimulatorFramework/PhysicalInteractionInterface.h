#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PhysicalInteractionInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UPhysicalInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that can be physically grabbed/manipulated.
 * Grab replaces standard interaction — mutually exclusive.
 * Physics handle implementation lives in SimulatorFramework.
 */
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API IPhysicalInteractionInterface
{
	GENERATED_BODY()

public:
	/** Can this actor be grabbed right now? */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Physical Interaction")
	bool CanBeGrabbed() const;
    
	/** Called when grab starts */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Physical Interaction")
	void OnGrabbed(AActor* GrabbingActor);
    
	/** Called when grab ends */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Physical Interaction")
	void OnReleased(AActor* ReleasingActor);
    
	/** Get grab offset from actor origin */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Physical Interaction")
	FVector GetGrabOffset() const;
    
	/** Get primitive component to grab */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Physical Interaction")
	UPrimitiveComponent* GetGrabComponent() const;
    
	/** Get mass for physics handle */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Physical Interaction")
	float GetGrabMass() const;
    
	/** Is currently being held? */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Physical Interaction")
	bool IsBeingHeld() const;
};