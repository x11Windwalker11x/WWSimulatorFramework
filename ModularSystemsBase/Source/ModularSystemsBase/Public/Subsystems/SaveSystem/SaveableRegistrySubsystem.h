// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Delegates/ModularSaveGameSystem/SaveDelegates.h"
#include "SaveableRegistrySubsystem.generated.h"

class ISaveableInterface;

/**
 * USaveableRegistrySubsystem — Central registry for all saveable objects.
 * Lives in MSB (L0.5) so any L2 plugin can register without cross-plugin deps.
 *
 * Objects implementing ISaveableInterface register here in BeginPlay
 * and unregister in EndPlay. The save system queries this registry
 * to discover what needs saving.
 *
 * Provides priority-sorted iteration (Rule #39) and dirty-only queries (Rule #40).
 */
UCLASS()
class MODULARSYSTEMSBASE_API USaveableRegistrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// === LIFECYCLE ===

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Get registry from any world context */
	UFUNCTION(BlueprintPure, Category = "Save System|Registry", meta = (WorldContext = "WorldContextObject"))
	static USaveableRegistrySubsystem* Get(const UObject* WorldContextObject);

	// === REGISTRATION ===

	/** Register a saveable object. Validates ISaveableInterface, stores by SaveID.
	 *  @return true if successfully registered */
	UFUNCTION(BlueprintCallable, Category = "Save System|Registry")
	bool RegisterSaveable(UObject* SaveableObject);

	/** Unregister a saveable object by its SaveID.
	 *  @return true if found and removed */
	UFUNCTION(BlueprintCallable, Category = "Save System|Registry")
	bool UnregisterSaveable(const FString& SaveID);

	// === QUERIES ===

	/** Get all registered saveables, sorted by priority ascending (lower = first) */
	UFUNCTION(BlueprintCallable, Category = "Save System|Registry")
	TArray<UObject*> GetAllSaveables() const;

	/** Get only dirty saveables, sorted by priority ascending */
	UFUNCTION(BlueprintCallable, Category = "Save System|Registry")
	TArray<UObject*> GetDirtySaveables() const;

	/** Find a specific saveable by its SaveID */
	UFUNCTION(BlueprintCallable, Category = "Save System|Registry")
	UObject* GetSaveableByID(const FString& SaveID) const;

	/** Get count of registered saveables */
	UFUNCTION(BlueprintPure, Category = "Save System|Registry")
	int32 GetSaveableCount() const;

	/** Call ClearDirty() on all registered saveables */
	UFUNCTION(BlueprintCallable, Category = "Save System|Registry")
	void MarkAllClean();

	// === DELEGATES ===

	UPROPERTY(BlueprintAssignable, Category = "Save System|Registry")
	FOnSaveableRegistered OnSaveableRegistered;

	UPROPERTY(BlueprintAssignable, Category = "Save System|Registry")
	FOnSaveableUnregistered OnSaveableUnregistered;

private:
	/** SaveID → Weak reference to saveable object */
	UPROPERTY()
	TMap<FString, TWeakObjectPtr<UObject>> RegisteredSaveables;

	/** Cached priority-sorted list: (Priority, SaveID) pairs */
	TArray<TPair<int32, FString>> PrioritySortedIDs;

	/** Rebuild the priority cache after register/unregister */
	void RebuildPriorityCache();

	/** Get objects from priority-sorted IDs, filtering out stale weak refs */
	TArray<UObject*> GetSortedObjects() const;
};
