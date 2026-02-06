#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Interfaces/AdvancedWidgetFramework/ManagedWidgetInterface.h"

#include "Widget_InteractionPrompt.generated.h"

UCLASS()
class ADVANCEDWIDGETFRAMEWORK_API UWidget_InteractionPrompt : public UUserWidget, public IManagedWidgetInterface
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bIsValidWidget = false;
	// Bind this to your text block in the widget blueprint
	UPROPERTY(meta=(BindWidget))
	UTextBlock* PromptText;
    
	// Function to update the prompt
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void SetPromptText(const FText& NewText);
    
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void SetInteractionKey(const FText& KeyText);
protected:
	virtual void NativeConstruct() override;

	// IManagedWidgetInterface
	virtual UObject* GetManagedWidgetAsObject_Implementation() override;
	virtual FGameplayTag GetWidgetCategoryTag_Implementation() const override;
	virtual bool IsValidWidget_Implementation() const override;
};