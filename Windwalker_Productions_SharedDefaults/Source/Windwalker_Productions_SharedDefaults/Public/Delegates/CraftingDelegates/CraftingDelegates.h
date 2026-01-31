#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CraftingDelegates.generated.h"

// ===== STATION DELEGATES =====



/** Fired when station gains XP */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnStationXPGained,
    FGameplayTag, StationTag,
    int32, AmountGained,
    int32, NewTotal
);

/** Fired when station levels up */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnStationLeveledUp,
    AActor*, Station,
    int32, OldLevel,
    int32, NewLevel
);

/** Fired when station durability changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnStationDurabilityChanged,
    AActor*, Station,
    float, OldDurability,
    float, NewDurability
);

/** Fired when station breaks (durability <= 0) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnStationBroken,
    AActor*, Station
);

/** Fired when station is repaired */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnStationRepaired,
    AActor*, Station,
    float, NewDurability
);

// ===== TOOL DELEGATES =====

/** Fired when tool gains XP */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnToolXPGained,
    FName, ItemID,
    int32, AmountGained,
    int32, NewTotal
);

/** Fired when tool levels up */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnToolLeveledUp,
    FName, ItemID,
    int32, OldLevel,
    int32, NewLevel
);

/** Fired when tool durability changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnToolDurabilityChanged,
    FName, ItemID,
    float, OldDurability,
    float, NewDurability
);

/** Fired when tool breaks */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnToolBroken,
    FName, ItemID
);

/** Fired when tool is repaired */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnToolRepaired,
    FName, ItemID,
    float, NewDurability
);

// ===== CRAFTING DELEGATES =====

/** Fired when crafting starts */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnCraftingStarted,
    FName, RecipeID,
    AActor*, Station
);

/** Fired when crafting completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnCraftingCompleted,
    FName, RecipeID,
    FName, OutputItemID,
    int32, Quantity
);

/** Fired when crafting fails */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnCraftingFailed,
    FName, RecipeID,
    FString, Reason
);

/** Fired when crafting progress updates (for timed crafting) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnCraftingProgress,
    FName, RecipeID,
    float, Progress,
    float, TimeRemaining
);