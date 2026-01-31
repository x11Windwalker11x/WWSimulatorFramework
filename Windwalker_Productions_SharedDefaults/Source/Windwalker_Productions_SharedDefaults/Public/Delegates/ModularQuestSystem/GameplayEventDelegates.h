#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/ModularQuestSystem/GameplayEventData.h"
#include "GameplayEventDelegates.generated.h"

//All Gameplay events and quests are in module ModularQuestSystem.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnGameplayEvent,
	FGameplayTag, EventTag,
	AActor*, Instigator,
	UObject*, Payload
);

DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FGameplayEventCallback,
	FGameplayTag, EventTag,
	AActor*, Instigator,
	UObject*, Payload
);

