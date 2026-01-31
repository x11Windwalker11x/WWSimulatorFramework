// ContextMenuWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "ContextMenuWidget_Master.generated.h"

class UButton;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuItemClicked, const FString&, ItemName);

/**
 * Base Context Menu Widget
 * Provides core menu functionality - inherit from this for specific menu types
 */
UCLASS(Abstract)
class MODULARINVENTORYSYSTEM_API UContextMenuWidget_Master : public UManagedWidget_Master
{
    GENERATED_BODY()

public:
    // ============================================================================
    // PUBLIC API
    // ============================================================================
    UContextMenuWidget_Master(const FObjectInitializer& ObjectInitializer);

    /** Add a menu item to the menu */
    UFUNCTION(BlueprintCallable, Category = "Context Menu")
    void AddMenuItem(const FString& ItemName, const FString& ItemLabel);

    /** Clear all menu items */
    UFUNCTION(BlueprintCallable, Category = "Context Menu")
    void ClearMenuItems();

    /** Event fired when a menu item is clicked */
    UPROPERTY(BlueprintAssignable, Category = "Context Menu")
    FOnMenuItemClicked OnMenuItemClicked;

    /** Close the menu */
    UFUNCTION(BlueprintCallable, Category = "Context Menu")
    void CloseMenu();

protected:
    // ============================================================================
    // WIDGET LIFECYCLE
    // ============================================================================
    
    virtual void NativeConstruct() override;
    
    // ============================================================================
    // INPUT HANDLING
    // ============================================================================
    
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    // ============================================================================
    // INTERNAL
    // ============================================================================
    
    /** Called when a menu button is clicked */
    UFUNCTION()
    void OnMenuItemButtonClicked();  // No parameter - uses ButtonToItemNameMap

    /** The vertical box that holds menu items (must be named exactly "MenuItemContainer" in UMG) */
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* MenuItemContainer;

    /** Track if we should check for outside clicks (delayed one frame to prevent immediate closure) */
    bool bCheckOutsideClicks;
    
    /** Map buttons to their item names */
    UPROPERTY()
    TMap<TObjectPtr<UButton>, FString> ButtonToItemNameMap;
};