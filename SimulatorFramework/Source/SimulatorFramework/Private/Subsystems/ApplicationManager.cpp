#include "Subsystems/ApplicationManager.h"
#include "UI/ApplicationBase.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"

void UApplicationManager::Initialize(UUserWidget* InDesktopWidget)
{
    DesktopWidget = InDesktopWidget;
    CurrentZOrder = 10;
}

void UApplicationManager::Shutdown()
{
    CloseAllApplications();
    RegisteredApplications.Empty();
    DesktopWidget = nullptr;
}

// ============================================================================
// REGISTRY
// ============================================================================

void UApplicationManager::RegisterApplication(TSubclassOf<UApplicationBase> ApplicationClass)
{
    if (!ApplicationClass) return;

    UApplicationBase* CDO = ApplicationClass->GetDefaultObject<UApplicationBase>();
    if (!CDO || CDO->ApplicationID.IsNone()) return;

    RegisteredApplications.Add(CDO->ApplicationID, ApplicationClass);
}

void UApplicationManager::UnregisterApplication(FName ApplicationID)
{
    CloseApplicationByID(ApplicationID);
    RegisteredApplications.Remove(ApplicationID);
}

bool UApplicationManager::IsApplicationRegistered(FName ApplicationID) const
{
    return RegisteredApplications.Contains(ApplicationID);
}

TArray<FName> UApplicationManager::GetRegisteredApplicationIDs() const
{
    TArray<FName> Result;
    RegisteredApplications.GetKeys(Result);
    return Result;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

UApplicationBase* UApplicationManager::LaunchApplication(FName ApplicationID)
{
    if (!DesktopWidget) return nullptr;

    TSubclassOf<UApplicationBase>* FoundClass = RegisteredApplications.Find(ApplicationID);
    if (!FoundClass || !*FoundClass) return nullptr;

    UApplicationBase* CDO = (*FoundClass)->GetDefaultObject<UApplicationBase>();

    // Check single instance
    if (!CDO->bAllowMultipleInstances && IsApplicationRunning(ApplicationID))
    {
        UApplicationBase* Existing = GetRunningApplicationByID(ApplicationID);
        if (Existing)
        {
            if (Existing->IsMinimized())
            {
                Existing->RestoreApplication();
            }
            else
            {
                Existing->FocusApplication();
            }
            return Existing;
        }
    }

    // Create widget
    UApplicationBase* NewApp = CreateWidget<UApplicationBase>(DesktopWidget, *FoundClass);
    if (!NewApp) return nullptr;

    NewApp->InitializeApplication(this);

    // Add to desktop
    if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(DesktopWidget->GetRootWidget()))
    {
        Canvas->AddChild(NewApp);
    }

    RunningApplications.Add(NewApp);
    NewApp->OpenApplication();

    OnApplicationOpened.Broadcast(NewApp);

    return NewApp;
}

void UApplicationManager::CloseApplication(UApplicationBase* Application)
{
    if (!Application) return;
    Application->CloseApplication();
}

void UApplicationManager::CloseApplicationByID(FName ApplicationID)
{
    for (int32 i = RunningApplications.Num() - 1; i >= 0; --i)
    {
        if (RunningApplications[i] && RunningApplications[i]->ApplicationID == ApplicationID)
        {
            RunningApplications[i]->CloseApplication();
        }
    }
}

void UApplicationManager::CloseAllApplications()
{
    for (int32 i = RunningApplications.Num() - 1; i >= 0; --i)
    {
        if (RunningApplications[i])
        {
            RunningApplications[i]->CloseApplication();
        }
    }
}

// ============================================================================
// WINDOW MANAGEMENT
// ============================================================================

void UApplicationManager::BringToFront(UApplicationBase* Application)
{
    if (!Application) return;

    // Update Z-order
    CurrentZOrder++;
    if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Application->Slot))
    {
        Slot->SetZOrder(CurrentZOrder);
    }

    SetFocusedApplication(Application);
}

TArray<UApplicationBase*> UApplicationManager::GetMinimizedApplications() const
{
    TArray<UApplicationBase*> Result;
    for (UApplicationBase* App : RunningApplications)
    {
        if (App && App->IsMinimized())
        {
            Result.Add(App);
        }
    }
    return Result;
}

UApplicationBase* UApplicationManager::GetRunningApplicationByID(FName ApplicationID) const
{
    for (UApplicationBase* App : RunningApplications)
    {
        if (App && App->ApplicationID == ApplicationID)
        {
            return App;
        }
    }
    return nullptr;
}

bool UApplicationManager::IsApplicationRunning(FName ApplicationID) const
{
    return GetRunningApplicationByID(ApplicationID) != nullptr;
}

// ============================================================================
// CALLBACKS
// ============================================================================

void UApplicationManager::OnApplicationClosed(UApplicationBase* Application)
{
    RunningApplications.Remove(Application);

    if (FocusedApplication == Application)
    {
        // Focus next available
        UApplicationBase* NextFocus = nullptr;
        for (UApplicationBase* App : RunningApplications)
        {
            if (App && App->IsOpen())
            {
                NextFocus = App;
                break;
            }
        }
        SetFocusedApplication(NextFocus);
    }

    if (RunningApplications.Num() == 0)
    {
        OnAllApplicationsClosed.Broadcast();
    }
}

void UApplicationManager::OnApplicationMinimized(UApplicationBase* Application)
{
    if (FocusedApplication == Application)
    {
        // Focus next available
        UApplicationBase* NextFocus = nullptr;
        for (UApplicationBase* App : RunningApplications)
        {
            if (App && App->IsOpen() && App != Application)
            {
                NextFocus = App;
                break;
            }
        }
        SetFocusedApplication(NextFocus);
    }
}

void UApplicationManager::SetFocusedApplication(UApplicationBase* Application)
{
    if (FocusedApplication == Application) return;

    FocusedApplication = Application;
    OnApplicationFocusChanged.Broadcast(Application);
}