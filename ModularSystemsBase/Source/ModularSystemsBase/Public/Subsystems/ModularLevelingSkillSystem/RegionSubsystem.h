#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/ModularCraftingData/CraftingData.h"
#include "Delegates/CraftingDelegates/CraftingDelegates.h"
#include "RegionSubsystem.generated.h"



/*Manages how the xp pool for stations in bases in different regions accumulated.
 * TODO: tbd region rules
 * regions are only a TArray of UObjects only to hold something in memory to manage.
 * Only the stations like mixing, smelting, metalworks, or gun specific bench like
 * workbench are implemented.
 * Easily expandable. just implement the tags in data table (to be implemented)
 * and you are good to go. 
 */


UCLASS()
class MODULARSYSTEMSBASE_API URegionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // === XP POOL MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Region")
    void AddStationXP(FGameplayTag StationTag, int32 Amount);

    UFUNCTION(BlueprintPure, Category = "Region")
    int32 GetAvailableXP(FGameplayTag StationTag) const;

    UFUNCTION(BlueprintPure, Category = "Region")
    int32 GetTotalXPEarned(FGameplayTag StationTag) const;

    UFUNCTION(BlueprintCallable, Category = "Region")
    bool SpendXP(FGameplayTag StationTag, int32 Amount);

    // === STATION MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Region")
    void RegisterStation(AActor* Station, const FStationInstanceData& Data);

    UFUNCTION(BlueprintCallable, Category = "Region")
    void UnregisterStation(AActor* Station);

    UFUNCTION(BlueprintPure, Category = "Region")
    bool GetStationData(AActor* Station, FStationInstanceData& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "Region")
    bool UpdateStationData(AActor* Station, const FStationInstanceData& NewData);

    UFUNCTION(BlueprintPure, Category = "Region")
    TArray<AActor*> GetStationsByTag(FGameplayTag StationTag) const;

    UFUNCTION(BlueprintPure, Category = "Region")
    TArray<AActor*> GetAllStations() const;

    // === LEVEL UP ===

    UFUNCTION(BlueprintPure, Category = "Region")
    int32 GetLevelUpCost(AActor* Station) const;

    UFUNCTION(BlueprintPure, Category = "Region")
    bool CanLevelUp(AActor* Station) const;

    UFUNCTION(BlueprintCallable, Category = "Region")
    bool TryLevelUpStation(AActor* Station);

    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "Region")
    FOnStationXPGained OnStationXPGained;

    UPROPERTY(BlueprintAssignable, Category = "Region")
    FOnStationLeveledUp OnStationLeveledUp;

protected:
    UPROPERTY(SaveGame)
    TMap<FGameplayTag, FStationXPPool> XPPools;

    UPROPERTY()
    TMap<AActor*, FStationInstanceData> RegisteredStations;

private:
    FStationXPPool& GetOrCreatePool(FGameplayTag StationTag);
};