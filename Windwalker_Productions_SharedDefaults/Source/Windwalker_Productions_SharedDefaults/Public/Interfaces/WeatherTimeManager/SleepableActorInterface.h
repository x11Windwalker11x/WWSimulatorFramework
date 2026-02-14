// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SleepableActorInterface.generated.h"

class UActorComponent;

UINTERFACE(MinimalAPI, BlueprintType)
class USleepableActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that can participate in the sleep system.
 * Beds, sleeping bags, etc. implement this to define sleep behavior.
 */
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API ISleepableActorInterface
{
	GENERATED_BODY()

public:
	/** Can this actor currently be used for sleeping? */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sleep")
	bool CanSleep() const;

	/** Get the world location where the player sleeps */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sleep")
	FVector GetSleepLocation() const;

	/** Called when sleep begins on this actor */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sleep")
	void OnSleepStarted();

	/** Called when sleep completes on this actor */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sleep")
	void OnSleepCompleted();

	/** Called when sleep is cancelled on this actor */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sleep")
	void OnSleepCancelled();

	/** Rule #29: Mandatory component getter */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sleep|Components")
	UActorComponent* GetSleepComponent();
};
