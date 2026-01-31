#pragma once

#include "CoreMinimal.h"
#include "InteractionPredictionState.generated.h"

// ============================================================================
// IMPORTANT: FInteractableState MUST be declared BEFORE FPredictedInteraction
// because FPredictedInteraction uses it as a member variable
// ============================================================================

/** Stores actor state for rollback */
USTRUCT(BlueprintType)
struct FInteractableState
{
	GENERATED_BODY()

	/** Was actor visible? */
	UPROPERTY()
	bool bWasVisible = true;
    
	/** Actor's transform */
	UPROPERTY()
	FTransform CachedTransform;
    
	/** Was actor marked as interactable? */
	UPROPERTY()
	bool bWasInteractable = true;
    
	/** Custom state data (extend in child classes) */
	UPROPERTY()
	TMap<FName, FString> CustomData;
    
	FInteractableState()
		: bWasVisible(true)
		, CachedTransform(FTransform::Identity)
		, bWasInteractable(true)
	{
	}
};

/** Stores info about a predicted interaction */
USTRUCT(BlueprintType)
struct FPredictedInteraction
{
	GENERATED_BODY()

	/** The actor being interacted with */
	UPROPERTY()
	TWeakObjectPtr<AActor> Target;
    
	/** When prediction was made (for timeout) */
	UPROPERTY()
	float PredictionTime = 0.0f;
    
	/** Cached state before interaction (for rollback) */
	UPROPERTY()
	FInteractableState CachedState;
    
	/** Has server confirmed this? */
	UPROPERTY()
	bool bConfirmed = false;
    
	FPredictedInteraction()
		: Target(nullptr)
		, PredictionTime(0.0f)
		, bConfirmed(false)
	{
	}
};