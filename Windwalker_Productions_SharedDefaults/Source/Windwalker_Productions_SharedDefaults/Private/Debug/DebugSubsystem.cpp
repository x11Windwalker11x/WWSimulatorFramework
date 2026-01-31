#include "Debug/DebugSubsystem.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h" // Optional, but helps ensure StaticLoadObject is visible
// Include to ensure StaticLoadObject is defined:
#include "KismetTraceUtils.h"
#include "Logging/InteractableInventoryLogging.h"
#include "UObject/UnrealType.h" 

// ... (Other includes and function implementations)

void UDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Auto-find default asset
    if (DebugSettingsAsset.IsNull())
    {
        DebugSettingsAsset = TSoftObjectPtr<UDebugSettings>(
            FSoftObjectPath(TEXT("/Windwalker_Productions_SharedDefaults/DA_DebugSettings.DA_DebugSettings"))
        );
        UE_LOG(LogDebugSystem, Log, TEXT("No asset configured, setting path to default asset..."));
    }
    
    UDebugSettings* LoadedAsset = nullptr;

    // Load the asset using StaticLoadObject (most reliable method to avoid symbol errors)
    if (DebugSettingsAssetPath.IsValid())
    {
        LoadedAsset = Cast<UDebugSettings>(StaticLoadObject(UDebugSettings::StaticClass(), nullptr, *DebugSettingsAssetPath.ToString()));
    }
    
    // Fallback: Try loading from the TSoftObjectPtr if the path load failed or was invalid.
    if (!LoadedAsset && DebugSettingsAsset.IsValid())
    {
        LoadedAsset = DebugSettingsAsset.LoadSynchronous();
    }
    
    if (LoadedAsset)
    {
        DebugSettings = LoadedAsset; 
        DebugSettingsAsset = LoadedAsset; 
        
        UE_LOG(LogTemp, Log, TEXT("UDebugSubsystem: Successfully loaded Debug Settings Asset."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UDebugSubsystem: FAILED to load Debug Settings Asset from path: %s"), 
            *DebugSettingsAssetPath.ToString());
    }
    
    // Fallback to default object (Runs if DebugSettings is still nullptr after loading attempts)
    if (!DebugSettings)
    {
        DebugSettings = NewObject<UDebugSettings>(this);
        UE_LOG(LogDebugSystem, Warning, TEXT("No asset found or failed to load, using empty defaults"));
    }
    
    // ... (rest of initialize remains the same: Register console commands)
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("WW_Debug.ToggleLayer"),
        TEXT("Toggle a debug layer on/off. Usage: WW_Debug.ToggleLayer <LayerTagName>\nExample: WW_Debug.ToggleLayer Debug.Interaction"),
        FConsoleCommandWithArgsDelegate::CreateUObject(this, &UDebugSubsystem::ToggleDebugLayerCommand),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("WW_Debug.SetVerbosity"),
        TEXT("Set debug verbosity level. Usage: WW_Debug.SetVerbosity <0-5>\n0=None, 1=Critical, 2=Error, 3=Warning, 4=Info, 5=Verbose."),
        FConsoleCommandWithArgsDelegate::CreateUObject(this, &UDebugSubsystem::SetDebugVerbosityCommand),
        ECVF_Default
    );

    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("WW_Debug.ToggleTraces"),
        TEXT("Toggle trace visualization on/off. No arg calls will override all settings. Usage: WW_Debug.ToggleTraces(FString LayerTagName)"),
        FConsoleCommandWithArgsDelegate::CreateUObject(this, &UDebugSubsystem::ToggleTracesCommand),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("WW_Debug.ToggleCollision"),
        TEXT("Toggle collision shape visualization on/off. No arg calls will override all settings. Usage: WW_Debug.ToggleTraces(FString LayerTagName)"),
        FConsoleCommandWithArgsDelegate::CreateUObject(this, &UDebugSubsystem::ToggleCollisionCommand),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("WW_Debug.ToggleOnScreen"),
        TEXT("Toggle on-screen debug messages on/off"),
        FConsoleCommandWithArgsDelegate::CreateUObject(this, &UDebugSubsystem::ToggleOnScreenCommand),
        ECVF_Default
    );
}

