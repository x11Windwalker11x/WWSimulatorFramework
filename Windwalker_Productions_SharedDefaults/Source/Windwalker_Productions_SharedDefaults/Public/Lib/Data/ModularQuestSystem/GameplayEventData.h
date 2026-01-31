// GameplayEventData.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEventData.generated.h"

/**
 * Universal payload for all gameplay events
 * And Quest System.
 * Single struct accepted by EventBusSubsystem.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FGameplayEventPayload
{
    GENERATED_BODY()

    // === EVENT IDENTITY ===
    
    /** Event type tag (e.g., Event.Quest.ObjectiveComplete, Event.XP.Gained) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FGameplayTag EventTag;

    // === CONTEXT ===
    
    /** Actor that triggered the event */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TObjectPtr<AActor> Instigator;

    /** Target actor (if applicable) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TObjectPtr<AActor> Target;

    /** Additional context tags */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FGameplayTagContainer ContextTags;

    // === PAYLOAD DATA ===
    
    /** Generic ID (item ID, quest ID, skill ID, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    FName PrimaryID = NAME_None;

    /** Secondary ID if needed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    FName SecondaryID = NAME_None;

    /** Numeric value (XP amount, damage, quantity, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    float Value = 0.0f;

    /** Integer value (count, level, index, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    int32 IntValue = 0;

    /** String data (message, name, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    FString StringData;

    /** World location if relevant */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    FVector Location = FVector::ZeroVector;

    // === METADATA ===
    
    /** Timestamp when event was created */
    UPROPERTY(BlueprintReadOnly, Category = "Metadata")
    float Timestamp = 0.0f;

    /** Should this event be saved/replicated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    bool bPersistent = false;

    // === HELPERS ===
    
    bool IsValid() const { return EventTag.IsValid(); }

    static FGameplayEventPayload Make(FGameplayTag InEventTag, AActor* InInstigator = nullptr)
    {
        FGameplayEventPayload Payload;
        Payload.EventTag = InEventTag;
        Payload.Instigator = InInstigator;
        Payload.Timestamp = InInstigator ? InInstigator->GetWorld()->GetTimeSeconds() : 0.0f;
        return Payload;
    }
};

/**
 * Trigger definition for automatic event firing.
 * Placed on actors/components to fire events on conditions.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FGameplayEventTrigger
{
    GENERATED_BODY()

    /** Event to broadcast when triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FGameplayTag EventTag;

    /** Condition tag that must match for trigger to fire */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FGameplayTag ConditionTag;

    /** Payload template (values overridden at runtime) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FGameplayEventPayload PayloadTemplate;

    /** Fire only once */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bOneShot = false;

    /** Has this trigger fired (runtime) */
    UPROPERTY(BlueprintReadOnly, Category = "Trigger")
    bool bHasFired = false;

    bool CanFire() const { return !bOneShot || !bHasFired; }
};

