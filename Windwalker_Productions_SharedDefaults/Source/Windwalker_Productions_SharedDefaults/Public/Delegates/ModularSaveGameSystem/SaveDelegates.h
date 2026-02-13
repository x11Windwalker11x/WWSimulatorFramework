// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveDelegates.generated.h"

// ================================================================
// SAVE SYSTEM DELEGATES (L0 contracts)
// ================================================================

// --- Existing Save/Load Flow Delegates ---

/** Broadcast when a save operation begins */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSaveStarted,
	FName, SlotName);

/** Broadcast when a save operation completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSaveCompleted,
	FName, SlotName,
	bool, bSucceeded);

// --- Master Save/Load Complete Delegates ---

/** Broadcast when the master save game finishes saving */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnMasterSaveComplete,
	bool, bSuccess,
	const FString&, SaveSlotName);

/** Broadcast when the master save game finishes loading */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnMasterLoadComplete,
	bool, bSuccess,
	const FString&, SaveSlotName);

// --- Registry Delegates (SaveableRegistrySubsystem) ---

/** Broadcast when a saveable object registers with the registry */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSaveableRegistered,
	const FString&, SaveID,
	int32, Priority);

/** Broadcast when a saveable object unregisters from the registry */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSaveableUnregistered,
	const FString&, SaveID);

// --- World State Delegates ---

/** Broadcast after all actor states for a level have been loaded and restored */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnWorldStateLoaded,
	const FString&, LevelName);

/** Broadcast per-actor after individual actor state restoration */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnActorStateRestored,
	const FString&, ActorSaveID,
	bool, bSuccess);
