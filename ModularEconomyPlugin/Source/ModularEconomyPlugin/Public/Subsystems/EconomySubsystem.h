// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/ModularEconomyPlugin/EconomyDelegates.h"
#include "Lib/Data/ModularEconomyPlugin/EconomyData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "EconomySubsystem.generated.h"

/**
 * Economy Subsystem
 *
 * Central financial manager for the game. Handles:
 * - Balance tracking with full transaction history
 * - Recurring billing entries (rent, wages, subscriptions)
 * - Resource consumer registration (devices that consume power/water/gas)
 * - Configurable billing cycle that auto-deducts costs
 *
 * Uses FTimerHandle for billing (no tick). Polls consumers each cycle.
 */
UCLASS()
class MODULARECONOMYPLUGIN_API UEconomySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEconomySubsystem();

	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Static accessor */
	UFUNCTION(BlueprintPure, Category = "Economy", meta = (WorldContext = "WorldContextObject"))
	static UEconomySubsystem* Get(const UObject* WorldContextObject);

	// ============================================================================
	// DELEGATES
	// ============================================================================

	/** Fires when balance changes for any reason */
	UPROPERTY(BlueprintAssignable, Category = "Economy|Delegates")
	FOnBalanceChanged OnBalanceChanged;

	/** Fires after every transaction is processed */
	UPROPERTY(BlueprintAssignable, Category = "Economy|Delegates")
	FOnTransactionProcessed OnTransactionProcessed;

	/** Fires when a billing cycle completes */
	UPROPERTY(BlueprintAssignable, Category = "Economy|Delegates")
	FOnBillingCycleComplete OnBillingCycleComplete;

	/** Fires when a resource consumer is registered or unregistered */
	UPROPERTY(BlueprintAssignable, Category = "Economy|Delegates")
	FOnResourceConsumerChanged OnResourceConsumerChanged;

	/** Fires when a consumer's resource rate changes */
	UPROPERTY(BlueprintAssignable, Category = "Economy|Delegates")
	FOnResourceRateChanged OnResourceRateChanged;

	/** Fires when balance goes negative */
	UPROPERTY(BlueprintAssignable, Category = "Economy|Delegates")
	FOnBalanceNegative OnBalanceNegative;

	/** Fires when a billing entry is added or removed */
	UPROPERTY(BlueprintAssignable, Category = "Economy|Delegates")
	FOnBillingEntryChanged OnBillingEntryChanged;

	// ============================================================================
	// BALANCE API
	// ============================================================================

	/** Get current balance */
	UFUNCTION(BlueprintPure, Category = "Economy|Balance")
	float GetBalance() const { return Balance; }

	/** Add funds (income). Returns new balance. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Balance")
	float AddFunds(float Amount, FGameplayTag Category, const FString& Description, AActor* Source = nullptr);

	/** Deduct funds (expense). Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Balance")
	bool DeductFunds(float Amount, FGameplayTag Category, const FString& Description, AActor* Source = nullptr, bool bAllowDebt = false);

	/** Check if player can afford a given amount */
	UFUNCTION(BlueprintPure, Category = "Economy|Balance")
	bool CanAfford(float Amount) const { return Balance >= Amount; }

	/** Force-set balance (admin/cheat). Broadcasts OnBalanceChanged. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Balance")
	void SetBalance(float NewBalance);

	// ============================================================================
	// TRANSACTION HISTORY
	// ============================================================================

	/** Get N most recent transactions */
	UFUNCTION(BlueprintCallable, Category = "Economy|History")
	TArray<FEconomyTransaction> GetRecentTransactions(int32 Count = 10) const;

	/** Get all transactions matching a category tag */
	UFUNCTION(BlueprintCallable, Category = "Economy|History")
	TArray<FEconomyTransaction> GetTransactionsByCategory(FGameplayTag Category) const;

	/** Get financial summary snapshot for UI */
	UFUNCTION(BlueprintPure, Category = "Economy|History")
	FFinancialSummary GetFinancialSummary() const;

	/** Clear all transaction history and running totals */
	UFUNCTION(BlueprintCallable, Category = "Economy|History")
	void ClearTransactionHistory();

	// ============================================================================
	// RESOURCE CONSUMER REGISTRATION
	// ============================================================================

	/** Register an actor as a resource consumer (must implement IEconomyInterface) */
	UFUNCTION(BlueprintCallable, Category = "Economy|Resources")
	bool RegisterConsumer(AActor* Consumer);

	/** Unregister a resource consumer */
	UFUNCTION(BlueprintCallable, Category = "Economy|Resources")
	void UnregisterConsumer(AActor* Consumer);

	/** Get all registered consumers */
	UFUNCTION(BlueprintCallable, Category = "Economy|Resources")
	TArray<AActor*> GetRegisteredConsumers() const;

	/** Get total hourly cost across all active consumers */
	UFUNCTION(BlueprintPure, Category = "Economy|Resources")
	float GetTotalHourlyCost() const;

	/** Get hourly cost filtered by resource type */
	UFUNCTION(BlueprintPure, Category = "Economy|Resources")
	float GetHourlyCostByResource(FGameplayTag ResourceType) const;

	// ============================================================================
	// BILLING CYCLE
	// ============================================================================

	/** Start the automatic billing cycle timer */
	UFUNCTION(BlueprintCallable, Category = "Economy|Billing")
	void StartBillingCycle();

	/** Stop the automatic billing cycle timer */
	UFUNCTION(BlueprintCallable, Category = "Economy|Billing")
	void StopBillingCycle();

	/** Set billing interval in seconds */
	UFUNCTION(BlueprintCallable, Category = "Economy|Billing")
	void SetBillingInterval(float Seconds);

	/** Get current billing interval in seconds */
	UFUNCTION(BlueprintPure, Category = "Economy|Billing")
	float GetBillingInterval() const { return BillingIntervalSeconds; }

	/** Manually trigger a billing cycle now */
	UFUNCTION(BlueprintCallable, Category = "Economy|Billing")
	void ProcessBillingCycleNow();

	/** Is billing cycle currently active? */
	UFUNCTION(BlueprintPure, Category = "Economy|Billing")
	bool IsBillingCycleActive() const { return bBillingCycleActive; }

	// ============================================================================
	// BILLING ENTRIES
	// ============================================================================

	/** Add a recurring billing entry */
	UFUNCTION(BlueprintCallable, Category = "Economy|Billing")
	bool AddBillingEntry(const FBillingEntry& Entry);

	/** Remove a billing entry by ID */
	UFUNCTION(BlueprintCallable, Category = "Economy|Billing")
	bool RemoveBillingEntry(FName BillingID);

	/** Enable or disable a billing entry */
	UFUNCTION(BlueprintCallable, Category = "Economy|Billing")
	bool SetBillingEntryActive(FName BillingID, bool bActive);

	/** Get all billing entries */
	UFUNCTION(BlueprintCallable, Category = "Economy|Billing")
	TArray<FBillingEntry> GetBillingEntries() const { return BillingEntries; }

private:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	/** Current balance */
	float Balance = 0.f;

	/** Transaction history (circular buffer, max 200) */
	TArray<FEconomyTransaction> TransactionHistory;

	/** Maximum transactions to keep in history */
	static constexpr int32 MaxTransactionHistory = 200;

	/** Running income totals by category */
	TMap<FGameplayTag, float> IncomeTotals;

	/** Running expense totals by category */
	TMap<FGameplayTag, float> ExpenseTotals;

	/** Total income since last clear */
	float TotalIncome = 0.f;

	/** Total expenses since last clear */
	float TotalExpenses = 0.f;

	/** Registered resource consumers */
	TArray<TWeakObjectPtr<AActor>> RegisteredConsumers;

	/** Recurring billing entries */
	TArray<FBillingEntry> BillingEntries;

	/** Billing cycle interval in seconds */
	float BillingIntervalSeconds = 60.f;

	/** Whether billing cycle is active */
	bool bBillingCycleActive = false;

	/** Timer handle for billing cycle */
	FTimerHandle BillingCycleHandle;

	// ============================================================================
	// INTERNAL HELPERS
	// ============================================================================

	/** Record a transaction to history (capped at MaxTransactionHistory) */
	void RecordTransaction(const FEconomyTransaction& Transaction);

	/** Clean up stale (destroyed) consumer references */
	void CleanStaleConsumers();

	/** Get the world for timer access */
	UWorld* GetWorldForTimers() const;
};
