#include "UI/ApplicationBase.h"
#include "Subsystems/ApplicationManager.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

void UApplicationBase::InitializeApplication(UApplicationManager* InManager)
{
    OwningManager = InManager;
    CurrentState = FWWTagLibrary::Simulator_Application_State_Closed();
}

void UApplicationBase::OpenApplication()
{
    if (CurrentState == FWWTagLibrary::Simulator_Application_State_Open()) return;

    FGameplayTag PrevState = CurrentState;
    SetApplicationState(FWWTagLibrary::Simulator_Application_State_Opening());

    // Restore state if was minimized
    if (PrevState == FWWTagLibrary::Simulator_Application_State_Minimized() && CachedStateData.Num() > 0)
    {
        LoadApplicationState(CachedStateData);
    }

    SetVisibility(ESlateVisibility::Visible);
    SetApplicationState(FWWTagLibrary::Simulator_Application_State_Open());

    OnOpened();
    FocusApplication();
}

void UApplicationBase::CloseApplication()
{
    if (CurrentState == FWWTagLibrary::Simulator_Application_State_Closed()) return;

    SetApplicationState(FWWTagLibrary::Simulator_Application_State_Closing());

    CachedStateData = SaveApplicationState();

    OnClosed();
    SetVisibility(ESlateVisibility::Collapsed);
    SetApplicationState(FWWTagLibrary::Simulator_Application_State_Closed());

    OnApplicationClosed.Broadcast(this);

    if (OwningManager)
    {
        OwningManager->OnApplicationClosed(this);
    }
}

void UApplicationBase::MinimizeApplication()
{
    if (!bCanMinimize) return;
    if (CurrentState != FWWTagLibrary::Simulator_Application_State_Open()) return;

    CachedStateData = SaveApplicationState();

    SetVisibility(ESlateVisibility::Collapsed);
    SetApplicationState(FWWTagLibrary::Simulator_Application_State_Minimized());

    OnMinimized();

    if (OwningManager)
    {
        OwningManager->OnApplicationMinimized(this);
    }
}

void UApplicationBase::RestoreApplication()
{
    if (CurrentState != FWWTagLibrary::Simulator_Application_State_Minimized()) return;

    SetVisibility(ESlateVisibility::Visible);

    if (CachedStateData.Num() > 0)
    {
        LoadApplicationState(CachedStateData);
    }

    SetApplicationState(FWWTagLibrary::Simulator_Application_State_Open());

    OnRestored();
    FocusApplication();
}

void UApplicationBase::FocusApplication()
{
    if (CurrentState != FWWTagLibrary::Simulator_Application_State_Open()) return;

    if (OwningManager)
    {
        OwningManager->BringToFront(this);
    }

    OnFocused();
}

bool UApplicationBase::IsOpen() const
{
    return CurrentState == FWWTagLibrary::Simulator_Application_State_Open();
}

bool UApplicationBase::IsMinimized() const
{
    return CurrentState == FWWTagLibrary::Simulator_Application_State_Minimized();
}

void UApplicationBase::SetApplicationState(FGameplayTag NewState)
{
    if (CurrentState == NewState) return;

    FGameplayTag OldState = CurrentState;
    CurrentState = NewState;

    OnApplicationStateChanged.Broadcast(OldState, NewState);
}

TMap<FName, FString> UApplicationBase::SaveApplicationState_Implementation()
{
    return TMap<FName, FString>();
}

void UApplicationBase::LoadApplicationState_Implementation(const TMap<FName, FString>& StateData)
{
}