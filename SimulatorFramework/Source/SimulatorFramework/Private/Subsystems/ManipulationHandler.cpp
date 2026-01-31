// ManipulationHandler.cpp
#include "Subsystems/ManipulationHandler.h"
#include "Components/MiniGameComponent.h"
#include "Interfaces/SimulatorFramework/MiniGameStationInterface.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Lib/Data/Tags//WW_TagLibrary.h"

void UManipulationHandler::Initialize(UMiniGameComponent* InOwner, const FMiniGameData& InConfig, const FGuid& InObjectiveSetID)
{
    Super::Initialize(InOwner, InConfig, InObjectiveSetID);
    ManipConfig = InConfig.ManipulationConfig;
}

void UManipulationHandler::Activate()
{
    Super::Activate();

    // Reset state
    HeldActor.Reset();
    CurrentHoldDistance = (ManipConfig.HoldDistanceMin + ManipConfig.HoldDistanceMax) * 0.5f;
    CurrentPosition = FVector::ZeroVector;
    CurrentRotation = FRotator::ZeroRotator;
    bNearSnapPoint = false;
    SnappedCount = 0;

    // Load snap points from station
    LoadSnapPoints();
    TotalSnapPoints = AvailableSnapPoints.Num();
}

void UManipulationHandler::Deactivate()
{
    if (HeldActor.IsValid())
    {
        ReleaseHeld(false);
    }
    Super::Deactivate();
}

void UManipulationHandler::TickHandler(float DeltaTime)
{
    Super::TickHandler(DeltaTime);

    if (!bIsActive) return;

    if (IsHolding())
    {
        UpdateHeldTransform();
        CheckSnapPoints();
    }
}

void UManipulationHandler::ProcessAxisInput(const FVector2D& AxisValue, float DeltaTime)
{
    if (!IsHolding()) return;

    // Movement uses raw axis, rotation uses modified
    if (ManipConfig.HasMovement())
    {
        ApplyMovement(AxisValue, DeltaTime);
    }

    if (ManipConfig.HasRotation())
    {
        ApplyRotation(AxisValue, DeltaTime);
    }
}

void UManipulationHandler::ProcessActionInput(const FGameplayTag& ActionTag, bool bPressed)
{
    // Primary action = grab/release/snap
    if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Primary"))))
    {
        if (bPressed)
        {
            if (IsHolding())
            {
                if (bNearSnapPoint)
                {
                    ExecuteSnap();
                }
                else if (ManipConfig.bAllowDrop)
                {
                    ReleaseHeld(false);
                }
            }
        }
    }
    // Secondary = cancel/drop
    else if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Secondary"))))
    {
        if (bPressed && IsHolding() && ManipConfig.bAllowDrop)
        {
            ReleaseHeld(false);
        }
    }
    // Scroll = adjust distance
    else if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.ScrollUp"))))
    {
        if (bPressed && IsHolding())
        {
            CurrentHoldDistance = FMath::Clamp(
                CurrentHoldDistance + 10.0f,
                ManipConfig.HoldDistanceMin,
                ManipConfig.HoldDistanceMax
            );
        }
    }
    else if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.ScrollDown"))))
    {
        if (bPressed && IsHolding())
        {
            CurrentHoldDistance = FMath::Clamp(
                CurrentHoldDistance - 10.0f,
                ManipConfig.HoldDistanceMin,
                ManipConfig.HoldDistanceMax
            );
        }
    }
}

void UManipulationHandler::ProcessPositionalInput(const FVector& WorldPosition, const FVector& WorldNormal)
{
    // Used for trace-based grab targeting
    // Component should trace and call this with hit result
}

