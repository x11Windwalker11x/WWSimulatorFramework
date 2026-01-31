// CameraEvents.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CameraEvents.generated.h"

/**
 * Camera mode request data
 * Sent via EventBus to request camera transitions
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FCameraModeRequest
{
    GENERATED_BODY()

    /** Requested camera mode tag (Camera.Mode.*) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag CameraModeTag;

    /** Priority (higher overrides lower) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 0;

    /** Blend time to this mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendTime = 0.3f;

    /** Optional target actor to focus on */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> FocusTarget;

    /** Requester ID (for release matching) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid RequesterID;

    /** Source tag identifying who requested (Simulator.MiniGame.*, Interaction.*, etc) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag SourceTag;

    bool IsValid() const { return CameraModeTag.IsValid() && RequesterID.IsValid(); }

    static FCameraModeRequest Make(const FGameplayTag& Mode, AActor* Target = nullptr, int32 InPriority = 0)
    {
        FCameraModeRequest Request;
        Request.CameraModeTag = Mode;
        Request.FocusTarget = Target;
        Request.Priority = InPriority;
        Request.RequesterID = FGuid::NewGuid();
        return Request;
    }
};

/**
 * Camera mode release data
 */
USTRUCT(BlueprintType)
struct SHAREDDEFAULTS_API FCameraModeRelease
{
    GENERATED_BODY()

    /** Requester ID that originally requested the mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid RequesterID;

    /** Blend time back to previous mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendTime = 0.3f;

    bool IsValid() const { return RequesterID.IsValid(); }
};

/** Delegates for camera events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraModeRequested, const FCameraModeRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraModeReleased, const FCameraModeRelease&, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCameraModeChanged, const FGameplayTag&, NewMode, const FGameplayTag&, PreviousMode);