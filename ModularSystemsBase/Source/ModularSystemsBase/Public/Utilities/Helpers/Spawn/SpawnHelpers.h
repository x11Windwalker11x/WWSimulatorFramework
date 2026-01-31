#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "Lib/Data/ModularSpawnSystem/SpawnData.h"
#include "SpawnHelpers.generated.h"

/**
 * Static utilities for spawn and drop operations.
 */

USTRUCT(BlueprintType)
struct FDropResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Drop")
    FName ItemID = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Drop")
    int32 Quantity = 0;

    FDropResult() {}
    
    FDropResult(FName InItemID, int32 InQuantity)
        : ItemID(InItemID), Quantity(InQuantity) {}

    bool IsValid() const { return !ItemID.IsNone() && Quantity > 0; }
};


UCLASS()
class MODULARSYSTEMSBASE_API USpawnHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // === DROP TABLE PROCESSING ===
    
public:
    // === DROP TABLE PROCESSING ===

    /** Process entire drop table, return all successful drops */
    UFUNCTION(BlueprintCallable, Category = "Spawn Helpers")
    static TArray<FDropResult> ProcessDropTable(const TArray<FDropTableEntry>& DropTable, AActor* Looter = nullptr);

    /** Roll single drop entry */
    UFUNCTION(BlueprintPure, Category = "Spawn Helpers")
    static bool RollDropEntry(const FDropTableEntry& Entry, FDropResult& OutResult);

    /** Check if looter meets drop requirements */
    UFUNCTION(BlueprintPure, Category = "Spawn Helpers")
    static bool MeetsDropRequirements(const FDropTableEntry& Entry, AActor* Looter);

    // === SCATTER LOCATIONS ===

    /** Generate scattered positions around origin */
    UFUNCTION(BlueprintPure, Category = "Spawn Helpers")
    static TArray<FVector> CalculateScatterLocations(FVector Origin, int32 Count, float MinRadius, float MaxRadius);

    /** Find valid spawn location using navmesh (returns false if none found) */
    UFUNCTION(BlueprintPure, Category = "Spawn Helpers", meta = (WorldContext = "WorldContext"))
    static bool FindValidSpawnLocation(UObject* WorldContext, FVector Origin, float SearchRadius, FVector& OutLocation);

    /** Snap transform to ground via line trace */
    UFUNCTION(BlueprintPure, Category = "Spawn Helpers", meta = (WorldContext = "WorldContext"))
    static FTransform SnapTransformToGround(UObject* WorldContext, FTransform InTransform, float TraceDistance = 1000.0f);

    // === SPAWN REQUEST BUILDING ===

    /** Build spawn request for item drop */
    UFUNCTION(BlueprintPure, Category = "Spawn Helpers")
    static FSpawnRequest BuildItemSpawnRequest(FName ItemID, FTransform SpawnTransform, TSoftClassPtr<AActor> ItemActorClass);

    /** Build spawn requests for all drop results */
    UFUNCTION(BlueprintCallable, Category = "Spawn Helpers")
    static TArray<FSpawnRequest> BuildSpawnRequestsFromDrops(const TArray<FDropResult>& Drops, FVector Origin, float ScatterRadius, TSoftClassPtr<AActor> ItemActorClass);

};