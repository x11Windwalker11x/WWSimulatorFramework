#pragma once

#include "CoreMinimal.h"
#include "SaveDelegates.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSaveStarted,
	FName, SlotName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSaveCompleted,
	FName, SlotName,
	bool, bSucceded);

