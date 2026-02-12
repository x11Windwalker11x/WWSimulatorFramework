#include "UI/Widget_InteractionPrompt.h"

#include "Logging/InteractableInventoryLogging.h"

void UWidget_InteractionPrompt::NativeConstruct()
{
    Super::NativeConstruct();
    bIsValidWidget = true;

    // Force visibility to Visible when created
    SetVisibility(ESlateVisibility::Visible);

    UE_LOG(LogDebugSystem, Log, TEXT("Widget_InteractionPrompt NativeConstruct called"));
}


void UWidget_InteractionPrompt::SetPromptText(const FText& NewText)
{
    if (PromptText)
    {
        PromptText->SetText(NewText);
    }
}

void UWidget_InteractionPrompt::SetInteractionKey(const FText& KeyText)
{
    if (PromptText)
    {
        FText FullText = FText::Format(
            INVTEXT("Press [{0}] {1}"),
            KeyText,
            INVTEXT("to interact")
        );
        PromptText->SetText(FullText);
    }
}

// IManagedWidgetInterface

UObject* UWidget_InteractionPrompt::GetManagedWidgetAsObject_Implementation()
{
    return this;
}

FGameplayTag UWidget_InteractionPrompt::GetWidgetCategoryTag_Implementation() const
{
    return FGameplayTag();
}

bool UWidget_InteractionPrompt::IsValidWidget_Implementation() const
{
    return bIsValidWidget;
}
