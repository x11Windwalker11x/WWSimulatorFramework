// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OutlineComponent.generated.h"


UCLASS(ClassGroup=(Components), meta=(BlueprintSpawnableComponent))
class MODULARINTERACTIONSYSTEM_API UOutlineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UOutlineComponent();
	UFUNCTION(BlueprintCallable, Category = "Outline")
	void SetOutlineActive(bool bActive);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(Transient)
	TObjectPtr<AActor> Owner;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Outline")
	int32 OutlineColorIndex;


	
};
