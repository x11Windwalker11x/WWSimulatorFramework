// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ComponentManager.h"

#include "GameplayTagContainer.h"


// Sets default values for this component's properties
UComponentManager::UComponentManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UComponentManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


FGameplayTagContainer UComponentManager::GetTagsFromComponent(const FName& CrafterID, UActorComponent* TargetComponent, const UDataTable* DataTable ) const
{
	FGameplayTagContainer EmptyContainer;
	if (!TargetComponent) return EmptyContainer;
	if (!CrafterID.IsValid()) return EmptyContainer;

	FCrafterActor* CrafterActorRow = DataTable->FindRow<FCrafterActor>(CrafterID);
	if (!CrafterActorRow) return EmptyContainer;
	

	//Associated comp tags are supposed to be a TMap of UActorComponent* and FGameplayTagContainer
	FGameplayTagContainer* Result = CrafterActorRow->AssociatedComponentTags.Find(TargetComponent);
	if (!Result->IsEmpty()) return Result;
	else return EmptyContainer;
	
}