void UManipulationHandler::ApplyMovement(const FVector2D& Input, float DeltaTime)
{
    FTransform ViewTransform = GetViewTransform();
    FVector Delta = FVector::ZeroVector;

    // X axis (left/right)
    if (ManipConfig.MovementX.bEnabled)
    {
        float Value = Input.X * ManipConfig.MovementX.Sensitivity * DeltaTime * 100.0f;
        if (ManipConfig.MovementX.bInvert) Value = -Value;

        FVector Right = (ManipConfig.MovementX.Reference == EAxisReference::View)
            ? ViewTransform.GetUnitAxis(EAxis::Y)
            : FVector::RightVector;
        Delta += Right * Value;
    }

    // Y axis (forward/back or up/down depending on config)
    if (ManipConfig.MovementY.bEnabled)
    {
        float Value = Input.Y * ManipConfig.MovementY.Sensitivity * DeltaTime * 100.0f;
        if (ManipConfig.MovementY.bInvert) Value = -Value;

        FVector Forward = (ManipConfig.MovementY.Reference == EAxisReference::View)
            ? ViewTransform.GetUnitAxis(EAxis::X)
            : FVector::ForwardVector;
        Delta += Forward * Value;
    }

    CurrentPosition += Delta;

    // Clamp if enabled
    if (ManipConfig.MovementX.bClamp)
    {
        CurrentPosition.Y = FMath::Clamp(CurrentPosition.Y, ManipConfig.MovementX.ClampMin, ManipConfig.MovementX.ClampMax);
    }
    if (ManipConfig.MovementY.bClamp)
    {
        CurrentPosition.X = FMath::Clamp(CurrentPosition.X, ManipConfig.MovementY.ClampMin, ManipConfig.MovementY.ClampMax);
    }
}

void UManipulationHandler::ApplyRotation(const FVector2D& Input, float DeltaTime)
{
    // Yaw from X input
    if (ManipConfig.RotationYaw.bEnabled)
    {
        float Value = Input.X * ManipConfig.RotationYaw.Sensitivity * DeltaTime * 100.0f;
        if (ManipConfig.RotationYaw.bInvert) Value = -Value;
        CurrentRotation.Yaw += Value;

        if (ManipConfig.RotationYaw.bClamp)
        {
            CurrentRotation.Yaw = FMath::Clamp(CurrentRotation.Yaw, ManipConfig.RotationYaw.ClampMin, ManipConfig.RotationYaw.ClampMax);
        }
    }

    // Pitch from Y input
    if (ManipConfig.RotationPitch.bEnabled)
    {
        float Value = Input.Y * ManipConfig.RotationPitch.Sensitivity * DeltaTime * 100.0f;
        if (ManipConfig.RotationPitch.bInvert) Value = -Value;
        CurrentRotation.Pitch += Value;

        if (ManipConfig.RotationPitch.bClamp)
        {
            CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch, ManipConfig.RotationPitch.ClampMin, ManipConfig.RotationPitch.ClampMax);
        }
    }
}

void UManipulationHandler::UpdateHeldTransform()
{
    AActor* Held = HeldActor.Get();
    if (!Held) return;

    FTransform ViewTransform = GetViewTransform();
    
    // Calculate world position
    FVector TargetPos = ViewTransform.GetLocation() + ViewTransform.GetUnitAxis(EAxis::X) * CurrentHoldDistance;
    TargetPos += CurrentPosition;

    // Apply rotation
    FRotator TargetRot = ViewTransform.GetRotation().Rotator() + CurrentRotation + HeldLocalRotation;

    // Smooth interpolation
    FVector CurrentPos = Held->GetActorLocation();
    FRotator CurrentRot = Held->GetActorRotation();

    FVector NewPos = FMath::VInterpTo(CurrentPos, TargetPos, GetWorld()->GetDeltaSeconds(), 15.0f);
    FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), 15.0f);

    Held->SetActorLocationAndRotation(NewPos, NewRot);
}

