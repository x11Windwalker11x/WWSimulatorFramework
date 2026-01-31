// ManipulationHandler.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/MiniGameHandlerBase.h"
#include "Lib/Data/SimulatorFramework/MiniGameData.h"
#include "ManipulationHandler.generated.h"

/**
 * Handler for 6-axis manipulation mini-games
 * Grab, move, rotate, snap mechanics (assembly, placement)
 */
UCLASS(BlueprintType, Blueprintable)
class SIMULATORFRAMEWORK_API UManipulationHandler : public UMiniGameHandlerBase
{
    GENERATED_BODY()

public:
    // === LIFECYCLE ===

    virtual void Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID) override;
    virtual void Activate() override;
    virtual void Deactivate() override;
    virtual void TickHandler(float DeltaTime) override;
    virtual bool NeedsTick() const override { return true; }

    // === INPUT ===

    virtual void ProcessAxisInput(const FVector2D& AxisValue, float DeltaTime) override;
    virtual void ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed) override;
    virtual void ProcessPositionalInput(const FVector& WorldPosition, const FVector& WorldNormal) override;

    // === MANIPULATION STATE ===

    /** Get currently held actor */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Manipulation")
    AActor* GetHeldActor() const { return HeldActor.Get(); }

    /** Check if currently holding an object */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Manipulation")
    bool IsHolding() const { return HeldActor.IsValid(); }

    /** Get current snap target (if any) */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Manipulation")
    const FSnapPointData& GetCurrentSnapTarget() const { return CurrentSnapTarget; }

    /** Check if currently snapping */
    UFUNCTION(BlueprintPure, Category = "MiniGame|Manipulation")
    bool IsNearSnapPoint() const { return bNearSnapPoint; }

protected:
    // === CONFIG ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    FManipulationConfig ManipConfig;

    // === STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    TWeakObjectPtr<AActor> HeldActor;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    FVector HeldLocalOffset;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    FRotator HeldLocalRotation;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    float CurrentHoldDistance;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    FVector CurrentPosition;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    FRotator CurrentRotation;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    bool bNearSnapPoint = false;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    FSnapPointData CurrentSnapTarget;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    TArray<FSnapPointData> AvailableSnapPoints;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    int32 SnappedCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|Manipulation")
    int32 TotalSnapPoints = 0;

    // === INTERNAL ===

    /** Apply movement input */
    void ApplyMovement(const FVector2D& Input, float DeltaTime);

    /** Apply rotation input */
    void ApplyRotation(const FVector2D& Input, float DeltaTime);

    /** Update held actor transform */
    void UpdateHeldTransform();

    /** Check for nearby snap points */
    void CheckSnapPoints();

    /** Execute snap to current target */
    void ExecuteSnap();

    /** Try to grab actor at position */
    bool TryGrab(AActor* Target);

    /** Release currently held actor */
    void ReleaseHeld(bool bSnapped);

    /** Get view transform for reference calculations */
    FTransform GetViewTransform() const;

    /** Load snap points from station */
    void LoadSnapPoints();

    /** Report snap objective */
    void ReportSnapProgress();
};