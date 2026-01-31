// WidgetManagerBase.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "WidgetManagerBase.generated.h"

class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetShown, UUserWidget*, Widget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetHidden, UUserWidget*, Widget);

/**
 * Base Widget Manager - generic widget lifecycle
 * Extend in plugins for specific functionality
 */
UCLASS()
class MODULARSYSTEMSBASE_API UWidgetManagerBase : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // === STATIC ACCESS ===

    /** Get WidgetManagerBase from PlayerController */
    UFUNCTION(BlueprintPure, Category = "Widget Manager", meta = (DefaultToSelf = "PlayerController"))
    static UWidgetManagerBase* Get(APlayerController* PlayerController);

    // === WIDGET LIFECYCLE ===

    /** Create and show a widget */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    UUserWidget* ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 0);

    /** Hide and optionally destroy a widget */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    void HideWidget(UUserWidget* Widget, bool bDestroy = false);

    /** Hide all active widgets */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    void HideAllWidgets(bool bDestroy = false);

    /** Check if a widget of class is currently shown */
    UFUNCTION(BlueprintPure, Category = "Widget Manager")
    bool IsWidgetShown(TSubclassOf<UUserWidget> WidgetClass) const;

    /** Get active widget of class (nullptr if not shown) */
    UFUNCTION(BlueprintPure, Category = "Widget Manager")
    UUserWidget* GetActiveWidget(TSubclassOf<UUserWidget> WidgetClass) const;

    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "Widget Manager|Events")
    FOnWidgetShown OnWidgetShown;

    UPROPERTY(BlueprintAssignable, Category = "Widget Manager|Events")
    FOnWidgetHidden OnWidgetHidden;

protected:
    /** Get owning player controller */
    APlayerController* GetOwningPlayer() const;

    /** Active widgets tracked by class */
    UPROPERTY()
    TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> ActiveWidgets;
};