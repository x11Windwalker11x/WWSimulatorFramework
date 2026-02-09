// Copyright Windwalker Productions. All Rights Reserved.

#include "Subsystems/EconomySubsystem.h"
#include "Interfaces/ModularEconomyPlugin/EconomyInterface.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEconomySubsystem::UEconomySubsystem()
{
}

void UEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TransactionHistory.Reserve(MaxTransactionHistory);
}

void UEconomySubsystem::Deinitialize()
{
	StopBillingCycle();
	RegisteredConsumers.Empty();
	TransactionHistory.Empty();
	BillingEntries.Empty();

	Super::Deinitialize();
}

UEconomySubsystem* UEconomySubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UEconomySubsystem>();
}

// ============================================================================
// BALANCE API
// ============================================================================

float UEconomySubsystem::AddFunds(float Amount, FGameplayTag Category, const FString& Description, AActor* Source)
{
	if (Amount <= 0.f)
	{
		return Balance;
	}

	const float OldBalance = Balance;
	Balance += Amount;

	// Record transaction
	FEconomyTransaction Transaction;
	Transaction.Amount = Amount;
	Transaction.Category = Category;
	Transaction.Description = Description;
	Transaction.SourceActor = Source;

	UWorld* World = GetWorldForTimers();
	Transaction.Timestamp = World ? World->GetTimeSeconds() : 0.f;

	RecordTransaction(Transaction);

	// Update running totals
	TotalIncome += Amount;
	IncomeTotals.FindOrAdd(Category) += Amount;

	// Broadcast
	OnBalanceChanged.Broadcast(Balance, Amount);
	OnTransactionProcessed.Broadcast(Amount, Category, Balance);

	return Balance;
}

bool UEconomySubsystem::DeductFunds(float Amount, FGameplayTag Category, const FString& Description, AActor* Source, bool bAllowDebt)
{
	if (Amount <= 0.f)
	{
		return false;
	}

	if (!bAllowDebt && Balance < Amount)
	{
		return false;
	}

	const float OldBalance = Balance;
	Balance -= Amount;

	// Record transaction
	FEconomyTransaction Transaction;
	Transaction.Amount = -Amount;
	Transaction.Category = Category;
	Transaction.Description = Description;
	Transaction.SourceActor = Source;

	UWorld* World = GetWorldForTimers();
	Transaction.Timestamp = World ? World->GetTimeSeconds() : 0.f;

	RecordTransaction(Transaction);

	// Update running totals
	TotalExpenses += Amount;
	ExpenseTotals.FindOrAdd(Category) += Amount;

	// Broadcast
	OnBalanceChanged.Broadcast(Balance, -Amount);
	OnTransactionProcessed.Broadcast(-Amount, Category, Balance);

	if (Balance < 0.f)
	{
		OnBalanceNegative.Broadcast(Balance);
	}

	return true;
}

void UEconomySubsystem::SetBalance(float NewBalance)
{
	const float Delta = NewBalance - Balance;
	Balance = NewBalance;

	OnBalanceChanged.Broadcast(Balance, Delta);

	if (Balance < 0.f)
	{
		OnBalanceNegative.Broadcast(Balance);
	}
}

// ============================================================================
// TRANSACTION HISTORY
// ============================================================================

TArray<FEconomyTransaction> UEconomySubsystem::GetRecentTransactions(int32 Count) const
{
	const int32 Num = FMath::Min(Count, TransactionHistory.Num());
	TArray<FEconomyTransaction> Result;
	Result.Reserve(Num);

	// Return most recent first (history is stored oldest-first)
	for (int32 i = TransactionHistory.Num() - 1; i >= TransactionHistory.Num() - Num; --i)
	{
		Result.Add(TransactionHistory[i]);
	}

	return Result;
}

TArray<FEconomyTransaction> UEconomySubsystem::GetTransactionsByCategory(FGameplayTag Category) const
{
	TArray<FEconomyTransaction> Result;

	for (const FEconomyTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.Category.MatchesTagExact(Category))
		{
			Result.Add(Transaction);
		}
	}

	return Result;
}

