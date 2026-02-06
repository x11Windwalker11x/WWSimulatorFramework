// ManagedWidgetInterface.h - L0 interface for cross-plugin managed widget access
// Merged from IValidWidgetInterface (Feb 2, 2026) - Implementation completed Feb 6, 2026

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "ManagedWidgetInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UManagedWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class IManagedWidgetInterface
{
	GENERATED_BODY()

public:
	/** Mandatory getter - Rule #29: type-safe access without cross-plugin casting */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Widget")
	UObject* GetManagedWidgetAsObject();

	/** Widget category tag for ESC priority and category management */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Widget")
	FGameplayTag GetWidgetCategoryTag() const;

	/** Is this widget currently valid for display? (Replaces old IValidWidgetInterface marker) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Widget")
	bool IsValidWidget() const;
};