void UManipulationHandler::CheckSnapPoints()
{
    AActor* Held = HeldActor.Get();
    if (!Held) return;

    FVector HeldPos = Held->GetActorLocation();
    FRotator HeldRot = Held->GetActorRotation();

    bNearSnapPoint = false;
    float ClosestDist = MAX_FLT;

    for (const FSnapPointData& SnapPoint : AvailableSnapPoints)
    {
        if (SnapPoint.bIsOccupied) continue;

        // Get world position of snap point
        FVector SnapWorldPos = SnapPoint.LocalPosition; // TODO: Transform by station actor

        float Dist = FVector::Dist(HeldPos, SnapWorldPos);
        float SnapDist = (SnapPoint.SnapDistanceOverride > 0.0f) 
            ? SnapPoint.SnapDistanceOverride 
            : ManipConfig.SnapSettings.SnapDistance;

        if (Dist < SnapDist && Dist < ClosestDist)
        {
            // Check rotation if required
            if (ManipConfig.SnapSettings.bRequireRotationMatch)
            {
                float RotDiff = FMath::Abs(FRotator::NormalizeAxis(HeldRot.Yaw - SnapPoint.RequiredRotation.Yaw));
                if (RotDiff > ManipConfig.SnapSettings.RotationTolerance)
                {
                    continue;
                }
            }

            bNearSnapPoint = true;
            CurrentSnapTarget = SnapPoint;
            ClosestDist = Dist;
        }
    }
}

void UManipulationHandler::ExecuteSnap()
{
    if (!bNearSnapPoint || !HeldActor.IsValid()) return;

    AActor* Held = HeldActor.Get();

    // Snap to position
    FVector SnapWorldPos = CurrentSnapTarget.LocalPosition; // TODO: Transform by station
    FRotator SnapWorldRot = CurrentSnapTarget.RequiredRotation;

    Held->SetActorLocationAndRotation(SnapWorldPos, SnapWorldRot);

    // Mark snap point occupied
    for (FSnapPointData& SP : AvailableSnapPoints)
    {
        if (SP.SnapPointTag == CurrentSnapTarget.SnapPointTag)
        {
            SP.bIsOccupied = true;
            SP.SnappedActor = Held;
            break;
        }
    }

    SnappedCount++;
    ReleaseHeld(true);

    // Report objective
    if (CurrentSnapTarget.OnSnapObjectiveTag.IsValid())
    {
        ReportObjectiveComplete(CurrentSnapTarget.OnSnapObjectiveTag);
    }

    ReportSnapProgress();

    // Check completion
    if (TotalSnapPoints > 0 && SnappedCount >= TotalSnapPoints)
    {
        ReportObjectiveComplete(FWWTagLibrary::MiniGame_Objective_AllPartsAssembled());
    }
}

bool UManipulationHandler::TryGrab(AActor* Target)
{
    if (!Target || IsHolding()) return false;

    HeldActor = Target;
    HeldLocalOffset = FVector::ZeroVector;
    HeldLocalRotation = FRotator::ZeroRotator;
    CurrentPosition = FVector::ZeroVector;
    CurrentRotation = FRotator::ZeroRotator;

    // TODO: Disable physics on held actor

    return true;
}

void UManipulationHandler::ReleaseHeld(bool bSnapped)
{
    if (!HeldActor.IsValid()) return;

    // TODO: Re-enable physics if not snapped

    HeldActor.Reset();
    bNearSnapPoint = false;
}

FTransform UManipulationHandler::GetViewTransform() const
{
    if (UMiniGameComponent* Owner = OwnerComponent.Get())
    {
        if (AActor* OwnerActor = Owner->GetOwner())
        {
            if (APawn* Pawn = Cast<APawn>(OwnerActor))
            {
                if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
                {
                    if (APlayerCameraManager* CM = PC->PlayerCameraManager)
                    {
                        return FTransform(CM->GetCameraRotation(), CM->GetCameraLocation());
                    }
                }
            }
        }
    }
    return FTransform::Identity;
}

void UManipulationHandler::LoadSnapPoints()
{
    AvailableSnapPoints.Empty();

    if (UMiniGameComponent* Owner = OwnerComponent.Get())
    {
        if (AActor* Station = Owner->GetCurrentStation())
        {
            if (Station->GetClass()->ImplementsInterface(UMiniGameStationInterface::StaticClass()))
            {
                AvailableSnapPoints = IMiniGameStationInterface::Execute_GetSnapPoints(Station);
            }
        }
    }
}

void UManipulationHandler::ReportSnapProgress()
{
    if (TotalSnapPoints > 0)
    {
        float Progress = static_cast<float>(SnappedCount) / static_cast<float>(TotalSnapPoints);
        ReportObjectiveValue(FWWTagLibrary::MiniGame_Objective_ItemSnapped(), Progress);
    }
}