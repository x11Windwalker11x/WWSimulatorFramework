// ObjectiveTrackerSubsystem.cpp
#include "Subsystems/ModularQuestSystem/ObjectiveTrackerSubsystem.h"
#include "Engine/World.h"

void UObjectiveTrackerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UObjectiveTrackerSubsystem::Deinitialize()
{
    TrackedSets.Empty();
    Super::Deinitialize();
}

// === REGISTRATION ===

FGuid UObjectiveTrackerSubsystem::RegisterObjectiveSet(const FObjectiveSet& Definition)
{
    FTrackedObjectiveSet NewSet;
    NewSet.Initialize(Definition);
    
    TrackedSets.Add(NewSet.SetID, NewSet);
    return NewSet.SetID;
}

void UObjectiveTrackerSubsystem::UnregisterObjectiveSet(const FGuid& SetID)
{
    TrackedSets.Remove(SetID);
}

bool UObjectiveTrackerSubsystem::IsSetRegistered(const FGuid& SetID) const
{
    return TrackedSets.Contains(SetID);
}

// === UPDATES ===

void UObjectiveTrackerSubsystem::SetObjectiveValue(const FGuid& SetID, const FGameplayTag& ObjectiveTag, float Value)
{
    UpdateObjectiveInternal(SetID, ObjectiveTag, Value);
}

void UObjectiveTrackerSubsystem::AddObjectiveValue(const FGuid& SetID, const FGameplayTag& ObjectiveTag, float Delta)
{
    float Current = GetObjectiveValue(SetID, ObjectiveTag);
    UpdateObjectiveInternal(SetID, ObjectiveTag, Current + Delta);
}

void UObjectiveTrackerSubsystem::CompleteObjective(const FGuid& SetID, const FGameplayTag& ObjectiveTag)
{
    FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    if (!Set) return;

    const FObjectiveEntry* Entry = Set->Definition.FindEntry(ObjectiveTag);
    if (!Entry) return;

    UpdateObjectiveInternal(SetID, ObjectiveTag, Entry->Condition.TargetValue);
}

void UObjectiveTrackerSubsystem::ResetObjective(const FGuid& SetID, const FGameplayTag& ObjectiveTag)
{
    FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    if (!Set) return;

    int32 Index = Set->Definition.FindEntryIndex(ObjectiveTag);
    if (Index != INDEX_NONE && Set->States.IsValidIndex(Index))
    {
        Set->States[Index].Reset();
        OnObjectiveUpdated.Broadcast(SetID, ObjectiveTag, 0.0f);
    }
}

void UObjectiveTrackerSubsystem::ResetObjectiveSet(const FGuid& SetID)
{
    FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    if (!Set) return;

    Set->Reset();

    for (const FObjectiveEntry& Entry : Set->Definition.Entries)
    {
        OnObjectiveUpdated.Broadcast(SetID, Entry.Condition.ObjectiveTag, 0.0f);
    }
}

void UObjectiveTrackerSubsystem::FailObjectiveSet(const FGuid& SetID)
{
    if (!TrackedSets.Contains(SetID)) return;
    
    OnObjectiveSetFailed.Broadcast(SetID);
}

// === QUERIES ===

float UObjectiveTrackerSubsystem::GetObjectiveValue(const FGuid& SetID, const FGameplayTag& ObjectiveTag) const
{
    const FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    if (!Set) return 0.0f;

    int32 Index = Set->Definition.FindEntryIndex(ObjectiveTag);
    if (Index != INDEX_NONE && Set->States.IsValidIndex(Index))
    {
        return Set->States[Index].CurrentValue;
    }
    return 0.0f;
}

bool UObjectiveTrackerSubsystem::IsObjectiveMet(const FGuid& SetID, const FGameplayTag& ObjectiveTag) const
{
    const FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    if (!Set) return false;

    int32 Index = Set->Definition.FindEntryIndex(ObjectiveTag);
    if (Index != INDEX_NONE && Set->States.IsValidIndex(Index))
    {
        return Set->States[Index].bIsMet;
    }
    return false;
}

bool UObjectiveTrackerSubsystem::AreMandatoriesMet(const FGuid& SetID) const
{
    const FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    return Set ? Set->AreMandatoriesMet() : false;
}

bool UObjectiveTrackerSubsystem::IsSetComplete(const FGuid& SetID) const
{
    const FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    return Set ? Set->IsComplete() : false;
}

bool UObjectiveTrackerSubsystem::HasBonusReward(const FGuid& SetID) const
{
    const FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    return Set ? Set->HasBonusReward() : false;
}

float UObjectiveTrackerSubsystem::GetSetProgress(const FGuid& SetID) const
{
    const FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    return Set ? Set->GetProgress() : 0.0f;
}

const FTrackedObjectiveSet* UObjectiveTrackerSubsystem::GetTrackedSet(const FGuid& SetID) const
{
    return TrackedSets.Find(SetID);
}

// === INTERNAL ===

void UObjectiveTrackerSubsystem::UpdateObjectiveInternal(const FGuid& SetID, const FGameplayTag& ObjectiveTag, float NewValue)
{
    FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    if (!Set) return;

    bool bNewlyMet = Set->UpdateObjective(ObjectiveTag, NewValue, GetWorldTime());

    OnObjectiveUpdated.Broadcast(SetID, ObjectiveTag, NewValue);

    if (bNewlyMet)
    {
        int32 Index = Set->Definition.FindEntryIndex(ObjectiveTag);
        if (Index != INDEX_NONE)
        {
            OnObjectiveMet.Broadcast(SetID, ObjectiveTag, Set->States[Index].MetTimestamp);
        }
        CheckCompletion(SetID);
    }
}

void UObjectiveTrackerSubsystem::CheckCompletion(const FGuid& SetID)
{
    const FTrackedObjectiveSet* Set = TrackedSets.Find(SetID);
    if (!Set) return;

    if (Set->IsComplete())
    {
        OnObjectiveSetComplete.Broadcast(SetID, Set->HasBonusReward());
    }
}

float UObjectiveTrackerSubsystem::GetWorldTime() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetTimeSeconds();
    }
    return 0.0f;
}