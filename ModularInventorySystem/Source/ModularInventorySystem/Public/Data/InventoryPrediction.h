#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "GameplayTagContainer.h"

/** Cached state for rollback on prediction failure */
USTRUCT()
struct FInventoryPrediction
{
    GENERATED_BODY()

    /** What operation was predicted */
    UPROPERTY()
    FName OperationType;  // "MoveItem", "SplitStack", "ToggleFavorite"

    /** Slots involved - stored for rollback */
    UPROPERTY()
    FGameplayTag SourceInventoryType;
    
    UPROPERTY()
    int32 SourceSlotIndex = INDEX_NONE;
    
    UPROPERTY()
    FInventorySlot SourceSlotCache;

    UPROPERTY()
    FGameplayTag DestInventoryType;
    
    UPROPERTY()
    int32 DestSlotIndex = INDEX_NONE;
    
    UPROPERTY()
    FInventorySlot DestSlotCache;

    /** Timestamp for timeout cleanup */
    UPROPERTY()
    double Timestamp = 0.0;

    FInventoryPrediction() = default;
};

    