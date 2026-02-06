// WidgetManagerBase.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "Delegates/AdvancedWidgetFramework/WW_WidgetDelegates.h"
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

    // === WIDGET REGISTRATION ===

    /**
     * Check if a widget class is registered for management
     * @param WidgetClass - The widget class to check
     * @return True if registered
     */
    UFUNCTION(BlueprintPure, Category = "Widget Manager")
    bool IsWidgetRegistered(TSubclassOf<UUserWidget> WidgetClass) const;

    /**
     * Register a widget class for management
     * @param WidgetClass - The widget class to register
     * @param PoolSize - Number of instances to pre-create (0 = lazy creation)
     * @param ZOrder - Z-order for display
     * @param bAutoShow - Automatically show on registration
     * @param bAllowMultiple - Allow multiple instances
     */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    virtual void RegisterWidget(TSubclassOf<UUserWidget> WidgetClass, int32 PoolSize = 0, int32 ZOrder = 0, bool bAutoShow = false, bool bAllowMultiple = false);

    /**
     * Hide widget by class (convenience overload)
     * @param WidgetClass - The widget class to hide
     */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    void HideWidget(TSubclassOf<UUserWidget> WidgetClass);

    // === WIDGET CATEGORY REGISTRATION ===

    /**
     * Register a widget with a category tag for stack management
     * @param Widget - The widget to register
     * @param CategoryTag - The category this widget belongs to
     */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    virtual void RegisterWidgetWithCategory(UUserWidget* Widget, FGameplayTag CategoryTag);

    /**
     * Unregister a widget from the category stack
     * @param Widget - The widget to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "Widget Manager")
    virtual void UnregisterWidgetFromStack(UUserWidget* Widget);

    /**
     * Get widgets by category tag
     * @param CategoryTag - The category to filter by
     * @return Array of widgets in this category
     */
    UFUNCTION(BlueprintPure, Category = "Widget Manager")
    TArray<UUserWidget*> GetWidgetsByCategory(FGameplayTag CategoryTag) const;

    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "Widget Manager|Events")
    FOnWidgetShown OnWidgetShown;

    UPROPERTY(BlueprintAssignable, Category = "Widget Manager|Events")
    FOnWidgetHidden OnWidgetHidden;

    /**
     * Transition intercept delegate (single-cast).
     * If bound, the interceptor (e.g. UWidgetStateManager) handles show/hide transitions.
     * If not bound, default immediate show/hide (graceful degradation when AWF deleted).
     * Return true = intercepted, false = proceed with default behavior.
     */
    FWidgetTransitionInterceptDelegate OnWidgetTransitionRequested;

    /** Fired after any widget state change (for external listeners) */
    UPROPERTY(BlueprintAssignable, Category = "Widget Manager|Events")
    FOnWidgetStateChanged OnWidgetStateChanged;

    /**
     * Sync intercept delegate (single-cast).
     * If bound, UWidgetSyncSubsystem captures state changes for replication.
     * Fires alongside OnWidgetStateChanged for sync capture.
     */
    FWidgetSyncInterceptDelegate OnWidgetSyncRequested;

    /**
     * Dock intercept delegate (single-cast).
     * If bound, UDockLayoutManager handles widget dock zone placement.
     * Return true = intercepted, false = no docking behavior.
     */
    FWidgetDockInterceptDelegate OnWidgetDockRequested;

    /** Fired when any dock/undock/rearrange occurs */
    UPROPERTY(BlueprintAssignable, Category = "Widget Manager|Events")
    FOnDockLayoutChanged OnDockLayoutChanged;

protected:
    /** Get owning player controller */
    APlayerController* GetOwningPlayer() const;

    /** Active widgets tracked by class */
    UPROPERTY()
    TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> ActiveWidgets;

    /** Widgets tracked by category tag */
    UPROPERTY()
    TMap<FGameplayTag, TArray<TObjectPtr<UUserWidget>>> CategoryWidgets;

    /** Registered widget classes with their settings */
    UPROPERTY()
    TSet<TSubclassOf<UUserWidget>> RegisteredWidgetClasses;

    /** Default Z-order per registered class */
    TMap<TSubclassOf<UUserWidget>, int32> WidgetZOrders;
};