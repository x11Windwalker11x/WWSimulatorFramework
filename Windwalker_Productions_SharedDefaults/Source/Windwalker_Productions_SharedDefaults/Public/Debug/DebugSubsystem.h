#pragma once

#include "CoreMinimal.h"
#include "Components/ShapeComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "DebugSettingsDataAsset.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Logging/LogMacros.h"
#include "DebugSubsystem.generated.h"

struct FLogCategoryBase;

UCLASS(Config = Engine)
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API UDebugSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY()
    TSoftObjectPtr<UDebugSettings> DebugSettingsAsset;
    
    //DebugSettings DA asset path to use within editor
    UPROPERTY(Config, EditDefaultsOnly, Category = "Debug", meta = (AllowedClasses = "/Script/Windwalker_Productions_SharedDefaults.DebugSettings"))
    FSoftObjectPath DebugSettingsAssetPath;
    
    UPROPERTY()
    UDebugSettings* DebugSettings;
    
    // Static func to use within editor 
    // This is the function we will fix in the .cpp to be more robust.
    UFUNCTION(BlueprintCallable, Category = "Debug")
    static UDebugSettings* GetDebugSettingsAsset();

    
    UFUNCTION(BlueprintCallable, Category="Debug")
    bool IsLayerActive(FGameplayTag Layer) const;
    
    UFUNCTION(BlueprintCallable, Category="Debug")
    FGameplayTagContainer GetActiveLayerTags() const;
    UFUNCTION(BlueprintCallable, Category="Debug")
    FColor GetLayerColor(FGameplayTag Layer) const;
    
    UFUNCTION(BlueprintCallable, Category="Debug")
    FDebugLayerSettings GetTraceSettingsForLayer(FGameplayTag Layer) const;

    UFUNCTION(BlueprintCallable, Category="Debug")
    static void PrintDebug(
        const UObject* WorldContext,
        FGameplayTag Layer,
        const FString& Message,
        bool bShowOnScreen = false,
        EDebugVerbosity Verbosity = EDebugVerbosity::Info
    );

  
    // Version 2: Override with bShowOnScreen parameter
    void PrintDebug(FGameplayTag Layer, const FString& Message, bool bShowOnScreen,
        const FLogCategoryBase* LogCategory = &LogDebugSystem, 
        EDebugVerbosity Verbosity = EDebugVerbosity::Info);
    
    UFUNCTION(BlueprintCallable, Category="Debug")
    static void DrawDebugLineForLayer(const UObject* WorldContext, FGameplayTag Layer, FVector Start, FVector End, float Thickness = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category="Debug")
    static void DrawDebugCapsuleForLayer(
    const UObject* WorldContext, 
    FGameplayTag Layer, 
    FVector Start,
    FVector End,
    float HalfHeight, 
    float Radius, 
    const FRotator& Rotation,
    bool bHit,  // ✅ Add this - did trace hit?
    const TArray<FHitResult>& HitResults);  // ✅ Add this - hit results

    
    UFUNCTION(BlueprintCallable, Category="Debug")
    static void DrawDebugSphereForLayer(const UObject* WorldContext, FGameplayTag Layer, FVector Center, float Radius, int32 Segments = 16);



    /**
 * Safely setup debug visualization for a component
 * Static - handles all null checks internally
 */
UFUNCTION(BlueprintCallable, Category="Debug", meta=(WorldContext="WorldContext"))
    static void SetupComponentVisualization(
        UWorld* World,
        UPrimitiveComponent* Component,
        const FGameplayTag& DebugTag,
        bool bShouldLog = false
    );
UFUNCTION(BlueprintCallable, Category="Debug", meta=(WorldContext="WorldContext"))
    static void SetupCollisionVisualization(
        const UObject* WorldContext,
        UShapeComponent* ShapeComponent,
        FGameplayTag Layer,
        bool bShouldLog = false
    );

    UFUNCTION(BlueprintCallable, Category="Debug")
    void ToggleDebugLayer(FGameplayTag LayerTag);
    
    UFUNCTION(BlueprintCallable, Category="Debug")
    void SetDebugVerbosity(FGameplayTag LayerTag, int32 Level);

    UFUNCTION(BlueprintCallable, Category="Debug")
    EDebugVerbosity GetDebugVerbosity(FGameplayTag LayerTag);

    UFUNCTION(BlueprintCallable, Category="Debug")
    static FDebugLayerSettings GetLayerSettings(const UObject* WorldContext, FGameplayTag Layer);
    static FDebugLayerSettings GetLayerSettingsOverride(const UObject* WorldContext, FGameplayTag Layer);


private:
    ELogVerbosity::Type ConvertDebugVerbosityToLogVerbosity(EDebugVerbosity Verbosity) const;
    //Console Commands
    void ToggleDebugLayerCommand(const TArray<FString>& Args);
    void SetDebugVerbosityCommand(const TArray<FString>& Args);
    void ToggleTracesCommand(const TArray<FString>& Args);
    void ToggleCollisionCommand(const TArray<FString>& Args);
    void ToggleOnScreenCommand(const TArray<FString>& Args);
};