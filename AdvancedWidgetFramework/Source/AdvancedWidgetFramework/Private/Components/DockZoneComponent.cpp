// DockZoneComponent.cpp
// Placeable dock zone target component implementation

#include "Components/DockZoneComponent.h"
#include "Subsystems/DockLayoutManager.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"

UDockZoneComponent::UDockZoneComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDockZoneComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// No replicated properties yet - zone config is set at design time
}

void UDockZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ZoneConfig.IsValid()) return;

	// Find the local player's DockLayoutManager and register
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		// If not on a PlayerController, try to get the first local player
		if (UWorld* World = GetWorld())
		{
			PC = World->GetFirstPlayerController();
		}
	}

	if (PC)
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			UDockLayoutManager* Manager = LocalPlayer->GetSubsystem<UDockLayoutManager>();
			if (Manager)
			{
				CachedLayoutManager = Manager;
				Manager->RegisterDockZoneWithComponent(ZoneConfig, this);
			}
		}
	}
}

void UDockZoneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from layout manager
	if (UDockLayoutManager* Manager = CachedLayoutManager.Get())
	{
		Manager->UnregisterDockZone(ZoneConfig.ZoneTag);
	}

	Super::EndPlay(EndPlayReason);
}
