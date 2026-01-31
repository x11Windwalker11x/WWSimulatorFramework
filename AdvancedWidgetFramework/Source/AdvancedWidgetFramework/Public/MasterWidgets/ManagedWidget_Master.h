
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ManagedWidget_Master.generated.h"

/**
 * Base class for all managed widgets
 * Automatically registers/unregisters with WidgetManager on construct/destruct
 */
UCLASS(Abstract)
class ADVANCEDWIDGETFRAMEWORK_API UManagedWidget_Master : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Widget category tag for ESC priority (set in child class or Blueprint) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Managed Widget")
	FGameplayTag WidgetCategoryTag;
    
	/** Auto-register with WidgetManager? (default: true) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Managed Widget")
	bool bAutoRegister = true;
    
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
