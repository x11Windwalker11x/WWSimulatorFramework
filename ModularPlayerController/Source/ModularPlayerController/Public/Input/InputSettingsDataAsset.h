// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputSettingsDataAsset.generated.h"

/**
 * Data Asset that stores input mode preferences
 * Allows designers to configure toggle vs hold modes for various inputs
 */
UCLASS(BlueprintType)
class MODULARPLAYERCONTROLLER_API UInputSettingsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Input mode tags - determines if inputs use toggle or hold mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Modes", meta = (Categories = "Input.Mode"))
	FGameplayTagContainer InputModeTags;
    
	/** Display name for this input profile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FText ProfileName;
    
	/** Description of this input profile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info", meta = (MultiLine = true))
	FText Description;
};