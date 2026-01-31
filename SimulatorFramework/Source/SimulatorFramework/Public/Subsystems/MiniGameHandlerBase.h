// MiniGameHandlerBase.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/SimulatorFramework/MiniGameData.h"
#include "MiniGameHandlerBase.generated.h"

class UMiniGameComponent;
class UObjectiveTrackerSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiniGameProgress, const FGameplayTag&, ObjectiveTag, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiniGameComplete, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiniGameFailed, const FString&, Reason);

/**
 * Abstract base class for mini-game logic handlers
 * Spawned by MiniGameComponent, processes input, reports objectives
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SIMULATORFRAMEWORK_API UMiniGameHandlerBase : public UObject
{
    GENERATED_BODY()

public:
    // === LIFECYCLE ===

    /** Initialize handler with config and owning component */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Handler")
    virtual void Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID);

    /** Activate handler (start processing input) */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Handler")
    virtual void Activate();

    /** Deactivate handler (stop processing, cleanup) */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Handler")
    virtual void Deactivate();

    /** Tick update (called by component if handler needs tick) */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Handler")
    virtual void TickHandler(float DeltaTime);

    /** Check if handler needs tick updates */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Handler")
    virtual bool NeedsTick() const { return false; }

    // === INPUT PROCESSING ===

    /** Process axis input (mouse delta, analog stick) */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Input")
    virtual void ProcessAxisInput(const FVector2D& AxisValue, float DeltaTime);

    /** Process action input (button press/release) */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Input")
    virtual void ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed);

    /** Process positional input (world location from trace) */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Input")
    virtual void ProcessPositionalInput(const FVector& WorldPosition, const FVector& WorldNormal);

    // === STATE ===

    /** Get overall progress (0-1) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|State")
    virtual float GetProgress() const;

    /** Check if mini-game is complete (success) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|State")
    virtual bool IsComplete() const;

    /** Check if mini-game has failed */
    UFUNCTION(BlueprintPure, Category = "MiniGame|State")
    virtual bool IsFailed() const;

    /** Check if handler is currently active */
    UFUNCTION(BlueprintPure, Category = "MiniGame|State")
    bool IsActive() const { return bIsActive; }

    /** Get failure reason (if failed) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|State")
    FString GetFailureReason() const { return FailureReason; }

    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Events")
    FOnMiniGameProgress OnProgress;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Events")
    FOnMiniGameComplete OnComplete;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Events")
    FOnMiniGameFailed OnFailed;

protected:
    // === REFERENCES ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Handler")
    TWeakObjectPtr<UMiniGameComponent> OwnerComponent;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Handler")
    FMiniGameData Config;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Handler")
    FGuid ObjectiveSetID;

    // === STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Handler")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Handler")
    bool bIsComplete = false;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Handler")
    bool bIsFailed = false;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Handler")
    FString FailureReason;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Handler")
    float ElapsedTime = 0.0f;

    // === HELPERS ===

    /** Get objective tracker subsystem */
    UObjectiveTrackerSubsystem* GetObjectiveTracker() const;

    /** Report objective progress to subsystem */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Handler")
    void ReportObjectiveValue(const FGameplayTag& ObjectiveTag, float Value);

    /** Report objective complete */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Handler")
    void ReportObjectiveComplete(const FGameplayTag& ObjectiveTag);

    /** Mark handler as complete */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Handler")
    void MarkComplete(bool bSuccess);

    /** Mark handler as failed */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Handler")
    void MarkFailed(const FString& Reason);

    /** Check timeout */
    void CheckTimeout(float DeltaTime);
};