#include "Components/PhysicsGrabComponent.h"
#include "Interfaces/SimulatorFramework/PhysicalInteractionInterface.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsGrab, Log, All);

UPhysicsGrabComponent::UPhysicsGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    SetIsReplicatedByDefault(true);
}

void UPhysicsGrabComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    if (Owner->GetClass()->ImplementsInterface(UPhysicalInteractionInterface::StaticClass()))
    {
        APawn* OwnerPawn = Owner->GetInstigatorController()->GetPawn();
        if (!OwnerPawn) return;
        
        // Only create physics handle on owning client and server
        if (OwnerPawn->IsLocallyControlled())
        {
            PhysicsHandle = NewObject<UPhysicsHandleComponent>(GetOwner());
            if (PhysicsHandle)
            {
                PhysicsHandle->RegisterComponent();
                PhysicsHandle->LinearDamping = LinearDamping;
                PhysicsHandle->AngularDamping = AngularDamping;
                PhysicsHandle->InterpolationSpeed = InterpolationSpeed;
            }
        }
    }

}

void UPhysicsGrabComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION(UPhysicsGrabComponent, HeldActor, COND_OwnerOnly);
}

void UPhysicsGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsHoldingObject()) return;

    // Local physics update
    UpdateHeldObjectLocation(DeltaTime);

    // Network update at reduced rate (server only)
    if (GetOwner()->HasAuthority())
    {
        NetworkUpdateAccumulator += DeltaTime;
        if (NetworkUpdateAccumulator >= NetworkUpdateRate)
        {
            NetworkUpdateAccumulator = 0.0f;
            Multicast_UpdateHeldLocation(GetHoldLocation());
        }
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

bool UPhysicsGrabComponent::TryGrab()
{
    if (IsHoldingObject()) return false;

    FHitResult Hit;
    if (!TraceForGrabbable(Hit)) return false;

    AActor* Target = Hit.GetActor();
    if (!ValidateGrabTarget(Target)) return false;

    // Server authoritative
    if (!GetOwner()->HasAuthority())
    {
        Server_TryGrab(Target);
        return true; // Optimistic
    }

    return Internal_TryGrab(Target);
}

void UPhysicsGrabComponent::Release()
{
    if (!IsHoldingObject()) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_Release();
        return;
    }

    Internal_Release();
}

void UPhysicsGrabComponent::Throw()
{
    if (!IsHoldingObject()) return;

    FVector ThrowDirection = GetHoldLocation() - GetOwner()->GetActorLocation();
    ThrowDirection.Normalize();

    if (!GetOwner()->HasAuthority())
    {
        Server_Throw(ThrowDirection);
        return;
    }

    Internal_Throw(ThrowDirection);
}

// ============================================================================
// QUERIES
// ============================================================================

AActor* UPhysicsGrabComponent::GetGrabCandidate() const
{
    FHitResult Hit;
    if (TraceForGrabbable(Hit))
    {
        AActor* Target = Hit.GetActor();
        if (ValidateGrabTarget(Target))
        {
            return Target;
        }
    }
    return nullptr;
}

// ============================================================================
// SERVER RPCs
// ============================================================================

bool UPhysicsGrabComponent::Server_TryGrab_Validate(AActor* TargetActor)
{
    return ValidateGrabTarget(TargetActor);
}

void UPhysicsGrabComponent::Server_TryGrab_Implementation(AActor* TargetActor)
{
    if (Internal_TryGrab(TargetActor))
    {
        Client_OnGrabConfirmed(TargetActor);
    }
    else
    {
        Client_OnGrabRejected(TEXT("Server rejected grab"));
    }
}

bool UPhysicsGrabComponent::Server_Release_Validate()
{
    return IsHoldingObject();
}

void UPhysicsGrabComponent::Server_Release_Implementation()
{
    Internal_Release();
}

bool UPhysicsGrabComponent::Server_Throw_Validate(FVector ThrowDirection)
{
    return IsHoldingObject() && !ThrowDirection.IsNearlyZero();
}

void UPhysicsGrabComponent::Server_Throw_Implementation(FVector ThrowDirection)
{
    Internal_Throw(ThrowDirection);
}

// ============================================================================
// CLIENT RPCs
// ============================================================================

void UPhysicsGrabComponent::Client_OnGrabConfirmed_Implementation(AActor* GrabbedActor)
{
    UE_LOG(LogPhysicsGrab, Log, TEXT("Grab confirmed: %s"), *GetNameSafe(GrabbedActor));
}

void UPhysicsGrabComponent::Client_OnGrabRejected_Implementation(const FString& Reason)
{
    UE_LOG(LogPhysicsGrab, Warning, TEXT("Grab rejected: %s"), *Reason);
    
    // Rollback any optimistic state
    if (PhysicsHandle)
    {
        PhysicsHandle->ReleaseComponent();
    }
    HeldActor.Reset();
    HeldComponent.Reset();
    SetComponentTickEnabled(false);
}

// ============================================================================
// MULTICAST
// ============================================================================

void UPhysicsGrabComponent::Multicast_UpdateHeldLocation_Implementation(FVector TargetLocation)
{
    // Non-owning clients update visual position
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    APawn* OwnerPawn = Owner->GetInstigatorController()->GetPawn();
    if (!OwnerPawn) return;
    if (OwnerPawn->IsLocallyControlled() && PhysicsHandle && IsHoldingObject())
    {
        PhysicsHandle->SetTargetLocation(TargetLocation);
    }
}

// ============================================================================
// REPLICATION
// ============================================================================

