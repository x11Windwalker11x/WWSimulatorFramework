#include "Utilities/Helpers/Spawn/SpawnHelpers.h"
#include "GameplayTagAssetInterface.h"
#include "NavigationSystem.h"
#include "Engine/World.h"

// ===== DROP TABLE PROCESSING =====

TArray<FDropResult> USpawnHelpers::ProcessDropTable(const TArray<FDropTableEntry>& DropTable, AActor* Looter)
{
    TArray<FDropResult> Results;

    for (const FDropTableEntry& Entry : DropTable)
    {
        // Check requirements first
        if (!MeetsDropRequirements(Entry, Looter))
        {
            continue;
        }

        // Roll for drop
        FDropResult Result;
        if (RollDropEntry(Entry, Result))
        {
            Results.Add(Result);
        }
    }

    return Results;
}

bool USpawnHelpers::RollDropEntry(const FDropTableEntry& Entry, FDropResult& OutResult)
{
    // Roll chance
    if (!Entry.RollDrop())
    {
        return false;
    }

    // Roll quantity
    OutResult.ItemID = Entry.ItemID;
    OutResult.Quantity = Entry.RollQuantity();

    return OutResult.IsValid();
}

bool USpawnHelpers::MeetsDropRequirements(const FDropTableEntry& Entry, AActor* Looter)
{
    // No requirements = always passes
    if (Entry.RequiredLooterTags.IsEmpty())
    {
        return true;
    }

    // No looter but has requirements = fail
    if (!Looter)
    {
        return false;
    }

    // Check if looter has required tags via interface
    if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Looter))
    {
        FGameplayTagContainer LooterTags;
        TagInterface->GetOwnedGameplayTags(LooterTags);
        return LooterTags.HasAll(Entry.RequiredLooterTags);
    }

    return false;
}

// ===== SCATTER LOCATIONS =====

TArray<FVector> USpawnHelpers::CalculateScatterLocations(FVector Origin, int32 Count, float MinRadius, float MaxRadius)
{
    TArray<FVector> Locations;

    if (Count <= 0)
    {
        return Locations;
    }

    // Single item = at origin
    if (Count == 1)
    {
        Locations.Add(Origin);
        return Locations;
    }

    for (int32 i = 0; i < Count; i++)
    {
        // Random angle in radians
        float Angle = FMath::FRandRange(0.0f, 2.0f * PI);

        // Random distance between min and max
        float Distance = FMath::FRandRange(MinRadius, MaxRadius);

        // Calculate offset
        FVector Offset;
        Offset.X = FMath::Cos(Angle) * Distance;
        Offset.Y = FMath::Sin(Angle) * Distance;
        Offset.Z = 0.0f;

        Locations.Add(Origin + Offset);
    }

    return Locations;
}

bool USpawnHelpers::FindValidSpawnLocation(UObject* WorldContext, FVector Origin, float SearchRadius, FVector& OutLocation)
{
    if (!WorldContext)
    {
        return false;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!World)
    {
        return false;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
    if (!NavSys)
    {
        // No navmesh, just return origin
        OutLocation = Origin;
        return true;
    }

    FNavLocation NavLocation;
    bool bFound = NavSys->GetRandomReachablePointInRadius(Origin, SearchRadius, NavLocation);

    if (bFound)
    {
        OutLocation = NavLocation.Location;
        return true;
    }

    // Fallback to origin
    OutLocation = Origin;
    return false;
}

FTransform USpawnHelpers::SnapTransformToGround(UObject* WorldContext, FTransform InTransform, float TraceDistance)
{
    if (!WorldContext)
    {
        return InTransform;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
    if (!World)
    {
        return InTransform;
    }

    FVector Location = InTransform.GetLocation();
    FVector TraceStart = Location + FVector(0.0f, 0.0f, TraceDistance * 0.5f);
    FVector TraceEnd = Location - FVector(0.0f, 0.0f, TraceDistance * 0.5f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        FVector SnappedLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, 5.0f); // Small offset
        FTransform Result = InTransform;
        Result.SetLocation(SnappedLocation);
        return Result;
    }

    return InTransform;
}

// ===== SPAWN REQUEST BUILDING =====

FSpawnRequest USpawnHelpers::BuildItemSpawnRequest(FName ItemID, FTransform SpawnTransform, TSoftClassPtr<AActor> ItemActorClass)
{
    FSpawnRequest Request;
    Request.ActorClass = ItemActorClass;
    Request.SpawnTransform = SpawnTransform;
    Request.SpawnTags.AddTag(FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("Item.%s"), *ItemID.ToString()))));
    Request.bUsePooling = true;
    Request.PoolID = ItemID;
    return Request;
}

TArray<FSpawnRequest> USpawnHelpers::BuildSpawnRequestsFromDrops(const TArray<FDropResult>& Drops, FVector Origin, float ScatterRadius, TSoftClassPtr<AActor> ItemActorClass)
{
    TArray<FSpawnRequest> Requests;

    // Count total items for scatter
    int32 TotalItems = 0;
    for (const FDropResult& Drop : Drops)
    {
        TotalItems += Drop.Quantity;
    }

    // Generate scatter locations
    TArray<FVector> Locations = CalculateScatterLocations(Origin, TotalItems, 0.0f, ScatterRadius);

    // Build requests
    int32 LocationIndex = 0;
    for (const FDropResult& Drop : Drops)
    {
        for (int32 i = 0; i < Drop.Quantity; i++)
        {
            FVector SpawnLocation = Locations.IsValidIndex(LocationIndex) ? Locations[LocationIndex] : Origin;
            FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector);

            FSpawnRequest Request = BuildItemSpawnRequest(Drop.ItemID, SpawnTransform, ItemActorClass);
            Requests.Add(Request);

            LocationIndex++;
        }
    }

    return Requests;
}