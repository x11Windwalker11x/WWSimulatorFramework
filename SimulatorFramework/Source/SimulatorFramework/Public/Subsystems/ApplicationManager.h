#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ApplicationManager.generated.h"

class UApplicationBase;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnApplicationOpened, UApplicationBase*, Application);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnApplicationFocusChanged, UApplicationBase*, FocusedApplication);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllApplicationsClosed);

/**
 * Manages device applications - registry, lifecycle, window order.
 */
UCLASS(BlueprintType, Blueprintable)
class SIMULATORFRAMEWORK_API UApplicationManager : public UObject
{
    GENERATED_BODY()

public:
    // ============================================================================
    // INITIALIZATION
    // ============================================================================

    UFUNCTION(BlueprintCallable, Category = "Application Manager")
    void Initialize(UUserWidget* InDesktopWidget);

    UFUNCTION(BlueprintCallable, Category = "Application Manager")
    void Shutdown();

    // ============================================================================
    // REGISTRY
    // ============================================================================

    UFUNCTION(BlueprintCallable, Category = "Application Manager")
    void RegisterApplication(TSubclassOf<UApplicationBase> ApplicationClass);

    UFUNCTION(BlueprintCallable, Category = "Application Manager")
    void UnregisterApplication(FName ApplicationID);

    UFUNCTION(BlueprintPure, Category = "Application Manager")
    bool IsApplicationRegistered(FName ApplicationID) const;

    UFUNCTION(BlueprintPure, Category = "Application Manager")
    TArray<FName> GetRegisteredApplicationIDs() const;

    // ============================================================================
    // LIFECYCLE
    // ============================================================================

    UFUNCTION(BlueprintCallable, Category = "Application Manager")
    UApplicationBase* LaunchApplication(FName ApplicationID);

    UFUNCTION(BlueprintCallable, Category = "Application Manager")
    void CloseApplication(UApplicationBase* Application);

    UFUNCTION(BlueprintCallable, Category = "Application Manager")
    void CloseApplicationByID(FName ApplicationID);

    UFUNCTION(BlueprintCallable, Category = "Application Manager")
    void CloseAllApplications();

    // ============================================================================
    // WINDOW MANAGEMENT
    // ============================================================================

    UFUNCTION(BlueprintCallable, Category = "Application Manager")
    void BringToFront(UApplicationBase* Application);

    UFUNCTION(BlueprintPure, Category = "Application Manager")
    FORCEINLINE UApplicationBase* GetFocusedApplication() const { return FocusedApplication; }

    UFUNCTION(BlueprintPure, Category = "Application Manager")
    TArray<UApplicationBase*> GetRunningApplications() const { return RunningApplications; }

    UFUNCTION(BlueprintPure, Category = "Application Manager")
    TArray<UApplicationBase*> GetMinimizedApplications() const;

    UFUNCTION(BlueprintPure, Category = "Application Manager")
    UApplicationBase* GetRunningApplicationByID(FName ApplicationID) const;

    UFUNCTION(BlueprintPure, Category = "Application Manager")
    bool IsApplicationRunning(FName ApplicationID) const;

    // ============================================================================
    // CALLBACKS
    // ============================================================================

    void OnApplicationClosed(UApplicationBase* Application);
    void OnApplicationMinimized(UApplicationBase* Application);

    // ============================================================================
    // EVENTS
    // ============================================================================

    UPROPERTY(BlueprintAssignable, Category = "Application Manager|Events")
    FOnApplicationOpened OnApplicationOpened;

    UPROPERTY(BlueprintAssignable, Category = "Application Manager|Events")
    FOnApplicationFocusChanged OnApplicationFocusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Application Manager|Events")
    FOnAllApplicationsClosed OnAllApplicationsClosed;

protected:
    UPROPERTY()
    TMap<FName, TSubclassOf<UApplicationBase>> RegisteredApplications;

    UPROPERTY()
    TArray<UApplicationBase*> RunningApplications;

    UPROPERTY()
    UApplicationBase* FocusedApplication = nullptr;

    UPROPERTY()
    UUserWidget* DesktopWidget = nullptr;

    int32 CurrentZOrder = 10;

    void SetFocusedApplication(UApplicationBase* Application);
};

//TODO: we need to make it use WidgetManager somehow...
/*I've got to update the WidetManager Module first*/