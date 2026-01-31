// ObjectiveData.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lib/Enums/WW_Enums.h"
#include "ObjectiveData.generated.h"

/**
 * Single objective condition check
 * Atomic struct - defines one measurable goal
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FObjectiveCondition
{
    GENERATED_BODY()

    /** Tag identifying this objective */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag ObjectiveTag;

    /** Comparison operator */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECompareOp Operator = ECompareOp::GreaterEqual;

    /** Target value to reach */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue = 1.0f;

    /** Secondary value for InRange operator */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Operator == ECompareOp::InRange"))
    float TargetValueMax = 1.0f;

    bool IsValid() const { return ObjectiveTag.IsValid(); }

    /** Evaluate condition against current value */
    bool Evaluate(float CurrentValue) const
    {
        switch (Operator)
        {
            case ECompareOp::Equal:        return FMath::IsNearlyEqual(CurrentValue, TargetValue, KINDA_SMALL_NUMBER);
            case ECompareOp::NotEqual:     return !FMath::IsNearlyEqual(CurrentValue, TargetValue, KINDA_SMALL_NUMBER);
            case ECompareOp::Greater:      return CurrentValue > TargetValue;
            case ECompareOp::GreaterEqual: return CurrentValue >= TargetValue;
            case ECompareOp::Less:         return CurrentValue < TargetValue;
            case ECompareOp::LessEqual:    return CurrentValue <= TargetValue;
            case ECompareOp::InRange:      return CurrentValue >= TargetValue && CurrentValue <= TargetValueMax;
            default:                       return false;
        }
    }
};

/**
 * Objective entry with mandatory/optional flag
 * Wraps condition with completion rules
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FObjectiveEntry
{
    GENERATED_BODY()

    /** The condition to evaluate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FObjectiveCondition Condition;

    /** If true, must complete for success. If false, optional/bonus */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMandatory = true;

    /** Weight for "complete N of M" optional pools */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", EditCondition = "!bIsMandatory"))
    int32 Weight = 1;

    /** Optional grouping tag for logical groups */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag GroupTag;

    bool IsValid() const { return Condition.IsValid(); }
    FGameplayTag GetObjectiveTag() const { return Condition.ObjectiveTag; }
};

/**
 * Runtime state for a single objective
 * NOT for DataTables - runtime only
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FObjectiveState
{
    GENERATED_BODY()

    /** Current progress value */
    UPROPERTY(BlueprintReadOnly)
    float CurrentValue = 0.0f;

    /** Has this objective been met */
    UPROPERTY(BlueprintReadOnly)
    bool bIsMet = false;

    /** Timestamp when objective was met (0 if not met) */
    UPROPERTY(BlueprintReadOnly)
    float MetTimestamp = 0.0f;

    void Reset()
    {
        CurrentValue = 0.0f;
        bIsMet = false;
        MetTimestamp = 0.0f;
    }
};

/**
 * Collection of objectives with completion rules
 * Used by: MiniGame, Quest systems
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FObjectiveSet
{
    GENERATED_BODY()

    /** All objectives in this set */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FObjectiveEntry> Entries;

    /** How many optional objectives required for success (0 = none required) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
    int32 OptionalRequiredCount = 0;

    /** How many optional objectives needed for bonus reward */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
    int32 OptionalBonusThreshold = 0;

    bool IsValid() const { return Entries.Num() > 0; }
    int32 GetEntryCount() const { return Entries.Num(); }

    /** Get count of mandatory entries */
    int32 GetMandatoryCount() const
    {
        int32 Count = 0;
        for (const FObjectiveEntry& Entry : Entries)
        {
            if (Entry.bIsMandatory) Count++;
        }
        return Count;
    }

    /** Get count of optional entries */
    int32 GetOptionalCount() const
    {
        return Entries.Num() - GetMandatoryCount();
    }

    /** Get total weight of optional entries */
    int32 GetOptionalTotalWeight() const
    {
        int32 TotalWeight = 0;
        for (const FObjectiveEntry& Entry : Entries)
        {
            if (!Entry.bIsMandatory) TotalWeight += Entry.Weight;
        }
        return TotalWeight;
    }

    /** Find entry by objective tag */
    const FObjectiveEntry* FindEntry(const FGameplayTag& ObjectiveTag) const
    {
        for (const FObjectiveEntry& Entry : Entries)
        {
            if (Entry.Condition.ObjectiveTag == ObjectiveTag)
            {
                return &Entry;
            }
        }
        return nullptr;
    }

    /** Find entry index by objective tag (-1 if not found) */
    int32 FindEntryIndex(const FGameplayTag& ObjectiveTag) const
    {
        for (int32 i = 0; i < Entries.Num(); i++)
        {
            if (Entries[i].Condition.ObjectiveTag == ObjectiveTag)
            {
                return i;
            }
        }
        return INDEX_NONE;
    }
};

