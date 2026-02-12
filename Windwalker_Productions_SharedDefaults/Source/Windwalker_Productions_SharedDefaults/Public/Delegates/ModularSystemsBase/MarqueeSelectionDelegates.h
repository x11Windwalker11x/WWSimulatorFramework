// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/ModularSystemsBase/MarqueeSelectionData.h"
#include "MarqueeSelectionDelegates.generated.h"

/** Broadcast when a marquee selection operation completes successfully */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMarqueeSelectionComplete,
	const FMarqueeSelectionResult&, Result);

/** Broadcast when a marquee selection operation is cancelled */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMarqueeCancelled);
