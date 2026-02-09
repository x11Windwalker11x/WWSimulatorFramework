// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EconomyDelegates.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnBalanceChanged,
	float, NewBalance,
	float, Delta);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnTransactionProcessed,
	float, Amount,
	FGameplayTag, Category,
	float, NewBalance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnBillingCycleComplete,
	float, TotalBilled,
	float, NewBalance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnResourceConsumerChanged,
	AActor*, Consumer,
	bool, bRegistered);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnResourceRateChanged,
	AActor*, Consumer,
	FGameplayTag, ResourceType,
	float, NewRate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnBalanceNegative,
	float, CurrentBalance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnBillingEntryChanged,
	FName, BillingID,
	bool, bAdded);
