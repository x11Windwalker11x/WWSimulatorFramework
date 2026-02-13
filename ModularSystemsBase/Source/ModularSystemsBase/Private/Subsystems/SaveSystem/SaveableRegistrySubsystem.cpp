// Copyright Windwalker Productions. All Rights Reserved.

#include "Subsystems/SaveSystem/SaveableRegistrySubsystem.h"
#include "Interfaces/ModularSaveGameSystem/SaveableInterface.h"

void USaveableRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USaveableRegistrySubsystem::Deinitialize()
{
	RegisteredSaveables.Empty();
	PrioritySortedIDs.Empty();
	Super::Deinitialize();
}

USaveableRegistrySubsystem* USaveableRegistrySubsystem::Get(const UObject* WorldContextObject)
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

	return GI->GetSubsystem<USaveableRegistrySubsystem>();
}

bool USaveableRegistrySubsystem::RegisterSaveable(UObject* SaveableObject)
{
	if (!SaveableObject || !SaveableObject->GetClass()->ImplementsInterface(USaveableInterface::StaticClass()))
	{
		return false;
	}

	const FString SaveID = ISaveableInterface::Execute_GetSaveID(SaveableObject);
	if (SaveID.IsEmpty())
	{
		return false;
	}

	if (RegisteredSaveables.Contains(SaveID))
	{
		return false;
	}

	RegisteredSaveables.Add(SaveID, SaveableObject);
	RebuildPriorityCache();

	const int32 Priority = ISaveableInterface::Execute_GetSavePriority(SaveableObject);
	OnSaveableRegistered.Broadcast(SaveID, Priority);

	return true;
}

bool USaveableRegistrySubsystem::UnregisterSaveable(const FString& SaveID)
{
	if (!RegisteredSaveables.Contains(SaveID))
	{
		return false;
	}

	RegisteredSaveables.Remove(SaveID);
	RebuildPriorityCache();

	OnSaveableUnregistered.Broadcast(SaveID);

	return true;
}

TArray<UObject*> USaveableRegistrySubsystem::GetAllSaveables() const
{
	return GetSortedObjects();
}

TArray<UObject*> USaveableRegistrySubsystem::GetDirtySaveables() const
{
	TArray<UObject*> Result;
	for (const TPair<int32, FString>& Pair : PrioritySortedIDs)
	{
		const TWeakObjectPtr<UObject>* WeakPtr = RegisteredSaveables.Find(Pair.Value);
		if (WeakPtr && WeakPtr->IsValid())
		{
			UObject* Obj = WeakPtr->Get();
			if (ISaveableInterface::Execute_IsDirty(Obj))
			{
				Result.Add(Obj);
			}
		}
	}
	return Result;
}

UObject* USaveableRegistrySubsystem::GetSaveableByID(const FString& SaveID) const
{
	const TWeakObjectPtr<UObject>* WeakPtr = RegisteredSaveables.Find(SaveID);
	if (WeakPtr && WeakPtr->IsValid())
	{
		return WeakPtr->Get();
	}
	return nullptr;
}

int32 USaveableRegistrySubsystem::GetSaveableCount() const
{
	return RegisteredSaveables.Num();
}

void USaveableRegistrySubsystem::MarkAllClean()
{
	for (auto& Pair : RegisteredSaveables)
	{
		if (Pair.Value.IsValid())
		{
			ISaveableInterface::Execute_ClearDirty(Pair.Value.Get());
		}
	}
}

void USaveableRegistrySubsystem::RebuildPriorityCache()
{
	PrioritySortedIDs.Empty(RegisteredSaveables.Num());

	for (const auto& Pair : RegisteredSaveables)
	{
		if (Pair.Value.IsValid())
		{
			const int32 Priority = ISaveableInterface::Execute_GetSavePriority(Pair.Value.Get());
			PrioritySortedIDs.Emplace(Priority, Pair.Key);
		}
	}

	// Sort ascending: lower priority number = loads first
	PrioritySortedIDs.Sort([](const TPair<int32, FString>& A, const TPair<int32, FString>& B)
	{
		return A.Key < B.Key;
	});
}

TArray<UObject*> USaveableRegistrySubsystem::GetSortedObjects() const
{
	TArray<UObject*> Result;
	Result.Reserve(PrioritySortedIDs.Num());

	for (const TPair<int32, FString>& Pair : PrioritySortedIDs)
	{
		const TWeakObjectPtr<UObject>* WeakPtr = RegisteredSaveables.Find(Pair.Value);
		if (WeakPtr && WeakPtr->IsValid())
		{
			Result.Add(WeakPtr->Get());
		}
	}

	return Result;
}
