// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/WeatherTimeManager/DaySummaryData.h"
#include "DaySummaryDelegates.generated.h"

/** Broadcast to all contributors when a day summary is requested. Contributors should call SubmitEntry(). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnDaySummaryRequested,
	int32, CurrentDay,
	float, HoursSlept);

/** Broadcast when all entries are collected and sorted, ready for display. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnDaySummaryReady,
	const TArray<FDaySummaryEntry>&, Entries);
