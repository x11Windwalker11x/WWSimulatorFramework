// CommonEnums.h
#pragma once

#include "CoreMinimal.h"
#include "WW_Enums.generated.h"

/**
 * Play mode types for game systems
 * Used by InteractionSubsystem, SaveSystem, UIManager, etc.
 */
UENUM(BlueprintType)
enum class EPlayMode : uint8
{
	Unknown          UMETA(DisplayName = "Unknown"),
	SinglePlayer     UMETA(DisplayName = "Single Player"),      // 1 local player, no network
	SplitScreen      UMETA(DisplayName = "Split Screen"),       // 2-4 local players, same machine
	ListenServer     UMETA(DisplayName = "Listen Server"),      // Host + remote clients
	DedicatedServer  UMETA(DisplayName = "Dedicated Server"),   // Pure server, no local players
	Client           UMETA(DisplayName = "Client")              // Remote client
};

/**
 * Network role types (simplified)
 */
UENUM(BlueprintType)
enum class ENetworkRole : uint8
{
	None             UMETA(DisplayName = "None"),
	Authority        UMETA(DisplayName = "Authority"),          // Server
	AutonomousProxy  UMETA(DisplayName = "Autonomous Proxy"),   // Local player
	SimulatedProxy   UMETA(DisplayName = "Simulated Proxy")     // Remote player/AI
};

/**
 * Input source for mini-game axis controls
 */
UENUM(BlueprintType)
enum class EInputSource : uint8
{
    Mouse    UMETA(DisplayName = "Mouse"),
    Keys     UMETA(DisplayName = "Keys"),
    Both     UMETA(DisplayName = "Both")
};

/**
 * Reference frame for axis manipulation
 */
UENUM(BlueprintType)
enum class EAxisReference : uint8
{
    View     UMETA(DisplayName = "View"),      // Relative to camera
    Actor    UMETA(DisplayName = "Actor"),     // Relative to grabbed actor
    World    UMETA(DisplayName = "World")      // World space
};

/**
 * Comparison operators for objective conditions
 */
UENUM(BlueprintType)
enum class ECompareOp : uint8
{
    Equal           UMETA(DisplayName = "=="),
    NotEqual        UMETA(DisplayName = "!="),
    Greater         UMETA(DisplayName = ">"),
    GreaterEqual    UMETA(DisplayName = ">="),
    Less            UMETA(DisplayName = "<"),
    LessEqual       UMETA(DisplayName = "<="),
    InRange         UMETA(DisplayName = "In Range")
};

/**
 * Feedback type for sweetspot/proximity detection
 */
UENUM(BlueprintType)
enum class EFeedbackType : uint8
{
    None        UMETA(DisplayName = "None"),
    Vibration   UMETA(DisplayName = "Vibration"),
    Visual      UMETA(DisplayName = "Visual"),
    Audio       UMETA(DisplayName = "Audio"),
    Combined    UMETA(DisplayName = "Combined")
};

/**
 * Lockpicking mechanic style
 */
UENUM(BlueprintType)
enum class ELockpickStyle : uint8
{
    Skyrim      UMETA(DisplayName = "Skyrim"),      // Rotate pick, find sweetspot
    Oblivion    UMETA(DisplayName = "Oblivion"),    // Tumbler timing
    ESO         UMETA(DisplayName = "ESO"),         // Hold + release on tremble
    Dial        UMETA(DisplayName = "Dial"),        // Safe dial rotation
    Tumbler     UMETA(DisplayName = "Tumbler")      // Pin by pin
};