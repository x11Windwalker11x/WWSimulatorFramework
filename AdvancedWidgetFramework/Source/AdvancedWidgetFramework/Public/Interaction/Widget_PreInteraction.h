#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Widget_PreInteraction.generated.h"

UCLASS()
class ADVANCEDWIDGETFRAMEWORK_API UWidget_PreInteraction : public UUserWidget
{
	GENERATED_BODY()

public:
	// Bind to the Image widget in Blueprint
	UPROPERTY(meta=(BindWidget))
	UImage* Image_PreInteraction;

	// Change dot color
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void SetDotColor(FLinearColor NewColor);

	// Change dot size
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void SetDotSize(float Size);
};