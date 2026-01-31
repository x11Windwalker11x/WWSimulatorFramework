// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/OutlineComponent.h"

#include "Windwalker_Productions_SharedDefaults.h"


// Sets default values for this component's properties
UOutlineComponent::UOutlineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	OutlineColorIndex = InteractionDefaultsConstants::OUTLINE_DEFAULT_COLOR;

	// ...
}

void UOutlineComponent::SetOutlineActive(bool bActive)
{
	if (!Owner || Owner->HasAuthority()) return;

	//Get all the meshes the owner has
	//TODO: write a stencil aware post process material
	TArray<UMeshComponent*> Meshes;
	Owner->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* MeshComponent : Meshes)
	{
		MeshComponent->SetRenderCustomDepth(bActive);
		MeshComponent->SetCustomDepthStencilValue(bActive ? OutlineColorIndex : 0);
	}
}


// Called when the game starts
void UOutlineComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = GetOwner();
	SetOutlineActive(false);
	// ...
	
}

void UOutlineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetOutlineActive(false);
	Super::EndPlay(EndPlayReason);
}




