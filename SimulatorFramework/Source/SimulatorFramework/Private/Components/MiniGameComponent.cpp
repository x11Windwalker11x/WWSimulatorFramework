// MiniGameComponent.cpp
#include "Components/MiniGameComponent.h"
#include "Subsystems/MiniGameHandlerBase.h"
#include "Subsystems/ModularQuestSystem/ObjectiveTrackerSubsystem.h"
#include "Interfaces/SimulatorFramework/MiniGameStationInterface.h"
#include "Lib/Data/ModularQuestSystem/CameraEvents.h"
#include "Interfaces/ModularPlayerController//CameraControlInterface.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"

UMiniGameComponent::UMiniGameComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMiniGameComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UMiniGameComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bIsActive)
    {
        CancelMiniGame(TEXT("Component destroyed"));
    }
    Super::EndPlay(EndPlayReason);
}

void UMiniGameComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (ActiveHandler && ActiveHandler->IsActive() && ActiveHandler->NeedsTick())
    {
        ActiveHandler->TickHandler(DeltaTime);
    }
}

// === MINI-GAME CONTROL ===

bool UMiniGameComponent::StartMiniGame(const FGameplayTag& MiniGameID, AActor* StationActor)
{
    FMiniGameData Config;
    
    // Try station override first
    if (StationActor && StationActor->GetClass()->ImplementsInterface(UMiniGameStationInterface::StaticClass()))
    {
        if (IMiniGameStationInterface::Execute_GetMiniGameConfig(StationActor, Config))
        {
            return StartMiniGameWithConfig(Config, StationActor);
        }
    }

    // Fallback to DataTable
    if (!GetMiniGameConfig(MiniGameID, Config))
    {
        UE_LOG(LogTemp, Warning, TEXT("MiniGameComponent: Config not found for %s"), *MiniGameID.ToString());
        return false;
    }

    return StartMiniGameWithConfig(Config, StationActor);
}

bool UMiniGameComponent::StartMiniGameWithConfig(const FMiniGameData& Config, AActor* StationActor)
{
    if (bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("MiniGameComponent: Already active, cannot start %s"), *Config.MiniGameID.ToString());
        return false;
    }

    if (!Config.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MiniGameComponent: Invalid config"));
        return false;
    }

    // Check station availability
    if (StationActor && StationActor->GetClass()->ImplementsInterface(UMiniGameStationInterface::StaticClass()))
    {
        if (!IMiniGameStationInterface::Execute_IsMiniGameAvailable(StationActor))
        {
            UE_LOG(LogTemp, Warning, TEXT("MiniGameComponent: Station not available"));
            return false;
        }
    }

    // Store state
    CurrentConfig = Config;
    CurrentMiniGameID = Config.MiniGameID;
    CurrentStation = StationActor;

    // Register objectives
    CurrentObjectiveSetID = RegisterObjectives(Config.Objectives);

    // Spawn handler
    ActiveHandler = SpawnHandler(Config);
    if (!ActiveHandler)
    {
        UE_LOG(LogTemp, Error, TEXT("MiniGameComponent: Failed to spawn handler for %s"), *Config.MiniGameID.ToString());
        UnregisterObjectives();
        return false;
    }

    // Initialize and activate
    ActiveHandler->Initialize(this, Config, CurrentObjectiveSetID);
    ActiveHandler->OnComplete.AddDynamic(this, &UMiniGameComponent::OnHandlerComplete);
    ActiveHandler->OnFailed.AddDynamic(this, &UMiniGameComponent::OnHandlerFailed);
    ActiveHandler->Activate();

    // Request camera
    if (Config.CameraModeTag.IsValid())
    {
        RequestCameraMode(Config.CameraModeTag);
    }

    // Enable tick if handler needs it
    SetComponentTickEnabled(ActiveHandler->NeedsTick());

    bIsActive = true;

    NotifyStationStarted();
    OnMiniGameStarted.Broadcast(CurrentMiniGameID, ActiveHandler);

    return true;
}

