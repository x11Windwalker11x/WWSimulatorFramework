// ObjectiveTrackerSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/Core/ObjectiveData.h"
#include "ObjectiveTrackerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, const FGuid&, SetID, const FGameplayTag&, ObjectiveTag, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveMet, const FGuid&, SetID, const FGameplayTag&, ObjectiveTag, float, MetTimestamp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveSetComplete, const FGuid&, SetID, bool, bBonusAchieved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveSetFailed, const FGuid&, SetID);

/**
 * Shared objective tracking for MiniGame and Quest systems
 * Owns runtime state; consumers register sets and listen to delegates
 */
UCLASS()
class MODULARSYSTEMSBASE_API UObjectiveTrackerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // === REGISTRATION ===

    /** Register objective set, returns unique SetID */
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    FGuid RegisterObjectiveSet(const FObjectiveSet& Definition);

    /** Unregister objective set */
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void UnregisterObjectiveSet(const FGuid& SetID);

    /** Check if set is registered */
    UFUNCTION(BlueprintPure, Category = "Objectives")
    bool IsSetRegistered(const FGuid& SetID) const;

    // === UPDATES ===

    /** Update objective value (absolute) */
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void SetObjectiveValue(const FGuid& SetID, const FGameplayTag& ObjectiveTag, float Value);

    /** Update objective value (additive) */
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void AddObjectiveValue(const FGuid& SetID, const FGameplayTag& ObjectiveTag, float Delta);

    /** Mark objective as complete (sets value to target) */
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void CompleteObjective(const FGuid& SetID, const FGameplayTag& ObjectiveTag);

    /** Reset single objective */
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void ResetObjective(const FGuid& SetID, const FGameplayTag& ObjectiveTag);

    /** Reset all objectives in set */
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void ResetObjectiveSet(const FGuid& SetID);

    /** Force fail the objective set */
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void FailObjectiveSet(const FGuid& SetID);

    // === QUERIES ===

    /** Get current value of objective */
    UFUNCTION(BlueprintPure, Category = "Objectives")
    float GetObjectiveValue(const FGuid& SetID, const FGameplayTag& ObjectiveTag) const;

    /** Check if specific objective is met */
    UFUNCTION(BlueprintPure, Category = "Objectives")
    bool IsObjectiveMet(const FGuid& SetID, const FGameplayTag& ObjectiveTag) const;

    /** Check if all mandatory objectives are met */
    UFUNCTION(BlueprintPure, Category = "Objectives")
    bool AreMandatoriesMet(const FGuid& SetID) const;

    /** Check if set is complete */
    UFUNCTION(BlueprintPure, Category = "Objectives")
    bool IsSetComplete(const FGuid& SetID) const;

    /** Check if bonus achieved */
    UFUNCTION(BlueprintPure, Category = "Objectives")
    bool HasBonusReward(const FGuid& SetID) const;

    /** Get overall progress (0-1) */
    UFUNCTION(BlueprintPure, Category = "Objectives")
    float GetSetProgress(const FGuid& SetID) const;

    /** Get tracked set by ID (returns nullptr if not found) */
    const FTrackedObjectiveSet* GetTrackedSet(const FGuid& SetID) const;

    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "Objectives")
    FOnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Objectives")
    FOnObjectiveMet OnObjectiveMet;

    UPROPERTY(BlueprintAssignable, Category = "Objectives")
    FOnObjectiveSetComplete OnObjectiveSetComplete;

    UPROPERTY(BlueprintAssignable, Category = "Objectives")
    FOnObjectiveSetFailed OnObjectiveSetFailed;

private:
    /** All active tracked sets */
    UPROPERTY()
    TMap<FGuid, FTrackedObjectiveSet> TrackedSets;

    /** Internal update with completion check */
    void UpdateObjectiveInternal(const FGuid& SetID, const FGameplayTag& ObjectiveTag, float NewValue);

    /** Check and broadcast completion if needed */
    void CheckCompletion(const FGuid& SetID);

    /** Get world time for timestamps */
    float GetWorldTime() const;
};