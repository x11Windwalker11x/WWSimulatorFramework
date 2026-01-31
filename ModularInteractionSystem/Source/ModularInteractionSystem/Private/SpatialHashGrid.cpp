// SpatialHashGrid.cpp
// Implementation of high-performance 2D spatial hash grid

#include "SpatialHashGrid.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "InteractionSubsystem.h"

FSpatialHashGrid::FSpatialHashGrid(float InCellSize)
    : CellSize(InCellSize)
{
    // Reserve some space to avoid initial allocations
    Grid.Reserve(256);
    ActorToCell.Reserve(512);
}

FSpatialHashGrid::~FSpatialHashGrid()
{
    Clear();
}

void FSpatialHashGrid::RegisterActor(AActor* Actor)
{
    if (!IsValid(Actor))
        return;

    // Check if already registered
    TWeakObjectPtr<AActor> WeakActor(Actor);
    if (ActorToCell.Contains(WeakActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor %s already registered in spatial grid"), *Actor->GetName());
        return;
    }

    // Calculate grid cell
    FIntPoint Cell = WorldToGrid(Actor->GetActorLocation());
    
    // Add to grid and reverse lookup
    AddActorToCell(Actor, Cell);
}

void FSpatialHashGrid::UnregisterActor(AActor* Actor)
{
    if (!IsValid(Actor))
        return;

    TWeakObjectPtr<AActor> WeakActor(Actor);
    
    // Find which cell this actor is in
    if (FIntPoint* Cell = ActorToCell.Find(WeakActor))
    {
        RemoveActorFromCell(Actor, *Cell);
    }
}

void FSpatialHashGrid::UpdateActorPosition(AActor* Actor, const FVector& OldLocation)
{
    if (!IsValid(Actor))
        return;

    FIntPoint OldCell = WorldToGrid(OldLocation);
    FIntPoint NewCell = WorldToGrid(Actor->GetActorLocation());

    // Only update if actor moved to a different cell
    if (OldCell != NewCell)
    {
        TWeakObjectPtr<AActor> WeakActor(Actor);
        
        // Remove from old cell
        if (TArray<TWeakObjectPtr<AActor>>* CellActors = Grid.Find(OldCell))
        {
            CellActors->Remove(WeakActor);
            
            // Clean up empty cells
            if (CellActors->Num() == 0)
            {
                Grid.Remove(OldCell);
            }
        }

        // Add to new cell
        AddActorToCell(Actor, NewCell);
    }
}

void FSpatialHashGrid::Clear()
{
    Grid.Empty();
    ActorToCell.Empty();
}

TArray<AActor*> FSpatialHashGrid::GetNearbyActors(const FVector& Location, int32 CellRadius) const
{
    TArray<AActor*> Result;
    Result.Reserve(64); // Pre-allocate reasonable amount

    FIntPoint CenterCell = WorldToGrid(Location);

    // Check surrounding cells in a square pattern
    // For CellRadius=1: checks 3x3 = 9 cells
    // For CellRadius=2: checks 5x5 = 25 cells
    for (int32 X = -CellRadius; X <= CellRadius; X++)
    {
        for (int32 Y = -CellRadius; Y <= CellRadius; Y++)
        {
            FIntPoint CheckCell = CenterCell + FIntPoint(X, Y);
            
            if (const TArray<TWeakObjectPtr<AActor>>* CellActors = Grid.Find(CheckCell))
            {
                for (const TWeakObjectPtr<AActor>& WeakActor : *CellActors)
                {
                    if (WeakActor.IsValid())
                    {
                        Result.Add(WeakActor.Get());
                    }
                }
            }
        }
    }

    return Result;
}

TArray<AActor*> FSpatialHashGrid::GetActorsInRadius(const FVector& Center, float Radius) const
{
    TArray<AActor*> Result;
    Result.Reserve(64);

    // Calculate how many cells we need to check
    int32 CellRadius = FMath::CeilToInt(Radius / CellSize);
    float RadiusSq = Radius * Radius;

    FIntPoint CenterCell = WorldToGrid(Center);

    // Check cells in range
    for (int32 X = -CellRadius; X <= CellRadius; X++)
    {
        for (int32 Y = -CellRadius; Y <= CellRadius; Y++)
        {
            FIntPoint CheckCell = CenterCell + FIntPoint(X, Y);
            
            if (const TArray<TWeakObjectPtr<AActor>>* CellActors = Grid.Find(CheckCell))
            {
                for (const TWeakObjectPtr<AActor>& WeakActor : *CellActors)
                {
                    if (AActor* Actor = WeakActor.Get())
                    {
                        // Distance check for precision
                        float DistSq = FVector::DistSquared(Center, Actor->GetActorLocation());
                        if (DistSq <= RadiusSq)
                        {
                            Result.Add(Actor);
                        }
                    }
                }
            }
        }
    }

    return Result;
}

TArray<AActor*> FSpatialHashGrid::GetActorsInCell(const FVector& Location) const
{
    TArray<AActor*> Result;
    
    FIntPoint Cell = WorldToGrid(Location);
    
    if (const TArray<TWeakObjectPtr<AActor>>* CellActors = Grid.Find(Cell))
    {
        Result.Reserve(CellActors->Num());
        
        for (const TWeakObjectPtr<AActor>& WeakActor : *CellActors)
        {
            if (WeakActor.IsValid())
            {
                Result.Add(WeakActor.Get());
            }
        }
    }

    return Result;
}

int32 FSpatialHashGrid::GetTotalActorCount() const
{
    int32 Count = 0;
    for (const auto& Pair : Grid)
    {
        Count += Pair.Value.Num();
    }
    return Count;
}

int32 FSpatialHashGrid::GetCellCount() const
{
    return Grid.Num();
}

void FSpatialHashGrid::DebugDrawGrid(UWorld* World, const FVector& Center, int32 Radius, float Duration) const
{
    if (!World)
        return;

    FIntPoint CenterCell = WorldToGrid(Center);

    for (int32 X = -Radius; X <= Radius; X++)
    {
        for (int32 Y = -Radius; Y <= Radius; Y++)
        {
            FIntPoint Cell = CenterCell + FIntPoint(X, Y);
            FVector CellCenter = GridToWorld(Cell);
            
            // Draw cell boundaries
            FVector Min(CellCenter.X - CellSize * 0.5f, CellCenter.Y - CellSize * 0.5f, CellCenter.Z);
            FVector Max(CellCenter.X + CellSize * 0.5f, CellCenter.Y + CellSize * 0.5f, CellCenter.Z + 100.f);
            
            FColor CellColor = FColor::Green;
            
            // Highlight cells with actors
            if (const TArray<TWeakObjectPtr<AActor>>* CellActors = Grid.Find(Cell))
            {
                if (CellActors->Num() > 0)
                {
                    CellColor = FColor::Red;
                    
                    // Draw actor count
                    DrawDebugString(World, CellCenter + FVector(0, 0, 150.f), 
                        FString::Printf(TEXT("%d"), CellActors->Num()), 
                        nullptr, CellColor, Duration);
                }
            }
            
            DrawDebugBox(World, CellCenter + FVector(0, 0, 50.f), 
                FVector(CellSize * 0.5f, CellSize * 0.5f, 50.f), 
                CellColor, false, Duration, 0, 2.f);
        }
    }
}

void FSpatialHashGrid::PrintGridStats() const
{
    int32 TotalActors = GetTotalActorCount();
    int32 TotalCells = GetCellCount();
    float AvgActorsPerCell = TotalCells > 0 ? (float)TotalActors / TotalCells : 0.f;

    UE_LOG(LogTemp, Log, TEXT("=== Spatial Grid Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("Cell Size: %.1f"), CellSize);
    UE_LOG(LogTemp, Log, TEXT("Total Cells: %d"), TotalCells);
    UE_LOG(LogTemp, Log, TEXT("Total Actors: %d"), TotalActors);
    UE_LOG(LogTemp, Log, TEXT("Avg Actors/Cell: %.2f"), AvgActorsPerCell);

    // Find hotspots
    int32 MaxActorsInCell = 0;
    for (const auto& Pair : Grid)
    {
        MaxActorsInCell = FMath::Max(MaxActorsInCell, Pair.Value.Num());
    }
    UE_LOG(LogTemp, Log, TEXT("Max Actors in Single Cell: %d"), MaxActorsInCell);
}

// Private helper methods

void FSpatialHashGrid::AddActorToCell(AActor* Actor, const FIntPoint& Cell)
{
    TWeakObjectPtr<AActor> WeakActor(Actor);
    
    // Add to grid cell
    Grid.FindOrAdd(Cell).AddUnique(WeakActor);
    
    // Update reverse lookup
    ActorToCell.Add(WeakActor, Cell);
}

void FSpatialHashGrid::RemoveActorFromCell(AActor* Actor, const FIntPoint& Cell)
{
    TWeakObjectPtr<AActor> WeakActor(Actor);
    
    // Remove from grid cell
    if (TArray<TWeakObjectPtr<AActor>>* CellActors = Grid.Find(Cell))
    {
        CellActors->Remove(WeakActor);
        
        // Clean up empty cells
        if (CellActors->Num() == 0)
        {
            Grid.Remove(Cell);
        }
    }
    
    // Remove from reverse lookup
    ActorToCell.Remove(WeakActor);
}

// ============================================================================
// CONSOLE COMMAND REGISTRATION
// ============================================================================

// Static storage for console commands and grid pointer
static TUniquePtr<FAutoConsoleCommand> PrintStatsCommand;
static TUniquePtr<FAutoConsoleCommand> DebugDrawCommand;
static TUniquePtr<FAutoConsoleCommand> EnableCommand;
static TUniquePtr<FAutoConsoleCommand> DisableCommand;
static FSpatialHashGrid* GlobalGridPtr = nullptr;
static UWorld* GlobalWorldPtr = nullptr;
static UInteractionSubsystem* GlobalSubsystemPtr = nullptr;

void FSpatialHashGrid::RegisterConsoleCommands(UInteractionSubsystem* Subsystem)
{
    if (!Subsystem)
        return;
    
    // Prevent double-registration
    if (PrintStatsCommand.IsValid() || DebugDrawCommand.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Console commands already registered, skipping"));
        return;
    }
    
    // Store grid, world, and subsystem pointers for console commands to access
    GlobalGridPtr = Subsystem->GetSpatialGrid();
    GlobalWorldPtr = Subsystem->GetWorld();
    GlobalSubsystemPtr = Subsystem;
    
    if (!GlobalGridPtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register console commands: Spatial Grid not initialized"));
        return;
    }
    
    // Register PrintSpatialGridStats command
    PrintStatsCommand = MakeUnique<FAutoConsoleCommand>(
        TEXT("PrintSpatialGridStats"),
        TEXT("Prints statistics about the spatial hash grid (actor count, cell count, distribution)"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            if (!GlobalGridPtr)
            {
                UE_LOG(LogTemp, Warning, TEXT("Spatial Grid not initialized!"));
                return;
            }
            
            UE_LOG(LogTemp, Log, TEXT("======================================="));
            UE_LOG(LogTemp, Log, TEXT("SPATIAL HASH GRID STATS"));
            UE_LOG(LogTemp, Log, TEXT("======================================="));
            
            GlobalGridPtr->PrintGridStats();
            
            UE_LOG(LogTemp, Log, TEXT("======================================="));
        })
    );
    
    // Register DebugDrawSpatialGrid command
    DebugDrawCommand = MakeUnique<FAutoConsoleCommand>(
        TEXT("DebugDrawSpatialGrid"),
        TEXT("Draws visual representation of the spatial hash grid. Usage: DebugDrawSpatialGrid [Duration=5.0]"),
        FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
        {
            if (!GlobalGridPtr)
            {
                UE_LOG(LogTemp, Warning, TEXT("Spatial Grid not initialized!"));
                return;
            }
            
            if (!GlobalWorldPtr)
            {
                UE_LOG(LogTemp, Warning, TEXT("No valid world found!"));
                return;
            }
            
            // Parse duration argument
            float Duration = 5.f;
            if (Args.Num() > 0)
            {
                Duration = FCString::Atof(*Args[0]);
            }
            
            // Get first player location as center point
            FVector CenterLocation = FVector::ZeroVector;
            if (APlayerController* PC = GlobalWorldPtr->GetFirstPlayerController())
            {
                if (APawn* Pawn = PC->GetPawn())
                {
                    CenterLocation = Pawn->GetActorLocation();
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Drawing spatial grid (Duration: %.1fs, Center: %s)"), 
                Duration, *CenterLocation.ToString());
            
            // Draw grid centered on player (5 cell radius = 50 meters from player)
            GlobalGridPtr->DebugDrawGrid(GlobalWorldPtr, CenterLocation, 5, Duration);
        })
    );
    
    // Register EnableSpatialHashing command
    EnableCommand = MakeUnique<FAutoConsoleCommand>(
        TEXT("EnableSpatialHashing"),
        TEXT("Enables spatial hash grid optimization for interaction queries"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            if (!GlobalSubsystemPtr)
            {
                UE_LOG(LogTemp, Warning, TEXT("Interaction Subsystem not available!"));
                return;
            }
            
            GlobalSubsystemPtr->SetUseSpatialHashing(true);
            UE_LOG(LogTemp, Log, TEXT("✅ Spatial Hashing ENABLED - Using fast grid-based queries"));
        })
    );
    
    // Register DisableSpatialHashing command
    DisableCommand = MakeUnique<FAutoConsoleCommand>(
        TEXT("DisableSpatialHashing"),
        TEXT("Disables spatial hash grid, falls back to brute force queries"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            if (!GlobalSubsystemPtr)
            {
                UE_LOG(LogTemp, Warning, TEXT("Interaction Subsystem not available!"));
                return;
            }
            
            GlobalSubsystemPtr->SetUseSpatialHashing(false);
            UE_LOG(LogTemp, Warning, TEXT("⚠️ Spatial Hashing DISABLED - Using brute force queries (slower)"));
        })
    );
    
    UE_LOG(LogTemp, Log, TEXT("Spatial hash console commands registered: PrintSpatialGridStats, DebugDrawSpatialGrid, EnableSpatialHashing, DisableSpatialHashing"));
}

void FSpatialHashGrid::UnregisterConsoleCommands()
{
    // Only unregister if they exist
    if (PrintStatsCommand.IsValid() || DebugDrawCommand.IsValid() || EnableCommand.IsValid() || DisableCommand.IsValid())
    {
        PrintStatsCommand.Reset();
        DebugDrawCommand.Reset();
        EnableCommand.Reset();
        DisableCommand.Reset();
        GlobalGridPtr = nullptr;
        GlobalWorldPtr = nullptr;
        GlobalSubsystemPtr = nullptr;
        
        UE_LOG(LogTemp, Log, TEXT("Spatial hash console commands unregistered"));
    }
}