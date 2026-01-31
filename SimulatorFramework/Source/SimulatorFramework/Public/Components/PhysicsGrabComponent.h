#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "PhysicsGrabComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectGrabbed, AActor*, GrabbedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectReleased, AActor*, ReleasedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectThrown, AActor*, ThrownActor);

/**
 * Handles physics-based grabbing of objects implementing IPhysicalInteractionInterface.
 * Server authoritative with client prediction for responsiveness.
 */
UCLASS(ClassGroup=(SimulatorFramework), meta=(BlueprintSpawnableComponent))
class SIMULATORFRAMEWORK_API UPhysicsGrabComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsGrabComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Config")
    float GrabRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Config")
    float HoldDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Config")
    float MaxHoldMass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Config")
    float ThrowForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
    float LinearDamping = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
    float AngularDamping = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
    float InterpolationSpeed = 10.0f;

    // ============================================================================
    // PUBLIC API
    // ============================================================================

    UFUNCTION(BlueprintCallable, Category = "Grab")
    bool TryGrab();

    UFUNCTION(BlueprintCallable, Category = "Grab")
    void Release();

    UFUNCTION(BlueprintCallable, Category = "Grab")
    void Throw();

    // ============================================================================
    // QUERIES
    // ============================================================================

    UFUNCTION(BlueprintPure, Category = "Grab")
    FORCEINLINE bool IsHoldingObject() const { return HeldActor.IsValid(); }

    UFUNCTION(BlueprintPure, Category = "Grab")
    FORCEINLINE AActor* GetHeldActor() const { return HeldActor.Get(); }

    UFUNCTION(BlueprintPure, Category = "Grab")
    AActor* GetGrabCandidate() const;

    // ============================================================================
    // EVENTS
    // ============================================================================

    UPROPERTY(BlueprintAssignable, Category = "Grab|Events")
    FOnObjectGrabbed OnObjectGrabbed;

    UPROPERTY(BlueprintAssignable, Category = "Grab|Events")
    FOnObjectReleased OnObjectReleased;

    UPROPERTY(BlueprintAssignable, Category = "Grab|Events")
    FOnObjectThrown OnObjectThrown;

    // ============================================================================
    // SERVER RPCs
    // ============================================================================

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TryGrab(AActor* TargetActor);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Release();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Throw(FVector ThrowDirection);

    // ============================================================================
    // CLIENT RPCs
    // ============================================================================

    UFUNCTION(Client, Reliable)
    void Client_OnGrabConfirmed(AActor* GrabbedActor);

    UFUNCTION(Client, Reliable)
    void Client_OnGrabRejected(const FString& Reason);

    // ============================================================================
    // MULTICAST
    // ============================================================================

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_UpdateHeldLocation(FVector TargetLocation);

protected:
    // ============================================================================
    // REPLICATED STATE
    // ============================================================================

    UPROPERTY(ReplicatedUsing=OnRep_HeldActor)
    TWeakObjectPtr<AActor> HeldActor;

    UFUNCTION()
    void OnRep_HeldActor();

    // ============================================================================
    // LOCAL STATE (Not replicated)
    // ============================================================================

    UPROPERTY()
    TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

    UPROPERTY()
    TWeakObjectPtr<UPrimitiveComponent> HeldComponent;

    /** Tick rate limiter for network updates */
    float NetworkUpdateAccumulator = 0.0f;
    
    static constexpr float NetworkUpdateRate = 0.05f; // 20Hz

    // ============================================================================
    // INTERNALS
    // ============================================================================

    bool Internal_TryGrab(AActor* TargetActor);
    void Internal_Release();
    void Internal_Throw(const FVector& ThrowDirection);

    FVector GetHoldLocation() const;
    bool TraceForGrabbable(FHitResult& OutHit) const;
    void UpdateHeldObjectLocation(float DeltaTime);
    
    bool ValidateGrabTarget(AActor* Target) const;
};