FFinancialSummary UEconomySubsystem::GetFinancialSummary() const
{
	FFinancialSummary Summary;
	Summary.Balance = Balance;
	Summary.TotalIncome = TotalIncome;
	Summary.TotalExpenses = TotalExpenses;
	Summary.NetProfit = TotalIncome - TotalExpenses;
	Summary.IncomeByCategory = IncomeTotals;
	Summary.ExpensesByCategory = ExpenseTotals;

	return Summary;
}

void UEconomySubsystem::ClearTransactionHistory()
{
	TransactionHistory.Empty();
	IncomeTotals.Empty();
	ExpenseTotals.Empty();
	TotalIncome = 0.f;
	TotalExpenses = 0.f;
}

void UEconomySubsystem::RecordTransaction(const FEconomyTransaction& Transaction)
{
	if (TransactionHistory.Num() >= MaxTransactionHistory)
	{
		TransactionHistory.RemoveAt(0);
	}

	TransactionHistory.Add(Transaction);
}

// ============================================================================
// RESOURCE CONSUMER REGISTRATION
// ============================================================================

bool UEconomySubsystem::RegisterConsumer(AActor* Consumer)
{
	if (!Consumer)
	{
		return false;
	}

	// Rule #25-32: Verify interface via ImplementsInterface
	if (!Consumer->GetClass()->ImplementsInterface(UEconomyInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("EconomySubsystem::RegisterConsumer - Actor %s does not implement IEconomyInterface"), *Consumer->GetName());
		return false;
	}

	// Check for duplicates
	for (const TWeakObjectPtr<AActor>& Existing : RegisteredConsumers)
	{
		if (Existing.Get() == Consumer)
		{
			return false;
		}
	}

	RegisteredConsumers.Add(Consumer);
	OnResourceConsumerChanged.Broadcast(Consumer, true);

	return true;
}

void UEconomySubsystem::UnregisterConsumer(AActor* Consumer)
{
	if (!Consumer)
	{
		return;
	}

	const int32 Removed = RegisteredConsumers.RemoveAll([Consumer](const TWeakObjectPtr<AActor>& Weak)
	{
		return Weak.Get() == Consumer;
	});

	if (Removed > 0)
	{
		OnResourceConsumerChanged.Broadcast(Consumer, false);
	}
}

TArray<AActor*> UEconomySubsystem::GetRegisteredConsumers() const
{
	TArray<AActor*> Result;
	Result.Reserve(RegisteredConsumers.Num());

	for (const TWeakObjectPtr<AActor>& Weak : RegisteredConsumers)
	{
		if (AActor* Actor = Weak.Get())
		{
			Result.Add(Actor);
		}
	}

	return Result;
}

float UEconomySubsystem::GetTotalHourlyCost() const
{
	float Total = 0.f;

	for (const TWeakObjectPtr<AActor>& Weak : RegisteredConsumers)
	{
		AActor* Actor = Weak.Get();
		if (!Actor)
		{
			continue;
		}

		if (IEconomyInterface::Execute_IsConsuming(Actor))
		{
			Total += IEconomyInterface::Execute_GetCostPerHour(Actor);
		}
	}

	return Total;
}

float UEconomySubsystem::GetHourlyCostByResource(FGameplayTag ResourceType) const
{
	float Total = 0.f;

	for (const TWeakObjectPtr<AActor>& Weak : RegisteredConsumers)
	{
		AActor* Actor = Weak.Get();
		if (!Actor)
		{
			continue;
		}

		if (IEconomyInterface::Execute_IsConsuming(Actor) &&
			IEconomyInterface::Execute_GetResourceType(Actor).MatchesTagExact(ResourceType))
		{
			Total += IEconomyInterface::Execute_GetCostPerHour(Actor);
		}
	}

	return Total;
}

