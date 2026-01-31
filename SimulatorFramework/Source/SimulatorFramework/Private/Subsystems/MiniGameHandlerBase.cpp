// MiniGameHandlerBase.cpp
#include "Subsystems/MiniGameHandlerBase.h"
#include "Components/MiniGameComponent.h"
#include "Subsystems/ModularQuestSystem/ObjectiveTrackerSubsystem.h"
#include "Engine/GameInstance.h"

void UMiniGameHandlerBase::Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID)
{
    OwnerComponent = InOwner;
    Config = InConfig;
    ObjectiveSetID = InObjectiveSetID;

    bIsActive = false;
    bIsComplete = false;
    bIsFailed = false;
    FailureReason.Empty();
    ElapsedTime = 0.0f;
}

void UMiniGameHandlerBase::Activate()
{
    bIsActive = true;
    ElapsedTime = 0.0f;
}

void UMiniGameHandlerBase::Deactivate()
{
    bIsActive = false;
}

void UMiniGameHandlerBase::TickHandler(float DeltaTime)
{
    if (!bIsActive || bIsComplete || bIsFailed) return;

    ElapsedTime += DeltaTime;
    CheckTimeout(DeltaTime);
}

void UMiniGameHandlerBase::ProcessAxisInput(const FVector2D& AxisValue, float DeltaTime)
{
    // Override in derived classes
}

void UMiniGameHandlerBase::ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed)
{
    // Override in derived classes
}

void UMiniGameHandlerBase::ProcessPositionalInput(const FVector& WorldPosition, const FVector& WorldNormal)
{
    // Override in derived classes
}

float UMiniGameHandlerBase::GetProgress() const
{
    if (UObjectiveTrackerSubsystem* Tracker = GetObjectiveTracker())
    {
        return Tracker->GetSetProgress(ObjectiveSetID);
    }
    return 0.0f;
}

bool UMiniGameHandlerBase::IsComplete() const
{
    return bIsComplete;
}

bool UMiniGameHandlerBase::IsFailed() const
{
    return bIsFailed;
}

UObjectiveTrackerSubsystem* UMiniGameHandlerBase::GetObjectiveTracker() const
{
    if (UMiniGameComponent* Owner = OwnerComponent.Get())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            if (UGameInstance* GI = World->GetGameInstance())
            {
                return GI->GetSubsystem<UObjectiveTrackerSubsystem>();
            }
        }
    }
    return nullptr;
}

void UMiniGameHandlerBase::ReportObjectiveValue(const FGameplayTag& ObjectiveTag, float Value)
{
    if (UObjectiveTrackerSubsystem* Tracker = GetObjectiveTracker())
    {
        Tracker->SetObjectiveValue(ObjectiveSetID, ObjectiveTag, Value);
        OnProgress.Broadcast(ObjectiveTag, Value);
    }
}

void UMiniGameHandlerBase::ReportObjectiveComplete(const FGameplayTag& ObjectiveTag)
{
    if (UObjectiveTrackerSubsystem* Tracker = GetObjectiveTracker())
    {
        Tracker->CompleteObjective(ObjectiveSetID, ObjectiveTag);

        // Check if set is now complete
        if (Tracker->IsSetComplete(ObjectiveSetID))
        {
            MarkComplete(true);
        }
    }
}

void UMiniGameHandlerBase::MarkComplete(bool bSuccess)
{
    if (bIsComplete || bIsFailed) return;

    bIsComplete = true;
    bIsActive = false;
    OnComplete.Broadcast(bSuccess);
}

void UMiniGameHandlerBase::MarkFailed(const FString& Reason)
{
    if (bIsComplete || bIsFailed) return;

    bIsFailed = true;
    bIsActive = false;
    FailureReason = Reason;
    OnFailed.Broadcast(Reason);

    if (UObjectiveTrackerSubsystem* Tracker = GetObjectiveTracker())
    {
        Tracker->FailObjectiveSet(ObjectiveSetID);
    }
}

void UMiniGameHandlerBase::CheckTimeout(float DeltaTime)
{
    if (Config.TimeoutSeconds > 0.0f && ElapsedTime >= Config.TimeoutSeconds)
    {
        MarkFailed(TEXT("Time expired"));
    }
}