void UDebugSubsystem::Deinitialize()
{
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("WW_Debug.ToggleLayer"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("WW_Debug.SetVerbosity"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("WW_Debug.ToggleTraces"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("WW_Debug.ToggleCollision"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("WW_Debug.ToggleOnScreen"));
    
    Super::Deinitialize();
}

bool UDebugSubsystem::IsLayerActive(FGameplayTag Layer) const
{
    if (!DebugSettings) return false;
    return DebugSettings->ActiveDebugLayers.HasTag(Layer);
}

FGameplayTagContainer UDebugSubsystem::GetActiveLayerTags() const
{
    if (!DebugSettings)
    {
        return FGameplayTagContainer();
    }
        
    return DebugSettings->ActiveDebugLayers;
}

FColor UDebugSubsystem::GetLayerColor(FGameplayTag Layer) const
{
    if (!DebugSettings) return FColor::White;
    
    FDebugLayerSettings* LayerSettings = DebugSettings->GetLayerSettings(Layer);
    return LayerSettings ? LayerSettings->DebugColor : FColor::White;
}

FDebugLayerSettings UDebugSubsystem::GetTraceSettingsForLayer(FGameplayTag Layer) const
{
    if (!DebugSettings) return FDebugLayerSettings();
    
    FDebugLayerSettings* LayerSettings = DebugSettings->GetLayerSettings(Layer);
    return LayerSettings ? *LayerSettings : FDebugLayerSettings();
}


// Version 2: Override with bShowOnScreen parameter
void UDebugSubsystem::PrintDebug(
    const UObject* WorldContext,
    FGameplayTag Layer,
    const FString& Message,
    bool bShowOnScreen,
    EDebugVerbosity Verbosity)
{
    if (!WorldContext) return;
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return;
    
    UDebugSubsystem* DebugSubsystem = GameInstance->GetSubsystem<UDebugSubsystem>();
    if (!DebugSubsystem) 
    {
        // Fallback to regular log if subsystem unavailable
        UE_LOG(LogTemp, Log, TEXT("[Debug] %s"), *Message);
        UE_LOG(LogTemp, Log, TEXT("DebugSubsystem Is not Avalaible fallen back to UE_LOG LogTemp"));
        return;
    }
    
    // Call the non-static version
    DebugSubsystem->PrintDebug(Layer, Message, bShowOnScreen, &LogDebugSystem, Verbosity);
}

void UDebugSubsystem::PrintDebug(
    FGameplayTag Layer,
    const FString& Message,
    bool bShowOnScreen,
    const FLogCategoryBase* LogCategory,
    EDebugVerbosity Verbosity)
{
    if (!DebugSettings || !IsLayerActive(Layer)) return;
    
    FDebugLayerSettings* LayerSettings = DebugSettings->GetLayerSettings(Layer);
    
    // Find layer-specific settings
    for (FDebugLayerSettings& Settings : DebugSettings->DebugLayerSettings)
    {
        if (Settings.LayerTag == Layer)
        {
            LayerSettings = &Settings;
            break;
        }
    }
    
    
    if (!LayerSettings) return;
    
    // Check verbosity against layer's verbosity
    if (Verbosity > LayerSettings->DebugVerbosity) return;

    // Show on screen if requested and layer allows it
    if (bShowOnScreen && LayerSettings->bShowOnScreenMessages && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, LayerSettings->OnScreenMessageDuration, LayerSettings->DebugColor, Message);
    }

    // Console log
    ELogVerbosity::Type LogVerbosity = ConvertDebugVerbosityToLogVerbosity(Verbosity);
    FString FormattedMessage = FString::Printf(TEXT("[%s] %s"), *Layer.ToString(), *Message);
    
    if (LogCategory)
    {
        GLog->Log(LogCategory->GetCategoryName(), LogVerbosity, FormattedMessage);
    }
    else
    {
        GLog->Log(LogDebugSystem.GetCategoryName(), LogVerbosity, FormattedMessage);
    }
}

ELogVerbosity::Type UDebugSubsystem::ConvertDebugVerbosityToLogVerbosity(EDebugVerbosity Verbosity) const
{
    switch (Verbosity)
    {
    case EDebugVerbosity::None:
        return ELogVerbosity::NoLogging;
    case EDebugVerbosity::Critical:
        return ELogVerbosity::Fatal;
    case EDebugVerbosity::Error:
        return ELogVerbosity::Error;
    case EDebugVerbosity::Warning:
        return ELogVerbosity::Warning;
    case EDebugVerbosity::Info:
        return ELogVerbosity::Log;
    case EDebugVerbosity::Verbose:
        return ELogVerbosity::Verbose;
    default:
        return ELogVerbosity::Log;
    }
}



void UDebugSubsystem::DrawDebugSphereForLayer(const UObject* WorldContext, FGameplayTag Layer, FVector Center, float Radius, int32 Segments)
{
    if (!WorldContext) return;
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return;
    
    UDebugSubsystem* DebugSubsystem = GameInstance->GetSubsystem<UDebugSubsystem>();
    if (!DebugSubsystem || !DebugSubsystem->DebugSettings) return;
    
    if (!DebugSubsystem->IsLayerActive(Layer)) return;
    
    // Find layer settings
    FDebugLayerSettings* LayerSettings = nullptr;
    for (FDebugLayerSettings& Settings : DebugSubsystem->DebugSettings->DebugLayerSettings)
    {
        if (Settings.LayerTag == Layer)
        {
            LayerSettings = &Settings;
            break;
        }
    }
    
    if (!LayerSettings || !LayerSettings->bShowCollisionShapes) return;
    
    bool bPersistent = (LayerSettings->TraceDrawType == EDrawDebugTrace::Persistent);
    float Duration = (LayerSettings->TraceDrawType == EDrawDebugTrace::ForOneFrame) ? 0.0f : LayerSettings->DebugDrawDuration;
    
    if (LayerSettings->TraceDrawType != EDrawDebugTrace::None)
    {
        DrawDebugSphere(World, Center, Radius, Segments, LayerSettings->DebugColor, bPersistent, Duration);
    }
}

void UDebugSubsystem::DrawDebugLineForLayer(const UObject* WorldContext, FGameplayTag Layer, FVector Start, FVector End, float Thickness)
{
    if (!WorldContext) return;
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return;
    
    UDebugSubsystem* DebugSubsystem = GameInstance->GetSubsystem<UDebugSubsystem>();
    if (!DebugSubsystem || !DebugSubsystem->DebugSettings) return;
    
    // Find layer settings
    FDebugLayerSettings* LayerSettings = nullptr;
    for (FDebugLayerSettings& Settings : DebugSubsystem->DebugSettings->DebugLayerSettings)
    {
        if (Settings.LayerTag == Layer)
        {
            LayerSettings = &Settings;
            break;
        }
    }
    if (!DebugSubsystem->IsLayerActive(Layer) || !LayerSettings->bShowTraces) return;
    
    bool bPersistent = (LayerSettings->TraceDrawType == EDrawDebugTrace::Persistent);
    float Duration = (LayerSettings->TraceDrawType == EDrawDebugTrace::ForOneFrame) ? 0.0f : LayerSettings->DebugDrawDuration;
    
    if (LayerSettings->TraceDrawType != EDrawDebugTrace::None)
    {
        DrawDebugLine(World, Start, End, DebugSubsystem->GetLayerColor(Layer), bPersistent, Duration, 0, Thickness);
        UE_LOG(LogInteraction, Warning, TEXT("Drawing debug line from %s to %s"), 
            *Start.ToString(), *End.ToString());
    }
}

FDebugLayerSettings UDebugSubsystem::GetLayerSettings(const UObject* WorldContext, FGameplayTag Layer)
{
    if (!WorldContext) return FDebugLayerSettings();
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return FDebugLayerSettings();
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return FDebugLayerSettings();
    
    UDebugSubsystem* DebugSubsystem = GameInstance->GetSubsystem<UDebugSubsystem>();
    if (!DebugSubsystem || !DebugSubsystem->DebugSettings) return FDebugLayerSettings();
    
    FDebugLayerSettings* Settings = DebugSubsystem->DebugSettings->GetLayerSettings(Layer);
    return Settings ? *Settings : FDebugLayerSettings();
}


void UDebugSubsystem::SetupComponentVisualization(
    UWorld* World,
    UPrimitiveComponent* Component,
    const FGameplayTag& DebugTag,
    bool bShouldLog)
{
    if (!World || !Component) return;

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return;

    UDebugSubsystem* DebugSubsystem = GameInstance->GetSubsystem<UDebugSubsystem>();
    if (!DebugSubsystem) return;

    // Check if layer is active
    if (DebugSubsystem->IsLayerActive(DebugTag))
    {
        // Show component
        Component->SetHiddenInGame(false);
        
        // Set color if it's a shape component
        if (UShapeComponent* ShapeComp = Cast<UShapeComponent>(Component))
        {
            ShapeComp->ShapeColor = DebugSubsystem->GetLayerColor(DebugTag);
        }
        
        // Optional debug print
        if (bShouldLog)
        {
            DebugSubsystem->PrintDebug(
                DebugTag,
                FString::Printf(TEXT("Componnet %s visualization enabled - Color: %s"),
                    *Component->GetName(),
                    *DebugSubsystem->GetLayerColor(DebugTag).ToString()),
                false,
                nullptr,
                EDebugVerbosity::Info
            );
        }
    }
    else
    {
        // Hide component when layer is not active
        Component->SetHiddenInGame(true);
    }
}

void UDebugSubsystem::SetupCollisionVisualization(
    const UObject* WorldContext,
    UShapeComponent* ShapeComponent,
    FGameplayTag Layer,
    bool bShouldLog)
{
    if (!WorldContext || !ShapeComponent) return;
    
    // Defaults from constants
    FColor Color = DebugDefaultsConstants::DEBUG_LAYER_DEBUGDEFAULT_LINEARCOLOR.ToFColor(true);
    bool bShowCollision = true;
    bool bIsActive = false;
    
    // Try to get from data asset
    FDebugLayerSettings Settings = GetLayerSettings(WorldContext, Layer);

    if (Settings.LayerTag.IsValid())
    {
        Color = Settings.DebugColor;
        bShowCollision = Settings.bShowCollisionShapes;
        bIsActive = Settings.bIsActive;
    }


    
    // Apply settings
    ShapeComponent->ShapeColor = Color;
    
    if (bIsActive && bShowCollision)
    {
        ShapeComponent->SetHiddenInGame(false);
    }
    else
    {
        ShapeComponent->SetHiddenInGame(true);
    }
    //Optional Log
    if (bShouldLog)
    {
        UDebugSubsystem::PrintDebug(
            WorldContext,
            Layer,
            FString::Printf(TEXT("%s Collision Component visualization enabled - Color: %s"),
                *ShapeComponent->GetName(),
                *Color.ToString()),
            false,
            EDebugVerbosity::Info
        );
    }

}



void UDebugSubsystem::ToggleDebugLayerCommand(const TArray<FString>& Args)
{
    if (Args.Num() > 0)
    {
        FGameplayTag LayerTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
        if (LayerTag.IsValid())
        {
            ToggleDebugLayer(LayerTag);
        }
        else
        {
            UE_LOG(LogDebugSystem, Warning, TEXT("Invalid gameplay tag: %s"), *Args[0]);
        }
    }
    else
    {
        UE_LOG(LogDebugSystem, Warning, TEXT("Usage: WW_Debug.ToggleLayer <TagName>"));
    }
}


void UDebugSubsystem::ToggleDebugLayer(FGameplayTag LayerTag)
{
    if (!DebugSettings || !LayerTag.IsValid()) 
    {
        UE_LOG(LogDebugSystem, Warning, TEXT("Cannot toggle layer: Invalid tag or DebugSettings is null"));
        return;
    }
    
    if (DebugSettings->ActiveDebugLayers.HasTag(LayerTag))
    {
        DebugSettings->ActiveDebugLayers.RemoveTag(LayerTag);
        UE_LOG(LogDebugSystem, Log, TEXT("Toggled debug layer %s: OFF"), *LayerTag.ToString());
    }
    else
    {
        DebugSettings->ActiveDebugLayers.AddTag(LayerTag);
        UE_LOG(LogDebugSystem, Log, TEXT("Toggled debug layer %s: ON"), *LayerTag.ToString());
    }
}

// UDebugSubsystem.cpp

void UDebugSubsystem::SetDebugVerbosity(FGameplayTag LayerTag, int32 Level)
{
    if (!DebugSettings)
    {
        UE_LOG(LogDebugSystem, Warning, TEXT("Cannot set verbosity: DebugSettings is null"));
        return;
    }
    
    // 1. Handle Global/Default Case (if LayerTag is non-valid)
    if (!LayerTag.IsValid())
    {
        if (Level >= 0 && Level <= 5)
        {
            DebugSettings->DebugLayerSettingsOverride.DebugVerbosity = static_cast<EDebugVerbosity>(Level);
            UE_LOG(LogDebugSystem, Log, TEXT("[Global]: DebugVerbosity set to: %d"), Level);
        }
        else
        {
            UE_LOG(LogDebugSystem, Warning, TEXT("[Global]: Invalid verbosity level: %d (valid range: 0-5)"), Level);
        }
        return;
    }

    // 2. Handle Layer-Specific Case (LayerTag is valid)

    // Find layer settings: Compare against LayerTag
    FDebugLayerSettings* LayerSettings = nullptr;
    for (FDebugLayerSettings& Settings : DebugSettings->DebugLayerSettings)
    {
        if (Settings.LayerTag == LayerTag) 
        {
            LayerSettings = &Settings;
            break;
        }
    }
    
    if (!LayerSettings)
    {
        UE_LOG(LogDebugSystem, Warning, TEXT("Cannot set verbosity: Layer %s not found in settings array"), *LayerTag.ToString());
        return;
    }
    
    if (Level >= 0 && Level <= 5)
    {
        LayerSettings->DebugVerbosity = static_cast<EDebugVerbosity>(Level);
        UE_LOG(LogDebugSystem, Log, TEXT("Layer %s verbosity set to: %d"), *LayerTag.ToString(), Level);
    }
    else
    {
        UE_LOG(LogDebugSystem, Warning, TEXT("Invalid verbosity level: %d (valid range: 0-5)"), Level);
    }
}

EDebugVerbosity UDebugSubsystem::GetDebugVerbosity(FGameplayTag LayerTag)
{
    EDebugVerbosity Verbosity = EDebugVerbosity::Info; // Default value

    if (!LayerTag.IsValid()) return Verbosity;
    
    // Iterate through all settings to find a match
    for (FDebugLayerSettings& Settings : DebugSettings->DebugLayerSettings)
    {
        if (Settings.LayerTag == LayerTag)
        {
            Verbosity = Settings.DebugVerbosity;
            break; // Found the match, exit loop
        }
    }

    return Verbosity; 
}

void UDebugSubsystem::SetDebugVerbosityCommand(const TArray<FString>& Args)
{
    // Initialize Level safely
    int32 Level = -1;
    FGameplayTag LayerTag = FGameplayTag::EmptyTag;
    
    // We require two arguments: LayerTag and Level
    if (Args.Num() >= 2)
    {
        // Args[0] is the LayerTagName string, Args[1] is the Level string
        LayerTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
        Level = FCString::Atoi(*Args[1]);
        
        // Call the worker function and RETURN
        SetDebugVerbosity(LayerTag, Level);
        return; 
    }
    
    // If fewer than 2 args, print error and exit.
    UE_LOG(LogDebugSystem, Error, TEXT("Usage: WW_Debug.SetVerbosity <LayerTagName> <0-5>"));
}


void UDebugSubsystem::ToggleTracesCommand(const TArray<FString>& Args)
{
    FGameplayTag InLayerTag;
    
    // Check if a tag name was provided as an argument
    if (Args.Num() > 0)
    {
        InLayerTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
    }
    
    if (DebugSettings)
    {
        if (!InLayerTag.IsValid())
        {
            // The existing logic for when no tag is passed (global toggle)
            FDebugLayerSettings* OverriddenDebugLayerSettings = &DebugSettings->DebugLayerSettingsOverride; 
            OverriddenDebugLayerSettings->bShowTraces = !OverriddenDebugLayerSettings->bShowTraces;
            UE_LOG(LogDebugSystem, Log, TEXT("Trace visualization (Global): %s"), 
                OverriddenDebugLayerSettings->bShowTraces ? TEXT("ON") : TEXT("OFF"));
            return; // Exit after global toggle
        }

        // Find layer settings (Layer-specific toggle)
        FDebugLayerSettings* LayerSettings = nullptr;
        for (FDebugLayerSettings& Settings : DebugSettings->DebugLayerSettings)
        {
            if (Settings.LayerTag == InLayerTag)
            {
                LayerSettings = &Settings;
                break;
            }
        }
        
        if (LayerSettings)
        {
            LayerSettings->bShowTraces = !LayerSettings->bShowTraces;
            UE_LOG(LogDebugSystem, Log, TEXT("Trace visualization (%s): %s"), 
                *InLayerTag.ToString(),
                LayerSettings->bShowTraces ? TEXT("ON") : TEXT("OFF"));
        }
        else
        {
            UE_LOG(LogDebugSystem, Warning, TEXT("Could not find settings for tag: %s"), *InLayerTag.ToString());
        }
    }
}

void UDebugSubsystem::ToggleCollisionCommand(const TArray<FString>& Args)
{
    FGameplayTag InLayerTag;
    
    if (Args.Num() > 0)
        InLayerTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
    if (DebugSettings)
    {
        if (!InLayerTag.IsValid())
        {
            FDebugLayerSettings* OverriddenDebugLayerSettings = &DebugSettings->DebugLayerSettingsOverride; 
            OverriddenDebugLayerSettings->bShowCollisionShapes = !OverriddenDebugLayerSettings->bShowCollisionShapes;
            UE_LOG(LogDebugSystem, Log, TEXT("Collision visualization: %s"), 
                OverriddenDebugLayerSettings->bShowCollisionShapes ? TEXT("ON") : TEXT("OFF"));
            return;
        }
        // Find layer settings
        FDebugLayerSettings* LayerSettings = nullptr;
        for (FDebugLayerSettings& Settings : DebugSettings->DebugLayerSettings)
        {
            if (Settings.LayerTag == InLayerTag)
            {
                LayerSettings = &Settings;
                break;
            }
        }
        
        if (LayerSettings)
        {
            LayerSettings->bShowCollisionShapes = !LayerSettings->bShowCollisionShapes;
            UE_LOG(LogDebugSystem, Log, TEXT("Collision visualization: %s"), 
                LayerSettings->bShowCollisionShapes ? TEXT("ON") : TEXT("OFF"));
        }
    }
}

void UDebugSubsystem::ToggleOnScreenCommand(const TArray<FString>& Args)
{
    FGameplayTag InLayerTag;
    if (Args.Num() > 0)
        InLayerTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
    
    if (DebugSettings)
    {
        if (!InLayerTag.IsValid())
        {
            FDebugLayerSettings* OverriddenDebugLayerSettings = &DebugSettings->DebugLayerSettingsOverride; 
            OverriddenDebugLayerSettings->bShowOnScreenMessages = !OverriddenDebugLayerSettings->bShowOnScreenMessages;
            UE_LOG(LogDebugSystem, Log, TEXT("Trace visualization: %s"), 
                OverriddenDebugLayerSettings->bShowOnScreenMessages ? TEXT("ON") : TEXT("OFF"));
            return;
        }
        // Find layer settings
        FDebugLayerSettings* LayerSettings = nullptr;
        for (FDebugLayerSettings& Settings : DebugSettings->DebugLayerSettings)
        {
            if (Settings.LayerTag == InLayerTag)
            {
                LayerSettings = &Settings;
                break;
            }
        }
        
        if (LayerSettings)
        {
            LayerSettings->bShowOnScreenMessages = !LayerSettings->bShowOnScreenMessages;
            UE_LOG(LogDebugSystem, Log, TEXT("On-screen messages: %s"), 
                LayerSettings->bShowOnScreenMessages ? TEXT("ON") : TEXT("OFF"));
        }
    }
}

// ============================================================================
// CRITICAL FIX: GetDebugSettingsAsset
// ============================================================================
UDebugSettings* UDebugSubsystem::GetDebugSettingsAsset()
{
    // Get the CDO (Class Default Object) of the subsystem
    UDebugSubsystem* CDO = GetMutableDefault<UDebugSubsystem>();
    
    if (!CDO)
    {
        UE_LOG(LogDebugSystem, Error, TEXT("Failed to get DebugSubsystem CDO"));
        return nullptr;
    }
    
    // If already loaded, return it
    if (CDO->DebugSettings)
    {
        return CDO->DebugSettings;
    }
    
    // Try to load from the asset path
    if (CDO->DebugSettingsAssetPath.IsValid())
    {
        UObject* LoadedObject = StaticLoadObject(
            UDebugSettings::StaticClass(), 
            nullptr, 
            *CDO->DebugSettingsAssetPath.ToString()
        );
        
        CDO->DebugSettings = Cast<UDebugSettings>(LoadedObject);
        
        if (CDO->DebugSettings)
        {
            UE_LOG(LogDebugSystem, Log, TEXT("Loaded DebugSettings from path: %s"), *CDO->DebugSettingsAssetPath.ToString());
            return CDO->DebugSettings;
        }
    }
    
    // Try loading from TSoftObjectPtr
    if (CDO->DebugSettingsAsset.IsValid())
    {
        CDO->DebugSettings = CDO->DebugSettingsAsset.LoadSynchronous();
        if (CDO->DebugSettings)
        {
            return CDO->DebugSettings;
        }
    }
    
    UE_LOG(LogDebugSystem, Error, TEXT("Failed to load DebugSettings asset"));
    return nullptr;
}

void UDebugSubsystem::DrawDebugCapsuleForLayer(
    const UObject* WorldContext, 
    FGameplayTag Layer, 
    FVector Start,
    FVector End,
    float HalfHeight, 
    float Radius, 
    const FRotator& Rotation,
    bool bHit,
    const TArray<FHitResult>& HitResults)
{
    if (!WorldContext) return;
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return;
    
    UDebugSubsystem* DebugSubsystem = GameInstance->GetSubsystem<UDebugSubsystem>();
    if (!DebugSubsystem || !DebugSubsystem->DebugSettings) return;
    
    if (!DebugSubsystem->IsLayerActive(Layer)) return;
    
    // Find layer settings
    FDebugLayerSettings* LayerSettings = nullptr;
    for (FDebugLayerSettings& Settings : DebugSubsystem->DebugSettings->DebugLayerSettings)
    {
        if (Settings.LayerTag == Layer)
        {
            LayerSettings = &Settings;
            break;
        }
    }
    
    if (!LayerSettings || !LayerSettings->bShowCollisionShapes) return;
    
    bool bPersistent = (LayerSettings->TraceDrawType == EDrawDebugTrace::Persistent);
    float Duration = (LayerSettings->TraceDrawType == EDrawDebugTrace::ForOneFrame) ? 
                     0.0f : LayerSettings->DebugDrawDuration;
    
    if (LayerSettings->TraceDrawType != EDrawDebugTrace::None)
    {
        FColor TraceColor = DebugSubsystem->GetLayerColor(Layer);
        FColor HitColor = FColor::Red;
        FQuat CapsuleRot = Rotation.Quaternion();
        
        // ✅ Draw capsule at start
        DrawDebugCapsule(
            World,
            Start,
            HalfHeight,
            Radius,
            CapsuleRot,
            TraceColor,
            bPersistent,
            Duration,
            0,
            2.0f
        );
        
        // ✅ Draw capsule at end (lighter)
        DrawDebugCapsule(
            World,
            End,
            HalfHeight,
            Radius,
            CapsuleRot,
            TraceColor.WithAlpha(100),
            bPersistent,
            Duration,
            0,
            1.0f
        );
        
        // ✅ Draw line showing sweep direction
        DrawDebugLine(
            World,
            Start,
            End,
            TraceColor,
            bPersistent,
            Duration,
            0,
            3.0f
        );
        
        // ✅ Draw hit points
        if (bHit)
        {
            for (const FHitResult& Hit : HitResults)
            {
                DrawDebugPoint(
                    World,
                    Hit.ImpactPoint,
                    10.0f,
                    HitColor,
                    bPersistent,
                    Duration
                );
                
                DrawDebugLine(
                    World,
                    Hit.ImpactPoint,
                    Hit.ImpactPoint + Hit.ImpactNormal * 50.0f,
                    HitColor,
                    bPersistent,
                    Duration,
                    0,
                    2.0f
                );
            }
        }
    }
}