void UEconomySubsystem::CleanStaleConsumers()
{
	RegisteredConsumers.RemoveAll([](const TWeakObjectPtr<AActor>& Weak)
	{
		return !Weak.IsValid();
	});
}

// ============================================================================
// BILLING CYCLE
// ============================================================================

void UEconomySubsystem::StartBillingCycle()
{
	UWorld* World = GetWorldForTimers();
	if (!World)
	{
		return;
	}

	bBillingCycleActive = true;

	World->GetTimerManager().SetTimer(
		BillingCycleHandle,
		this,
		&UEconomySubsystem::ProcessBillingCycleNow,
		BillingIntervalSeconds,
		true // looping
	);
}

void UEconomySubsystem::StopBillingCycle()
{
	bBillingCycleActive = false;

	UWorld* World = GetWorldForTimers();
	if (World)
	{
		World->GetTimerManager().ClearTimer(BillingCycleHandle);
	}
}

void UEconomySubsystem::SetBillingInterval(float Seconds)
{
	BillingIntervalSeconds = FMath::Max(1.f, Seconds);

	// Restart timer if active
	if (bBillingCycleActive)
	{
		StopBillingCycle();
		StartBillingCycle();
	}
}

void UEconomySubsystem::ProcessBillingCycleNow()
{
	// Step 1: Clean stale consumer references
	CleanStaleConsumers();

	float TotalBilled = 0.f;

	// Step 2: Poll each consumer's cost
	const float CycleHours = BillingIntervalSeconds / 3600.f;

	for (const TWeakObjectPtr<AActor>& Weak : RegisteredConsumers)
	{
		AActor* Actor = Weak.Get();
		if (!Actor)
		{
			continue;
		}

		if (IEconomyInterface::Execute_IsConsuming(Actor))
		{
			const float HourlyCost = IEconomyInterface::Execute_GetCostPerHour(Actor);
			const float CycleCost = HourlyCost * CycleHours;

			if (CycleCost > 0.f)
			{
				TotalBilled += CycleCost;
			}
		}
	}

	// Step 3: Sum billing entries
	for (const FBillingEntry& Entry : BillingEntries)
	{
		if (Entry.bIsActive && Entry.CostPerCycle > 0.f)
		{
			TotalBilled += Entry.CostPerCycle;
		}
	}

	// Step 4: Deduct total (allow debt on billing cycles)
	if (TotalBilled > 0.f)
	{
		DeductFunds(TotalBilled, FWWTagLibrary::Economy_Category_Utility(), TEXT("Billing Cycle"), nullptr, true);
	}

	// Step 5: Broadcast
	OnBillingCycleComplete.Broadcast(TotalBilled, Balance);
}

// ============================================================================
// BILLING ENTRIES
// ============================================================================

bool UEconomySubsystem::AddBillingEntry(const FBillingEntry& Entry)
{
	if (!Entry.IsValid())
	{
		return false;
	}

	// Check for duplicate ID
	for (const FBillingEntry& Existing : BillingEntries)
	{
		if (Existing.BillingID == Entry.BillingID)
		{
			return false;
		}
	}

	BillingEntries.Add(Entry);
	OnBillingEntryChanged.Broadcast(Entry.BillingID, true);

	return true;
}

bool UEconomySubsystem::RemoveBillingEntry(FName BillingID)
{
	const int32 Removed = BillingEntries.RemoveAll([BillingID](const FBillingEntry& Entry)
	{
		return Entry.BillingID == BillingID;
	});

	if (Removed > 0)
	{
		OnBillingEntryChanged.Broadcast(BillingID, false);
		return true;
	}

	return false;
}

bool UEconomySubsystem::SetBillingEntryActive(FName BillingID, bool bActive)
{
	for (FBillingEntry& Entry : BillingEntries)
	{
		if (Entry.BillingID == BillingID)
		{
			Entry.bIsActive = bActive;
			return true;
		}
	}

	return false;
}

// ============================================================================
// HELPERS
// ============================================================================

UWorld* UEconomySubsystem::GetWorldForTimers() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}
