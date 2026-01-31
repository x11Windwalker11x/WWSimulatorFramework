// SpatialHashGrid.h
// Grid-based spatial hashing for high-performance actor queries
// Optimized for Fortnite-level dense object scenarios (200+ items in tight spaces)

#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Math/IntPoint.h"

// Forward declaration to avoid circular dependency
class UInteractionSubsystem;

/**
 * Fast 2D spatial hash grid for mostly-flat game worlds
 * Better than octrees for Fortnite-style games with limited vertical gameplay
 * 
 * Performance: O(1) insertion, O(1) queries for nearby actors
 * Memory: ~24 bytes per grid cell + actor references
 */
class MODULARINTERACTIONSYSTEM_API FSpatialHashGrid
{
public:
    FSpatialHashGrid(float InCellSize = 1000.f);
    ~FSpatialHashGrid();

    // Core Operations
    void RegisterActor(AActor* Actor);
    void UnregisterActor(AActor* Actor);
    void UpdateActorPosition(AActor* Actor, const FVector& OldLocation);
    void Clear();

    // Query Operations
    TArray<AActor*> GetNearbyActors(const FVector& Location, int32 CellRadius = 1) const;
    TArray<AActor*> GetActorsInRadius(const FVector& Center, float Radius) const;
    TArray<AActor*> GetActorsInCell(const FVector& Location) const;

    // Utilities
    int32 GetTotalActorCount() const;
    int32 GetCellCount() const;
    float GetCellSize() const { return CellSize; }

    // Debug
    void DebugDrawGrid(UWorld* World, const FVector& Center, int32 Radius = 5, float Duration = 1.f) const;
    void PrintGridStats() const;
    
    // Console command registration (called by owning subsystem)
    static void RegisterConsoleCommands(UInteractionSubsystem* Subsystem);
    static void UnregisterConsoleCommands();

private:
    // Grid cell size in world units (Fortnite uses ~1000.0 = 10 meters)
    float CellSize;

    // Main storage: Grid coordinate -> Array of actors in that cell
    TMap<FIntPoint, TArray<TWeakObjectPtr<AActor>>> Grid;

    // Reverse lookup: Actor -> Grid coordinate (for fast updates/removal)
    TMap<TWeakObjectPtr<AActor>, FIntPoint> ActorToCell;

    // Conversion helpers
    FORCEINLINE FIntPoint WorldToGrid(const FVector& WorldLocation) const;
    FORCEINLINE FVector GridToWorld(const FIntPoint& GridCoord) const;
    
    // Internal helpers
    void AddActorToCell(AActor* Actor, const FIntPoint& Cell);
    void RemoveActorFromCell(AActor* Actor, const FIntPoint& Cell);
};

// Inline implementations for performance
FORCEINLINE FIntPoint FSpatialHashGrid::WorldToGrid(const FVector& WorldLocation) const
{
    return FIntPoint(
        FMath::FloorToInt(WorldLocation.X / CellSize),
        FMath::FloorToInt(WorldLocation.Y / CellSize)
    );
}

FORCEINLINE FVector FSpatialHashGrid::GridToWorld(const FIntPoint& GridCoord) const
{
    return FVector(
        GridCoord.X * CellSize + CellSize * 0.5f,
        GridCoord.Y * CellSize + CellSize * 0.5f,
        0.f
    );
}