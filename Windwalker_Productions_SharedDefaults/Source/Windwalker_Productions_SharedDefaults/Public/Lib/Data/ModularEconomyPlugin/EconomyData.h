// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EconomyData.generated.h"

/**
 * Single financial event (income or expense)
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FEconomyTransaction
{
	GENERATED_BODY()

	/** Amount of the transaction (positive = income, negative = expense) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float Amount = 0.f;

	/** Category tag (Economy.Category.*) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FGameplayTag Category;

	/** Human-readable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FString Description;

	/** World time when transaction occurred */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float Timestamp = 0.f;

	/** Actor that caused this transaction (can be null) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	TWeakObjectPtr<AActor> SourceActor;

	bool IsValid() const
	{
		return Amount != 0.f;
	}
};

/**
 * Resource consumption rate definition
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FResourceRate
{
	GENERATED_BODY()

	/** Resource type tag (Economy.Resource.*) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FGameplayTag ResourceType;

	/** Units consumed per game-hour */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy", meta = (ClampMin = "0.0"))
	float UnitsPerHour = 1.f;

	/** Cost per unit consumed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy", meta = (ClampMin = "0.0"))
	float CostPerUnit = 1.f;

	/** Whether this resource is actively being consumed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	bool bIsActive = false;

	bool IsValid() const
	{
		return ResourceType.IsValid();
	}
};

/**
 * Recurring billing entry (rent, wages, subscriptions)
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FBillingEntry
{
	GENERATED_BODY()

	/** Unique identifier for this billing entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FName BillingID;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FString DisplayName;

	/** Category tag (Economy.Category.*) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FGameplayTag Category;

	/** Cost deducted each billing cycle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy", meta = (ClampMin = "0.0"))
	float CostPerCycle = 0.f;

	/** Whether this entry is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	bool bIsActive = true;

	bool IsValid() const
	{
		return !BillingID.IsNone() && CostPerCycle > 0.f;
	}
};

/**
 * Financial summary snapshot for UI display
 * Rule #12: Zero logic except IsValid()
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FFinancialSummary
{
	GENERATED_BODY()

	/** Current balance */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float Balance = 0.f;

	/** Total income since last reset */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float TotalIncome = 0.f;

	/** Total expenses since last reset */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float TotalExpenses = 0.f;

	/** Net profit (TotalIncome - TotalExpenses) */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float NetProfit = 0.f;

	/** Breakdown of income by category tag */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	TMap<FGameplayTag, float> IncomeByCategory;

	/** Breakdown of expenses by category tag */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	TMap<FGameplayTag, float> ExpensesByCategory;

	bool IsValid() const
	{
		return true;
	}
};
