// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ComponentManager.generated.h"


UCLASS(Abstract, ClassGroup=(Managers), meta=(BlueprintSpawnableComponent))
class MODULARSYSTEMSBASE_API UComponentManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UComponentManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Components")
	virtual FGameplayTagContainer GetTagsFromComponent(const FName& CrafterID, UActorComponent* TargetComponent, const UDataTable* DataTable ) const;

};