void UMiniGameComponent::EndMiniGame(bool bSuccess, bool bBonus)
{
    if (!bIsActive) return;

    FGameplayTag EndedID = CurrentMiniGameID;

    // Cleanup handler
    if (ActiveHandler)
    {
        ActiveHandler->OnComplete.RemoveDynamic(this, &UMiniGameComponent::OnHandlerComplete);
        ActiveHandler->OnFailed.RemoveDynamic(this, &UMiniGameComponent::OnHandlerFailed);
        ActiveHandler->Deactivate();
        ActiveHandler = nullptr;
    }

    // Release camera
    ReleaseCameraMode();

    // Unregister objectives
    UnregisterObjectives();

    // Notify station
    NotifyStationEnded(bSuccess, bBonus);

    // Reset state
    bIsActive = false;
    CurrentMiniGameID = FGameplayTag();
    CurrentStation.Reset();
    SetComponentTickEnabled(false);

    OnMiniGameEnded.Broadcast(EndedID, bSuccess, bBonus);
}

void UMiniGameComponent::CancelMiniGame(const FString& Reason)
{
    if (!bIsActive) return;

    if (!CurrentConfig.bCanCancel && Reason != TEXT("Component destroyed"))
    {
        UE_LOG(LogTemp, Warning, TEXT("MiniGameComponent: Cannot cancel %s"), *CurrentMiniGameID.ToString());
        return;
    }

    FGameplayTag CancelledID = CurrentMiniGameID;

    // Cleanup (same as EndMiniGame but different event)
    if (ActiveHandler)
    {
        ActiveHandler->OnComplete.RemoveDynamic(this, &UMiniGameComponent::OnHandlerComplete);
        ActiveHandler->OnFailed.RemoveDynamic(this, &UMiniGameComponent::OnHandlerFailed);
        ActiveHandler->Deactivate();
        ActiveHandler = nullptr;
    }

    ReleaseCameraMode();
    UnregisterObjectives();
    NotifyStationEnded(false, false);

    bIsActive = false;
    CurrentMiniGameID = FGameplayTag();
    CurrentStation.Reset();
    SetComponentTickEnabled(false);

    OnMiniGameCancelled.Broadcast(CancelledID, Reason);
}

// === INPUT ROUTING ===

void UMiniGameComponent::RouteAxisInput(const FVector2D& AxisValue, float DeltaTime)
{
    if (ActiveHandler && ActiveHandler->IsActive())
    {
        ActiveHandler->ProcessAxisInput(AxisValue, DeltaTime);
    }
}

void UMiniGameComponent::RouteActionInput(const FGameplayTag& ActionTag, bool bPressed)
{
    if (ActiveHandler && ActiveHandler->IsActive())
    {
        ActiveHandler->ProcessActionInput(ActionTag, bPressed);
    }
}

void UMiniGameComponent::RoutePositionalInput(const FVector& WorldPosition, const FVector& WorldNormal)
{
    if (ActiveHandler && ActiveHandler->IsActive())
    {
        ActiveHandler->ProcessPositionalInput(WorldPosition, WorldNormal);
    }
}

// === QUERIES ===

float UMiniGameComponent::GetCurrentProgress() const
{
    return ActiveHandler ? ActiveHandler->GetProgress() : 0.0f;
}

// === INTERNAL ===

bool UMiniGameComponent::GetMiniGameConfig(const FGameplayTag& MiniGameID, FMiniGameData& OutConfig) const
{
    if (!MiniGameDataTable) return false;

    const FString ContextString(TEXT("MiniGameComponent"));
    FMiniGameData* Found = MiniGameDataTable->FindRow<FMiniGameData>(FName(*MiniGameID.ToString()), ContextString);
    
    if (Found)
    {
        OutConfig = *Found;
        return true;
    }
    return false;
}

UMiniGameHandlerBase* UMiniGameComponent::SpawnHandler(const FMiniGameData& Config)
{
    TSubclassOf<UMiniGameHandlerBase> HandlerClass = Config.HandlerClass;

    // Fallback to default for type
    if (!HandlerClass)
    {
        HandlerClass = GetDefaultHandlerClass(Config.TypeTag);
    }

    if (!HandlerClass)
    {
        UE_LOG(LogTemp, Error, TEXT("MiniGameComponent: No handler class for type %s"), *Config.TypeTag.ToString());
        return nullptr;
    }

    return NewObject<UMiniGameHandlerBase>(this, HandlerClass);
}

TSubclassOf<UMiniGameHandlerBase> UMiniGameComponent::GetDefaultHandlerClass(const FGameplayTag& TypeTag) const
{
    // TODO: Map type tags to default handler classes
    // For now, return nullptr - handlers must be specified in config
    return nullptr;
}

FGuid UMiniGameComponent::RegisterObjectives(const FObjectiveSet& Objectives)
{
    if (UObjectiveTrackerSubsystem* Tracker = GetObjectiveTracker())
    {
        return Tracker->RegisterObjectiveSet(Objectives);
    }
    return FGuid();
}

