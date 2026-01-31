// MiniGameStationInterface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "MiniGameStationInterface.generated.h"

struct FMiniGameData;
struct FSnapPointData;

UINTERFACE(MinimalAPI, Blueprintable)
class UMiniGameStationInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that host mini-games (vaults, locks, assembly stations)
 * Station owns game-specific logic; MiniGameComponent owns orchestration
 */
class SHAREDDEFAULTS_API IMiniGameStationInterface
{
    GENERATED_BODY()

public:
    /** Get mini-game ID this station uses */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MiniGame")
    FGameplayTag GetMiniGameID() const;

    /** Get full mini-game config (station may override DataTable values) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MiniGame")
    bool GetMiniGameConfig(FMiniGameData& OutConfig) const;

    /** Called when mini-game starts */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MiniGame")
    void OnMiniGameStarted(const FGameplayTag& MiniGameID);

    /** Called when mini-game ends */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MiniGame")
    void OnMiniGameEnded(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus);

    /** Get snap points for assembly mini-games */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MiniGame|Assembly")
    TArray<FSnapPointData> GetSnapPoints() const;

    /** Get stored code for sequence mini-games (vault combo, etc.) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MiniGame|Sequence")
    TArray<FGameplayTag> GetStoredCode() const;

    /** Get current difficulty modifier (0-1, affects tolerances) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MiniGame")
    float GetDifficultyModifier() const;

    /** Check if station is available for mini-game interaction */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MiniGame")
    bool IsMiniGameAvailable() const;
};