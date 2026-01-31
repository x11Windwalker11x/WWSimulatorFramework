#include "Subsystems/ModularLevelingSkillSystem/RegionSubsystem.h"
#include "Utilities/Helpers/Progression/ProgressionHelpers.h"

// ===== XP POOL MANAGEMENT =====

void URegionSubsystem::AddStationXP(FGameplayTag StationTag, int32 Amount)
{
    if (!StationTag.IsValid() || Amount <= 0) return;

    FStationXPPool& Pool = GetOrCreatePool(StationTag);
    Pool.AddXP(Amount);

    OnStationXPGained.Broadcast(StationTag, Amount, Pool.AvailableXP);
}

int32 URegionSubsystem::GetAvailableXP(FGameplayTag StationTag) const
{
    const FStationXPPool* Pool = XPPools.Find(StationTag);
    return Pool ? Pool->AvailableXP : 0;
}

int32 URegionSubsystem::GetTotalXPEarned(FGameplayTag StationTag) const
{
    const FStationXPPool* Pool = XPPools.Find(StationTag);
    return Pool ? Pool->TotalXPEarned : 0;
}

bool URegionSubsystem::SpendXP(FGameplayTag StationTag, int32 Amount)
{
    FStationXPPool* Pool = XPPools.Find(StationTag);
    if (!Pool) return false;

    return Pool->SpendXP(Amount);
}

FStationXPPool& URegionSubsystem::GetOrCreatePool(FGameplayTag StationTag)
{
    FStationXPPool* Pool = XPPools.Find(StationTag);
    if (Pool)
    {
        return *Pool;
    }

    FStationXPPool NewPool(StationTag);
    XPPools.Add(StationTag, NewPool);
    return XPPools[StationTag];
}

// ===== STATION MANAGEMENT =====

void URegionSubsystem::RegisterStation(AActor* Station, const FStationInstanceData& Data)
{
    if (!Station || !Data.IsValid()) return;

    RegisteredStations.Add(Station, Data);
}

void URegionSubsystem::UnregisterStation(AActor* Station)
{
    if (!Station) return;

    RegisteredStations.Remove(Station);
}

bool URegionSubsystem::GetStationData(AActor* Station, FStationInstanceData& OutData) const
{
    const FStationInstanceData* Data = RegisteredStations.Find(Station);
    if (Data)
    {
        OutData = *Data;
        return true;
    }
    return false;
}

bool URegionSubsystem::UpdateStationData(AActor* Station, const FStationInstanceData& NewData)
{
    if (!Station || !NewData.IsValid()) return false;

    FStationInstanceData* Data = RegisteredStations.Find(Station);
    if (Data)
    {
        *Data = NewData;
        return true;
    }
    return false;
}

TArray<AActor*> URegionSubsystem::GetStationsByTag(FGameplayTag StationTag) const
{
    TArray<AActor*> Result;

    for (const auto& Pair : RegisteredStations)
    {
        if (Pair.Value.StationTag.MatchesTag(StationTag))
        {
            Result.Add(Pair.Key);
        }
    }

    return Result;
}

TArray<AActor*> URegionSubsystem::GetAllStations() const
{
    TArray<AActor*> Result;
    RegisteredStations.GetKeys(Result);
    return Result;
}

// ===== LEVEL UP =====

int32 URegionSubsystem::GetLevelUpCost(AActor* Station) const
{
    FStationInstanceData Data;
    if (!GetStationData(Station, Data)) return 0;

    if (Data.Level >= 3) return 0;

    return UProgressionHelpers::GetLevelUpCost(Data.Level, Data.Tier);
}

bool URegionSubsystem::CanLevelUp(AActor* Station) const
{
    FStationInstanceData Data;
    if (!GetStationData(Station, Data)) return false;

    if (Data.Level >= 3) return false;

    int32 Cost = UProgressionHelpers::GetLevelUpCost(Data.Level, Data.Tier);
    int32 Available = GetAvailableXP(Data.StationTag);

    return Available >= Cost;
}

bool URegionSubsystem::TryLevelUpStation(AActor* Station)
{
    if (!CanLevelUp(Station)) return false;

    FStationInstanceData Data;
    if (!GetStationData(Station, Data)) return false;

    int32 Cost = UProgressionHelpers::GetLevelUpCost(Data.Level, Data.Tier);

    if (!SpendXP(Data.StationTag, Cost)) return false;

    int32 OldLevel = Data.Level;
    Data.Level++;
    UpdateStationData(Station, Data);

    OnStationLeveledUp.Broadcast(Station, OldLevel, Data.Level);

    return true;
}
