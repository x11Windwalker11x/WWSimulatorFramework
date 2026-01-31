// MiniGameComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/SimulatorFramework/MiniGameData.h"
#include "MiniGameComponent.generated.h"

class UMiniGameHandlerBase;
class UObjectiveTrackerSubsystem;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiniGameStarted, const FGameplayTag&, MiniGameID, UMiniGameHandlerBase*, Handler);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMiniGameEnded, const FGameplayTag&, MiniGameID, bool, bSuccess, bool, bBonus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiniGameCancelled, const FGameplayTag&, MiniGameID, const FString&, Reason);

/**
 * Orchestrates mini-game lifecycle on player pawn
 * Loads config, spawns handler, routes input, manages camera
 */
UCLASS(ClassGroup = "Simulator", meta = (BlueprintSpawnableComponent))
class SIMULATORFRAMEWORK_API UMiniGameComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMiniGameComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === MINI-GAME CONTROL ===

    /** Start mini-game by ID (looks up in DataTable) */
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    bool StartMiniGame(const FGameplayTag& MiniGameID, AActor* StationActor = nullptr);

    /** Start mini-game with explicit config */
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    bool StartMiniGameWithConfig(const FMiniGameData& Config, AActor* StationActor = nullptr);

    /** End current mini-game */
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void EndMiniGame(bool bSuccess, bool bBonus = false);

    /** Cancel current mini-game */
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void CancelMiniGame(const FString& Reason = TEXT("Cancelled"));

    // === INPUT ROUTING ===

    /** Route axis input to active handler */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Input")
    void RouteAxisInput(const FVector2D& AxisValue, float DeltaTime);

    /** Route action input to active handler */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Input")
    void RouteActionInput(const FGameplayTag& ActionTag, bool bPressed);

    /** Route positional input to active handler */
    UFUNCTION(BlueprintCallable, Category = "MiniGame|Input")
    void RoutePositionalInput(const FVector& WorldPosition, const FVector& WorldNormal);

    // === QUERIES ===

    /** Check if mini-game is active */
    UFUNCTION(BlueprintPure, Category = "MiniGame")
    bool IsMiniGameActive() const { return bIsActive; }

    /** Get current mini-game ID */
    UFUNCTION(BlueprintPure, Category = "MiniGame")
    FGameplayTag GetCurrentMiniGameID() const { return CurrentMiniGameID; }

    /** Get current handler */
    UFUNCTION(BlueprintPure, Category = "MiniGame")
    UMiniGameHandlerBase* GetCurrentHandler() const { return ActiveHandler; }

    /** Get current station actor */
    UFUNCTION(BlueprintPure, Category = "MiniGame")
    AActor* GetCurrentStation() const { return CurrentStation.Get(); }

    /** Get progress of current mini-game */
    UFUNCTION(BlueprintPure, Category = "MiniGame")
    float GetCurrentProgress() const;

    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Events")
    FOnMiniGameStarted OnMiniGameStarted;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Events")
    FOnMiniGameEnded OnMiniGameEnded;

    UPROPERTY(BlueprintAssignable, Category = "MiniGame|Events")
    FOnMiniGameCancelled OnMiniGameCancelled;

    // === CONFIG ===

    /** DataTable containing mini-game definitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Config")
    TObjectPtr<UDataTable> MiniGameDataTable;

protected:
    // === STATE ===

    /*Active camera request ID*/
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|State")
    FGuid CameraRequestID;

    
    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|State")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|State")
    FGameplayTag CurrentMiniGameID;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|State")
    FMiniGameData CurrentConfig;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|State")
    TObjectPtr<UMiniGameHandlerBase> ActiveHandler;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|State")
    TWeakObjectPtr<AActor> CurrentStation;

    UPROPERTY(BlueprintReadOnly, Category = "MiniGame|State")
    FGuid CurrentObjectiveSetID;

    // === INTERNAL ===

    /** Lookup config from DataTable */
    bool GetMiniGameConfig(const FGameplayTag& MiniGameID, FMiniGameData& OutConfig) const;

    /** Spawn handler from config */
    UMiniGameHandlerBase* SpawnHandler(const FMiniGameData& Config);

    /** Get default handler class for type tag */
    TSubclassOf<UMiniGameHandlerBase> GetDefaultHandlerClass(const FGameplayTag& TypeTag) const;

    /** Register objectives with subsystem */
    FGuid RegisterObjectives(const FObjectiveSet& Objectives);

    /** Unregister objectives */
    void UnregisterObjectives();

    /** Get objective tracker */
    UObjectiveTrackerSubsystem* GetObjectiveTracker() const;

    /** Request camera mode via EventBus */
    void RequestCameraMode(const FGameplayTag& CameraModeTag);

    /** Release camera mode */
    void ReleaseCameraMode();

    /** Notify station of start/end */
    void NotifyStationStarted();
    void NotifyStationEnded(bool bSuccess, bool bBonus);

    /** Handler callbacks */
    UFUNCTION()
    void OnHandlerComplete(bool bSuccess);

    UFUNCTION()
    void OnHandlerFailed(const FString& Reason);
};