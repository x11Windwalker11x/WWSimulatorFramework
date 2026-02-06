// WidgetManagerBase.cpp
#include "Subsystems/AdvancedWidgetFramework/WidgetManagerBase.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void UWidgetManagerBase::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UWidgetManagerBase::Deinitialize()
{
    HideAllWidgets(true);
    Super::Deinitialize();
}

UWidgetManagerBase* UWidgetManagerBase::Get(APlayerController* PlayerController)
{
    if (!PlayerController) return nullptr;

    ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    if (!LocalPlayer) return nullptr;

    return LocalPlayer->GetSubsystem<UWidgetManagerBase>();
}

UUserWidget* UWidgetManagerBase::ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
    if (!WidgetClass) return nullptr;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return nullptr;

    // Check if already shown
    if (UUserWidget* Existing = GetActiveWidget(WidgetClass))
    {
        return Existing;
    }

    UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (Widget)
    {
        Widget->AddToViewport(ZOrder);
        ActiveWidgets.Add(WidgetClass, Widget);
        OnWidgetShown.Broadcast(Widget);
    }

    return Widget;
}

void UWidgetManagerBase::HideWidget(UUserWidget* Widget, bool bDestroy)
{
    if (!Widget) return;

    // Find and remove from tracking
    TSubclassOf<UUserWidget> FoundClass = nullptr;
    for (auto& Pair : ActiveWidgets)
    {
        if (Pair.Value == Widget)
        {
            FoundClass = Pair.Key;
            break;
        }
    }

    if (FoundClass)
    {
        ActiveWidgets.Remove(FoundClass);
    }

    Widget->RemoveFromParent();
    OnWidgetHidden.Broadcast(Widget);

    if (bDestroy)
    {
        Widget->ConditionalBeginDestroy();
    }
}

void UWidgetManagerBase::HideAllWidgets(bool bDestroy)
{
    TArray<UUserWidget*> ToHide;
    for (auto& Pair : ActiveWidgets)
    {
        if (Pair.Value)
        {
            ToHide.Add(Pair.Value);
        }
    }

    for (UUserWidget* Widget : ToHide)
    {
        HideWidget(Widget, bDestroy);
    }
}

bool UWidgetManagerBase::IsWidgetShown(TSubclassOf<UUserWidget> WidgetClass) const
{
    return ActiveWidgets.Contains(WidgetClass) && ActiveWidgets[WidgetClass] != nullptr;
}

UUserWidget* UWidgetManagerBase::GetActiveWidget(TSubclassOf<UUserWidget> WidgetClass) const
{
    const TObjectPtr<UUserWidget>* Found = ActiveWidgets.Find(WidgetClass);
    return Found ? *Found : nullptr;
}

APlayerController* UWidgetManagerBase::GetOwningPlayer() const
{
    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer) return nullptr;

    return LocalPlayer->GetPlayerController(GetWorld());
}

void UWidgetManagerBase::RegisterWidgetWithCategory(UUserWidget* Widget, FGameplayTag CategoryTag)
{
    if (!Widget || !CategoryTag.IsValid()) return;

    TArray<TObjectPtr<UUserWidget>>& Widgets = CategoryWidgets.FindOrAdd(CategoryTag);
    if (!Widgets.Contains(Widget))
    {
        Widgets.Add(Widget);
    }
}

void UWidgetManagerBase::UnregisterWidgetFromStack(UUserWidget* Widget)
{
    if (!Widget) return;

    // Remove from all categories
    for (auto& Pair : CategoryWidgets)
    {
        Pair.Value.Remove(Widget);
    }
}

TArray<UUserWidget*> UWidgetManagerBase::GetWidgetsByCategory(FGameplayTag CategoryTag) const
{
    TArray<UUserWidget*> Result;
    if (const TArray<TObjectPtr<UUserWidget>>* Found = CategoryWidgets.Find(CategoryTag))
    {
        for (const TObjectPtr<UUserWidget>& Widget : *Found)
        {
            if (Widget)
            {
                Result.Add(Widget);
            }
        }
    }
    return Result;
}

bool UWidgetManagerBase::IsWidgetRegistered(TSubclassOf<UUserWidget> WidgetClass) const
{
    return RegisteredWidgetClasses.Contains(WidgetClass);
}

void UWidgetManagerBase::RegisterWidget(TSubclassOf<UUserWidget> WidgetClass, int32 PoolSize, int32 ZOrder, bool bAutoShow, bool bAllowMultiple)
{
    if (!WidgetClass) return;

    RegisteredWidgetClasses.Add(WidgetClass);
    WidgetZOrders.Add(WidgetClass, ZOrder);

    if (bAutoShow)
    {
        ShowWidget(WidgetClass, ZOrder);
    }
}

void UWidgetManagerBase::HideWidget(TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass) return;

    if (UUserWidget* Widget = GetActiveWidget(WidgetClass))
    {
        HideWidget(Widget, false);
    }
}