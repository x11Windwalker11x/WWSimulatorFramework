// InteractionData.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/Core/WorldObjectData.h"
#include "InteractionData.generated.h"

/**
 * Data for interactable objects.
 * Extends FWorldObjectData with interaction-specific properties.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FInteractableData : public FWorldObjectData
{
    GENERATED_BODY()

    // === INTERACTION CONFIG ===
    
    /** Tags defining valid interaction types (e.g., Interaction.Type.Pickup, Interaction.Type.Use) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FGameplayTagContainer InteractionTypes;

    /** Tags required on interactor to interact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FGameplayTagContainer RequiredTags;

    /** Tags that block interaction if present on interactor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FGameplayTagContainer BlockedTags;

    // === STATE ===
    
    /** Current state tag (e.g., State.Interactable.Locked, State.Interactable.Broken) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    FGameplayTag CurrentState;

    /** Valid state transitions from current state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    TMap<FGameplayTag, FGameplayTag> StateTransitions;

    // === INTERACTION SETTINGS ===
    
    /** Hold duration for hold-type interactions (0 = instant) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0"))
    float HoldDuration = 0.0f;

    /** Max distance for interaction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0"))
    float InteractionRange = 200.0f;

    /** Can multiple players interact simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAllowMultipleInteractors = false;

    // === UI ===
    
    /** Prompt text override (empty = use DisplayName) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FText InteractionPrompt;

    FText GetPrompt() const
    {
        return InteractionPrompt.IsEmpty() ? DisplayName : InteractionPrompt;
    }
};

/**
 * Runtime state for an interactable instance.
 * Separate from data to allow same FInteractableData with different states.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FInteractableState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    FGameplayTag CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsBeingInteracted = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TWeakObjectPtr<AActor> CurrentInteractor;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float InteractionProgress = 0.0f;
};