void UMiniGameComponent::UnregisterObjectives()
{
    if (CurrentObjectiveSetID.IsValid())
    {
        if (UObjectiveTrackerSubsystem* Tracker = GetObjectiveTracker())
        {
            Tracker->UnregisterObjectiveSet(CurrentObjectiveSetID);
        }
        CurrentObjectiveSetID.Invalidate();
    }
}

UObjectiveTrackerSubsystem* UMiniGameComponent::GetObjectiveTracker() const
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            return GI->GetSubsystem<UObjectiveTrackerSubsystem>();
        }
    }
    return nullptr;
}

void UMiniGameComponent::RequestCameraMode(const FGameplayTag& CameraModeTag)
{
    if (!CameraModeTag.IsValid()) return;

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;

    // Find camera controller (typically on pawn or controller)
    ICameraControlInterface* CameraController = nullptr;

    // Check owner first
    if (OwnerActor->GetClass()->ImplementsInterface(UCameraControlInterface::StaticClass()))
    {
        CameraController = Cast<ICameraControlInterface>(OwnerActor);
    }
    // Check controller if owner is pawn
    else if (APawn* Pawn = Cast<APawn>(OwnerActor))
    {
        if (AController* Controller = Pawn->GetController())
        {
            if (Controller->GetClass()->ImplementsInterface(UCameraControlInterface::StaticClass()))
            {
                CameraController = Cast<ICameraControlInterface>(Controller);
            }
        }
    }

    if (!CameraController) return;

    // Build request
    FCameraModeRequest Request = FCameraModeRequest::Make(
        CameraModeTag,
        CurrentStation.Get(),
        100 // MiniGame priority
    );
    Request.SourceTag = CurrentMiniGameID;

    // Store ID for release
    CameraRequestID = Request.RequesterID;

    // Execute request
    ICameraControlInterface::Execute_RequestCameraMode(Cast<UObject>(CameraController), Request);

}

void UMiniGameComponent::ReleaseCameraMode()
{
    if (!CameraRequestID.IsValid()) return;

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;

    // Find camera controller (same logic as request)
    ICameraControlInterface* CameraController = nullptr;

    if (OwnerActor->GetClass()->ImplementsInterface(UCameraControlInterface::StaticClass()))
    {
        CameraController = Cast<ICameraControlInterface>(OwnerActor);
    }
    else if (APawn* Pawn = Cast<APawn>(OwnerActor))
    {
        if (AController* Controller = Pawn->GetController())
        {
            if (Controller->GetClass()->ImplementsInterface(UCameraControlInterface::StaticClass()))
            {
                CameraController = Cast<ICameraControlInterface>(Controller);
            }
        }
    }

    if (!CameraController) return;

    // Build release
    FCameraModeRelease Release;
    Release.RequesterID = CameraRequestID;
    Release.BlendTime = 0.3f;

    // Execute release
    ICameraControlInterface::Execute_ReleaseCameraMode(Cast<UObject>(CameraController), Release);

    CameraRequestID.Invalidate();

}

void UMiniGameComponent::NotifyStationStarted()
{
    if (AActor* Station = CurrentStation.Get())
    {
        if (Station->GetClass()->ImplementsInterface(UMiniGameStationInterface::StaticClass()))
        {
            IMiniGameStationInterface::Execute_OnMiniGameStarted(Station, CurrentMiniGameID);
        }
    }
}

void UMiniGameComponent::NotifyStationEnded(bool bSuccess, bool bBonus)
{
    if (AActor* Station = CurrentStation.Get())
    {
        if (Station->GetClass()->ImplementsInterface(UMiniGameStationInterface::StaticClass()))
        {
            IMiniGameStationInterface::Execute_OnMiniGameEnded(Station, CurrentMiniGameID, bSuccess, bBonus);
        }
    }
}

void UMiniGameComponent::OnHandlerComplete(bool bSuccess)
{
    if (UObjectiveTrackerSubsystem* Tracker = GetObjectiveTracker())
    {
        bool bBonus = Tracker->HasBonusReward(CurrentObjectiveSetID);
        EndMiniGame(bSuccess, bBonus);
    }
    else
    {
        EndMiniGame(bSuccess, false);
    }
}

void UMiniGameComponent::OnHandlerFailed(const FString& Reason)
{
    EndMiniGame(false, false);
}