/**
 * Runtime tracked objective set with state
 * NOT for DataTables - runtime only
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FTrackedObjectiveSet
{
    GENERATED_BODY()

    /** The objective definitions */
    UPROPERTY(BlueprintReadOnly)
    FObjectiveSet Definition;

    /** Runtime state per objective (parallel array to Definition.Entries) */
    UPROPERTY(BlueprintReadOnly)
    TArray<FObjectiveState> States;

    /** Unique ID for this tracked set */
    UPROPERTY(BlueprintReadOnly)
    FGuid SetID;

    /** Initialize states from definition */
    void Initialize(const FObjectiveSet& InDefinition)
    {
        Definition = InDefinition;
        States.SetNum(Definition.Entries.Num());
        for (FObjectiveState& State : States)
        {
            State.Reset();
        }
        SetID = FGuid::NewGuid();
    }

    /** Reset all states */
    void Reset()
    {
        for (FObjectiveState& State : States)
        {
            State.Reset();
        }
    }

    /** Update objective value and check if met */
    bool UpdateObjective(const FGameplayTag& ObjectiveTag, float NewValue, float WorldTime)
    {
        int32 Index = Definition.FindEntryIndex(ObjectiveTag);
        if (Index == INDEX_NONE || !States.IsValidIndex(Index)) return false;

        FObjectiveState& State = States[Index];
        const FObjectiveEntry& Entry = Definition.Entries[Index];

        State.CurrentValue = NewValue;
        
        bool bWasMet = State.bIsMet;
        State.bIsMet = Entry.Condition.Evaluate(NewValue);
        
        if (State.bIsMet && !bWasMet)
        {
            State.MetTimestamp = WorldTime;
            return true; // Newly met
        }
        
        return false;
    }

    /** Check if all mandatory objectives are met */
    bool AreMandatoriesMet() const
    {
        for (int32 i = 0; i < Definition.Entries.Num(); i++)
        {
            if (Definition.Entries[i].bIsMandatory && !States[i].bIsMet)
            {
                return false;
            }
        }
        return true;
    }

    /** Get count of completed optional objectives */
    int32 GetOptionalCompletedCount() const
    {
        int32 Count = 0;
        for (int32 i = 0; i < Definition.Entries.Num(); i++)
        {
            if (!Definition.Entries[i].bIsMandatory && States[i].bIsMet)
            {
                Count++;
            }
        }
        return Count;
    }

    /** Get total weight of completed optional objectives */
    int32 GetOptionalCompletedWeight() const
    {
        int32 Weight = 0;
        for (int32 i = 0; i < Definition.Entries.Num(); i++)
        {
            if (!Definition.Entries[i].bIsMandatory && States[i].bIsMet)
            {
                Weight += Definition.Entries[i].Weight;
            }
        }
        return Weight;
    }

    /** Check if set is complete (all mandatory + required optionals) */
    bool IsComplete() const
    {
        if (!AreMandatoriesMet()) return false;
        
        if (Definition.OptionalRequiredCount > 0)
        {
            return GetOptionalCompletedCount() >= Definition.OptionalRequiredCount;
        }
        
        return true;
    }

    /** Check if bonus threshold reached */
    bool HasBonusReward() const
    {
        if (Definition.OptionalBonusThreshold <= 0) return false;
        return GetOptionalCompletedCount() >= Definition.OptionalBonusThreshold;
    }

    /** Get overall progress (0-1) */
    float GetProgress() const
    {
        if (States.Num() == 0) return 0.0f;

        int32 MetCount = 0;
        for (const FObjectiveState& State : States)
        {
            if (State.bIsMet) MetCount++;
        }
        
        return static_cast<float>(MetCount) / static_cast<float>(States.Num());
    }
};