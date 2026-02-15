// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DaySummaryData.generated.h"

/**
 * FDaySummaryEntry
 * A single entry in the day-end summary screen.
 * Any L2 plugin can create and submit entries via UDaySummarySubsystem.
 * Rule #12: Data struct with zero logic (except IsValid).
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FDaySummaryEntry
{
	GENERATED_BODY()

	/** Category tag (Summary.Category.*) — used for sorting and grouping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DaySummary")
	FGameplayTag CategoryTag;

	/** Display text shown to the player (e.g. "Sold 5 items") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DaySummary")
	FText DisplayText;

	/** Icon tag (Summary.Icon.*) — used by widget to pick icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DaySummary")
	FGameplayTag IconTag;

	/** Sort order within category (lower = higher in list) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DaySummary")
	int32 SortOrder = 0;

	/** Numeric value for display (e.g. money earned, items crafted) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DaySummary")
	float NumericValue = 0.0f;

	/** Is this a positive entry (green) or negative (red)? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DaySummary")
	bool bIsPositive = true;

	bool IsValid() const
	{
		return CategoryTag.IsValid() && !DisplayText.IsEmpty();
	}
};
