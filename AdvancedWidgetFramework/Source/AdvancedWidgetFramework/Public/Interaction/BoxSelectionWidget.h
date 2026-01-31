// BoxSelectionWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "BoxSelectionWidget.generated.h"

/**
 * Visual box for rectangle selection
 * Shows a semi-transparent cyan rectangle during drag-to-select
 */
UCLASS()
class ADVANCEDWIDGETFRAMEWORK_API UBoxSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Border that shows the selection rectangle */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UBorder* Border_SelectionBox;

protected:
	virtual void NativeConstruct() override;
};