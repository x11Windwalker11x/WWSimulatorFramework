#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "WW_WidgetDelegates.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnWidgetShown,
	FGameplayTag, WidgetTag);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnWidgetHidden,
	FGameplayTag, WidgetTag);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWidgetRegistered,
	FGameplayTag, WidgetTag,
	UUserWidget*, Widget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnWidgetUnRegistered,
	FGameplayTag, WidgetTag);

