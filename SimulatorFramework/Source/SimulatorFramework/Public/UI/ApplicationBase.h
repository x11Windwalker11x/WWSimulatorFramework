#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ApplicationBase.generated.h"

class UApplicationManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnApplicationStateChanged, FGameplayTag, OldState, FGameplayTag, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnApplicationClosed, UApplicationBase*, Application);

/**
 * Base class for device applications.
 * Manages lifecycle, state persistence, and manager communication.
 */
UCLASS(Abstract, Blueprintable)
class SIMULATORFRAMEWORK_API UApplicationBase : public UUserWidget
{
    GENERATED_BODY()

public:
    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Application|Config")
    FName ApplicationID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Application|Config")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Application|Config")
    TSoftObjectPtr<UTexture2D> ApplicationIcon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Application|Config")
    bool bCanMinimize = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Application|Config")
    bool bAllowMultipleInstances = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Application|Config")
    int32 DefaultZOrder = 10;

    // ============================================================================
    // LIFECYCLE API
    // ============================================================================

    UFUNCTION(BlueprintCallable, Category = "Application")
    void InitializeApplication(UApplicationManager* InManager);

    UFUNCTION(BlueprintCallable, Category = "Application")
    void OpenApplication();

    UFUNCTION(BlueprintCallable, Category = "Application")
    void CloseApplication();

    UFUNCTION(BlueprintCallable, Category = "Application")
    void MinimizeApplication();

    UFUNCTION(BlueprintCallable, Category = "Application")
    void RestoreApplication();

    UFUNCTION(BlueprintCallable, Category = "Application")
    void FocusApplication();

    // ============================================================================
    // QUERIES
    // ============================================================================

    UFUNCTION(BlueprintPure, Category = "Application")
    FORCEINLINE FGameplayTag GetApplicationState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Application")
    bool IsOpen() const;

    UFUNCTION(BlueprintPure, Category = "Application")
    bool IsMinimized() const;

    UFUNCTION(BlueprintPure, Category = "Application")
    FORCEINLINE UApplicationManager* GetManager() const { return OwningManager; }

    // ============================================================================
    // EVENTS
    // ============================================================================

    UPROPERTY(BlueprintAssignable, Category = "Application|Events")
    FOnApplicationStateChanged OnApplicationStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Application|Events")
    FOnApplicationClosed OnApplicationClosed;

protected:
    // ============================================================================
    // BLUEPRINT EVENTS
    // ============================================================================

    UFUNCTION(BlueprintImplementableEvent, Category = "Application")
    void OnOpened();

    UFUNCTION(BlueprintImplementableEvent, Category = "Application")
    void OnClosed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Application")
    void OnMinimized();

    UFUNCTION(BlueprintImplementableEvent, Category = "Application")
    void OnRestored();

    UFUNCTION(BlueprintImplementableEvent, Category = "Application")
    void OnFocused();

    UFUNCTION(BlueprintImplementableEvent, Category = "Application")
    void OnFocusLost();

    UFUNCTION(BlueprintNativeEvent, Category = "Application")
    TMap<FName, FString> SaveApplicationState();

    UFUNCTION(BlueprintNativeEvent, Category = "Application")
    void LoadApplicationState(const TMap<FName, FString>& StateData);

    // ============================================================================
    // STATE
    // ============================================================================

    UPROPERTY(BlueprintReadOnly, Category = "Application")
    FGameplayTag CurrentState;

    UPROPERTY()
    UApplicationManager* OwningManager = nullptr;

    UPROPERTY()
    TMap<FName, FString> CachedStateData;

    void SetApplicationState(FGameplayTag NewState);
};