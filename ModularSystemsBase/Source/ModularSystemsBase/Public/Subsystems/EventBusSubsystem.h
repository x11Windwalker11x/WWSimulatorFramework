#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Delegates/ModularQuestSystem/GameplayEventDelegates.h"
#include "Lib/Data/ModularQuestSystem/GameplayEventData.h"
#include "EventBusSubsystem.generated.h"

/**
 * Central event bus for cross-plugin communication.
 * Single API for broadcasting gameplay events.
 */


UCLASS()
class MODULARSYSTEMSBASE_API UEventBusSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // === LIFECYCLE ===
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Get EventBus from any world context */
    UFUNCTION(BlueprintPure, Category = "Event Bus", meta = (WorldContext = "WorldContextObject"))
    static UEventBusSubsystem* Get(const UObject* WorldContextObject);

    // === BROADCASTING ===
    
    /**
     * Broadcast a gameplay event to all listeners.
     * Single API entry point for all cross-plugin events.
     */
    UFUNCTION(BlueprintCallable, Category = "Event Bus")
    void BroadcastEvent(const FGameplayEventPayload& Payload);

    /**
     * Broadcast with quick construction.
     * @param EventTag - The event type
     * @param Instigator - Actor that triggered event
     * @param Value - Optional numeric value
     */
    UFUNCTION(BlueprintCallable, Category = "Event Bus")
    void BroadcastSimpleEvent(FGameplayTag EventTag, AActor* Instigator = nullptr, float Value = 0.0f);

    // === DELEGATES ===
    
    /** Universal gameplay event delegate - bind to receive all events */
    UPROPERTY(BlueprintAssignable, Category = "Event Bus")
    FOnGameplayEvent OnGameplayEvent;

    /**
     * Get delegate for specific event tag filtering.
     * Listeners can filter in their handler, but this is for convenience.
     */
    UFUNCTION(BlueprintPure, Category = "Event Bus")
    bool DoesEventMatchTag(const FGameplayEventPayload& Payload, FGameplayTag FilterTag) const;

protected:
    /** Log event for debugging */
    void LogEvent(const FGameplayEventPayload& Payload) const;
};