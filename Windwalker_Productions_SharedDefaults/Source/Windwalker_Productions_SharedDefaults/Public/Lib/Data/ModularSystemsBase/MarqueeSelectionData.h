// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MarqueeSelectionData.generated.h"

/**
 * Configuration for marquee (box/lasso) selection behavior.
 * Used by UMarqueeSelectionWidget_Base and its consumers.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FMarqueeConfig
{
	GENERATED_BODY()

	/** Fill color of the selection rectangle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marquee Selection")
	FLinearColor FillColor = FLinearColor(0.2f, 0.5f, 1.0f, 0.15f);

	/** Border color of the selection rectangle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marquee Selection")
	FLinearColor BorderColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

	/** Border thickness in pixels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marquee Selection", meta = (ClampMin = "1.0"))
	float BorderThickness = 2.0f;

	/** Fill opacity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marquee Selection", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FillOpacity = 0.15f;

	/** Minimum drag distance before marquee activates (pixels) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marquee Selection", meta = (ClampMin = "0.0"))
	float MinDragThreshold = 5.0f;

	/** Minimum marquee size to register a valid selection (pixels) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marquee Selection", meta = (ClampMin = "0.0"))
	float MinMarqueeSize = 3.0f;

	/** Maximum objects that can be selected (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marquee Selection", meta = (ClampMin = "0"))
	int32 MaxSelectionCount = 0;

	/** Whether to detect modifier keys (Shift, Ctrl) for selection modes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marquee Selection")
	bool bEnableModifierKeys = true;
};

/**
 * A single candidate object that can be selected by marquee.
 * Uses TWeakObjectPtr<UObject> to avoid UMG dependency at L0.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FMarqueeCandidate
{
	GENERATED_BODY()

	/** The candidate object (widget, actor proxy, etc.) */
	UPROPERTY(BlueprintReadWrite, Category = "Marquee Selection")
	TWeakObjectPtr<UObject> CandidateObject;

	/** Screen-space position of the candidate's top-left corner */
	UPROPERTY(BlueprintReadWrite, Category = "Marquee Selection")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	/** Screen-space size of the candidate's bounding box */
	UPROPERTY(BlueprintReadWrite, Category = "Marquee Selection")
	FVector2D ScreenSize = FVector2D::ZeroVector;

	/** Priority for selection ordering (higher = selected first when capped) */
	UPROPERTY(BlueprintReadWrite, Category = "Marquee Selection")
	int32 SelectionPriority = 0;

	/** Optional filter tags for the candidate */
	UPROPERTY(BlueprintReadWrite, Category = "Marquee Selection")
	FGameplayTagContainer FilterTags;

	bool IsValid() const { return CandidateObject.IsValid(); }
};

/**
 * Result of a completed marquee selection operation.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FMarqueeSelectionResult
{
	GENERATED_BODY()

	/** Objects that were selected by the marquee */
	UPROPERTY(BlueprintReadOnly, Category = "Marquee Selection")
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;

	/** The modifier mode active during selection (UI.Selection.Mode.*) */
	UPROPERTY(BlueprintReadOnly, Category = "Marquee Selection")
	FGameplayTag ModifierMode;

	/** Top-left corner of the final marquee rectangle (local space) */
	UPROPERTY(BlueprintReadOnly, Category = "Marquee Selection")
	FVector2D MarqueeTopLeft = FVector2D::ZeroVector;

	/** Bottom-right corner of the final marquee rectangle (local space) */
	UPROPERTY(BlueprintReadOnly, Category = "Marquee Selection")
	FVector2D MarqueeBottomRight = FVector2D::ZeroVector;

	/** Total candidates evaluated during selection */
	UPROPERTY(BlueprintReadOnly, Category = "Marquee Selection")
	int32 TotalCandidatesTested = 0;

	bool IsValid() const { return SelectedObjects.Num() > 0; }
};