void UPhysicsGrabComponent::OnRep_HeldActor()
{
    if (HeldActor.IsValid())
    {
        // Setup local physics handle for other clients
        if (PhysicsHandle && HeldActor->GetClass()->ImplementsInterface(UPhysicalInteractionInterface::StaticClass()))
        {
            UPrimitiveComponent* GrabComp = IPhysicalInteractionInterface::Execute_GetGrabComponent(HeldActor.Get());
            if (GrabComp)
            {
                HeldComponent = GrabComp;
                PhysicsHandle->GrabComponentAtLocationWithRotation(
                    GrabComp, NAME_None, 
                    GrabComp->GetComponentLocation(), 
                    GrabComp->GetComponentRotation()
                );
                SetComponentTickEnabled(true);
            }
        }
        
        OnObjectGrabbed.Broadcast(HeldActor.Get());
    }
    else
    {
        if (PhysicsHandle)
        {
            PhysicsHandle->ReleaseComponent();
        }
        HeldComponent.Reset();
        SetComponentTickEnabled(false);
        
        OnObjectReleased.Broadcast(nullptr);
    }
}

// ============================================================================
// INTERNALS
// ============================================================================

bool UPhysicsGrabComponent::ValidateGrabTarget(AActor* Target) const
{
    if (!Target) return false;
    
    if (!Target->GetClass()->ImplementsInterface(UPhysicalInteractionInterface::StaticClass()))
    {
        return false;
    }
    
    if (!IPhysicalInteractionInterface::Execute_CanBeGrabbed(Target))
    {
        return false;
    }
    
    float Mass = IPhysicalInteractionInterface::Execute_GetGrabMass(Target);
    if (Mass > MaxHoldMass)
    {
        return false;
    }
    
    return true;
}

bool UPhysicsGrabComponent::Internal_TryGrab(AActor* TargetActor)
{
    if (!TargetActor || IsHoldingObject()) return false;
    if (!ValidateGrabTarget(TargetActor)) return false;

    UPrimitiveComponent* GrabComp = IPhysicalInteractionInterface::Execute_GetGrabComponent(TargetActor);
    if (!GrabComp || !GrabComp->IsSimulatingPhysics())
    {
        UE_LOG(LogPhysicsGrab, Warning, TEXT("No valid physics component"));
        return false;
    }

    // Grab with physics handle
    if (PhysicsHandle)
    {
        FVector GrabOffset = IPhysicalInteractionInterface::Execute_GetGrabOffset(TargetActor);
        FVector GrabLocation = GrabComp->GetComponentLocation() + GrabOffset;

        PhysicsHandle->GrabComponentAtLocationWithRotation(
            GrabComp, NAME_None,
            GrabLocation,
            GrabComp->GetComponentRotation()
        );
    }

    // Notify actor
    IPhysicalInteractionInterface::Execute_OnGrabbed(TargetActor, GetOwner());

    // Store state
    HeldActor = TargetActor;
    HeldComponent = GrabComp;

    SetComponentTickEnabled(true);

    OnObjectGrabbed.Broadcast(TargetActor);

    UE_LOG(LogPhysicsGrab, Log, TEXT("Grabbed: %s"), *TargetActor->GetName());
    return true;
}

void UPhysicsGrabComponent::Internal_Release()
{
    if (!IsHoldingObject()) return;

    AActor* Actor = HeldActor.Get();

    if (PhysicsHandle)
    {
        PhysicsHandle->ReleaseComponent();
    }

    if (Actor && Actor->GetClass()->ImplementsInterface(UPhysicalInteractionInterface::StaticClass()))
    {
        IPhysicalInteractionInterface::Execute_OnReleased(Actor, GetOwner());
    }

    OnObjectReleased.Broadcast(Actor);

    HeldActor.Reset();
    HeldComponent.Reset();

    SetComponentTickEnabled(false);

    UE_LOG(LogPhysicsGrab, Log, TEXT("Released"));
}

void UPhysicsGrabComponent::Internal_Throw(const FVector& ThrowDirection)
{
    if (!IsHoldingObject()) return;

    UPrimitiveComponent* Comp = HeldComponent.Get();
    AActor* Actor = HeldActor.Get();

    Internal_Release();

    if (Comp && Comp->IsSimulatingPhysics())
    {
        Comp->AddImpulse(ThrowDirection * ThrowForce, NAME_None, true);
    }

    OnObjectThrown.Broadcast(Actor);

    UE_LOG(LogPhysicsGrab, Log, TEXT("Threw"));
}

FVector UPhysicsGrabComponent::GetHoldLocation() const
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn) return GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * HoldDistance;

    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC || !PC->PlayerCameraManager) 
    {
        return Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * HoldDistance;
    }

    FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
    FVector CamFwd = PC->PlayerCameraManager->GetCameraRotation().Vector();

    return CamLoc + CamFwd * HoldDistance;
}

bool UPhysicsGrabComponent::TraceForGrabbable(FHitResult& OutHit) const
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn) return false;

    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC || !PC->PlayerCameraManager) return false;

    FVector Start = PC->PlayerCameraManager->GetCameraLocation();
    FVector End = Start + PC->PlayerCameraManager->GetCameraRotation().Vector() * GrabRange;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_PhysicsBody, Params);
}

void UPhysicsGrabComponent::UpdateHeldObjectLocation(float DeltaTime)
{
    if (!PhysicsHandle || !IsHoldingObject()) return;

    PhysicsHandle->SetTargetLocation(GetHoldLocation());
}