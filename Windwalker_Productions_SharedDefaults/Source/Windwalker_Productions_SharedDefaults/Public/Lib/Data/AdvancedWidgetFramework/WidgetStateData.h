// WidgetStateData.h
// Data structs for Widget State Machine Manager
// Layer: L0 (SharedDefaults) - Pure data, no logic, no UMG dependency

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WidgetStateData.generated.h"

/**
 * How a widget handles being interrupted by a higher-priority widget.
 * Operator enum per convention (acceptable like ECompareOp).
 */
UENUM(BlueprintType)
enum class EWidgetInterruptMode : uint8
{
	/** Immediately close this widget when interrupted */
	Cancel		UMETA(DisplayName = "Cancel"),

	/** Queue this widget to reopen after the interrupter closes */
	Queue		UMETA(DisplayName = "Queue"),

	/** Pause this widget (stay in tree but hidden) and resume after interrupter closes */
	Pause		UMETA(DisplayName = "Pause")
};

/**
 * Configuration for a widget's state machine behavior.
 * Assigned per-widget class or per-instance.
 * Pure data struct - no logic (Rule #12).
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWidgetStateConfig
{
	GENERATED_BODY()

	/** Priority for conflict resolution (higher = more important, takes precedence) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget State")
	int32 Priority = 0;

	/** How this widget reacts when a higher-priority widget opens */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget State")
	EWidgetInterruptMode InterruptMode = EWidgetInterruptMode::Cancel;

	/** Duration of the show/animate-in transition (seconds, 0 = instant) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget State", meta = (ClampMin = "0.0"))
	float TransitionInDuration = 0.0f;

	/** Duration of the hide/animate-out transition (seconds, 0 = instant) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget State", meta = (ClampMin = "0.0"))
	float TransitionOutDuration = 0.0f;

	/** Category tag for grouping (conflicts only checked within same category) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget State")
	FGameplayTag CategoryTag;

	/** Auto-close timeout (seconds, 0 = no auto-close) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget State", meta = (ClampMin = "0.0"))
	float AutoCloseTimeout = 0.0f;

	/** Allow multiple widgets of this config in same category simultaneously */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget State")
	bool bAllowConcurrent = false;

	bool IsValid() const
	{
		return Priority >= 0;
	}
};
