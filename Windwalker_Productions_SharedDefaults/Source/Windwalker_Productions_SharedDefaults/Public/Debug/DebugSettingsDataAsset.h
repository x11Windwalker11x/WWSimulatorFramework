#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WW_TagLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Debug/DebugDefaultsConstants.h"
#include "Logging/InteractableInventoryLogging.h"
#include "DebugSettingsDataAsset.generated.h"

UENUM(BlueprintType)
enum class EDebugVerbosity : uint8
{
    None,
    Critical,
    Error,
    Warning,
    Info,
    Verbose
};

USTRUCT(BlueprintType)
struct FDebugLayerSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers")
    FGameplayTag LayerTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers")
    bool bIsActive = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers")
    bool bShowCollisionShapes = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers")
    bool bShowTraces = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers|Colors")
    FColor DebugColor = FColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers|Colors")
    FColor TraceColor = DebugDefaultsConstants::DEBUG_LAYER_DEBUGDEFAULT_LINEARCOLOR.ToFColor(true);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers|Colors")
    FColor TraceHitColor = DebugDefaultsConstants::DEBUG_LAYER_DEBUGDEFAULT_LINEARCOLOR_HIT.ToFColor(true);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers", Meta = (ToolTip = "Overrides all trace and draw debug shape draw types."))
    TEnumAsByte<EDrawDebugTrace::Type> TraceDrawType = EDrawDebugTrace::ForDuration;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers")
    float DebugDrawDuration = DebugDefaultsConstants::DEBUG_DRAWDURATION;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers")
    bool bShowOnScreenMessages = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugLayers")
    float OnScreenMessageDuration = DebugDefaultsConstants::DEBUG_ONSCREENMESSAGEDURATION;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug Verbosity")
    EDebugVerbosity DebugVerbosity = EDebugVerbosity::Info;

    FDebugLayerSettings() {}

    FDebugLayerSettings(FLinearColor InTrace, FLinearColor InHit)
        : TraceColor(InTrace.ToFColor(true))
        , TraceHitColor(InHit.ToFColor(true)) 
    {}

    FDebugLayerSettings(
        FGameplayTag InTag,
        bool InIsActive,
        bool InbShowCollisionShapes,
        bool InShowTraces,
        FColor InDebugColor,
        FColor InTraceColor,
        FColor InTraceHitColor,
        TEnumAsByte<EDrawDebugTrace::Type> InTraceDrawType,
        float InDebugDrawDuration,
        bool InbShowOnScreenMessages,
        float InOnScreenMessageDuration,
        EDebugVerbosity InDebugVerbosity)
        : LayerTag(InTag)
        , bIsActive(InIsActive)
        , bShowCollisionShapes(InbShowCollisionShapes)
        , bShowTraces(InShowTraces)
        , DebugColor(InDebugColor)
        , TraceColor(InTraceColor)
        , TraceHitColor(InTraceHitColor)
        , TraceDrawType(InTraceDrawType)
        , DebugDrawDuration(InDebugDrawDuration)
        , bShowOnScreenMessages(InbShowOnScreenMessages)
        , OnScreenMessageDuration(InOnScreenMessageDuration)
        , DebugVerbosity(InDebugVerbosity)
    {}
};

UCLASS(BlueprintType)
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API UDebugSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Layers",
        meta=(ToolTip="Shows all 14 available debug layers."))
    FGameplayTagContainer DebugLayerPool;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Layers",
        meta=(ToolTip="Shows currently active layers based on settings."))
    FGameplayTagContainer ActiveDebugLayers;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug Layers")
    TArray<FDebugLayerSettings> DebugLayerSettings;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug Configuration Override",
        meta=(ToolTip="When enabled, ALL layers become active with override settings."))
    bool bOverrideAll = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug Configuration Override",
        meta=(ToolTip="Override settings. bIsActive here controls all layers when bOverrideAll is false."))
    FDebugLayerSettings DebugLayerSettingsOverride;
        
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
    {
        Super::PostEditChangeProperty(PropertyChangedEvent);
        
        FName PropertyName = PropertyChangedEvent.GetPropertyName();
        
        if (PropertyName == GET_MEMBER_NAME_CHECKED(UDebugSettings, DebugLayerPool))
        {
            RegenerateLayersFromTags();
            UpdateActiveDebugLayers();
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UDebugSettings, bOverrideAll))
        {
            UpdateActiveDebugLayers();
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UDebugSettings, DebugLayerSettings))
        {
            UpdateActiveDebugLayers();
        }
    }
    
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override
    {
        Super::PostEditChangeChainProperty(PropertyChangedEvent);
        UpdateActiveDebugLayers();
    }

    UFUNCTION(CallInEditor, Category="Debug Layers",
        meta=(ToolTip="Clears and reinitializes all 14 debug layers with defaults from code. WARNING: This will reset all your custom settings!"))
    void ReinitializeAllLayers()
    {
        DebugLayerSettings.Empty();
        DebugLayerPool.Reset();
        
        TArray<FDebugLayerSettings> AllLayers;
        CreateAllLayers(AllLayers);
        
        for (const FDebugLayerSettings& Layer : AllLayers)
        {
            DebugLayerSettings.Add(Layer);
            if (Layer.LayerTag.IsValid())
            {
                DebugLayerPool.AddTag(Layer.LayerTag);
            }
        }
        
        UpdateActiveDebugLayers();
        Modify();
    }

    UFUNCTION(CallInEditor, Category="Debug Layers",
        meta=(ToolTip="Adds any missing debug layers from code while preserving existing layers and their settings."))
    void AddMissingLayers()
    {
        TArray<FGameplayTag> ExistingTags;
        for (const FDebugLayerSettings& Layer : DebugLayerSettings)
        {
            if (Layer.LayerTag.IsValid())
            {
                ExistingTags.Add(Layer.LayerTag);
            }
        }

        TArray<FDebugLayerSettings> AllLayers;
        CreateAllLayers(AllLayers);

        int32 AddedCount = 0;
        for (const FDebugLayerSettings& Layer : AllLayers)
        {
            if (Layer.LayerTag.IsValid() && !ExistingTags.Contains(Layer.LayerTag))
            {
                DebugLayerSettings.Add(Layer);
                DebugLayerPool.AddTag(Layer.LayerTag);
                AddedCount++;
                UE_LOG(LogDebugSystem, Log, TEXT("Added missing debug layer: %s"), *Layer.LayerTag.ToString());
            }
        }

        if (AddedCount == 0)
        {
            UE_LOG(LogDebugSystem, Log, TEXT("No missing layers found. All layers already exist."));
        }
        else
        {
            UE_LOG(LogDebugSystem, Log, TEXT("Added %d missing debug layer(s)."), AddedCount);
        }

        UpdateActiveDebugLayers();
        Modify();
    }
#endif

    void CreateAllLayers(TArray<FDebugLayerSettings>& OutLayers)
    {
        auto CreateLayer = [](FName TagName, FLinearColor Color, FLinearColor HitColor) -> FDebugLayerSettings
        {
            FDebugLayerSettings Layer;
            Layer.LayerTag = FGameplayTag::RequestGameplayTag(TagName);
            Layer.bIsActive = true;
            Layer.bShowCollisionShapes = DebugDefaultsConstants::DEBUG_SHOWCOLLISIONSHAPES;
            Layer.bShowTraces = DebugDefaultsConstants::DEBUG_SHOWTRACES;
            Layer.DebugColor = Color.ToFColor(true);
            Layer.TraceColor = Color.ToFColor(true);
            Layer.TraceHitColor = HitColor.ToFColor(true);
            Layer.DebugDrawDuration = DebugDefaultsConstants::DEBUG_DRAWDURATION;
            Layer.bShowOnScreenMessages = DebugDefaultsConstants::DEBUG_SHOWONSCREENMESSAGES;
            Layer.OnScreenMessageDuration = DebugDefaultsConstants::DEBUG_ONSCREENMESSAGEDURATION;
            return Layer;
        };

        OutLayers.Add(CreateLayer(TEXT("Debug"), 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.Widget"), 
            DebugDefaultsConstants::DEBUG_LAYER_WIDGET_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_WIDGET_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.Interaction"), 
            DebugDefaultsConstants::DEBUG_LAYER_INTERACTION_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_INTERACTION_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.Inventory"), 
            DebugDefaultsConstants::DEBUG_LAYER_INVENTORY_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_INVENTORY_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.AI"), 
            DebugDefaultsConstants::DEBUG_LAYER_AI_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_AI_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.Combat"), 
            DebugDefaultsConstants::DEBUG_LAYER_COMBAT_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_COMBAT_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.CQC"), 
            DebugDefaultsConstants::DEBUG_LAYER_CQC_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_CQC_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.Parkour"), 
            DebugDefaultsConstants::DEBUG_LAYER_PARKOUR_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_PARKOUR_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.Movement"), 
            DebugDefaultsConstants::DEBUG_LAYER_MOVEMENT_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_MOVEMENT_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.Character"), 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.Pawn"), 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.Camera"), 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.InventoryInteraction"), 
            DebugDefaultsConstants::DEBUG_LAYER_INVENTORYINTERACTION_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_INVENTORYINTERACTION_LINEARCOLOR_HIT));

        OutLayers.Add(CreateLayer(TEXT("Debug.SaveSystem"), 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR, 
            DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR_HIT));
    }

    FDebugLayerSettings* GetLayerSettings(FGameplayTag LayerTag)
    {
        if (bOverrideAll)
        {
            return &DebugLayerSettingsOverride;
        }
        
        for (FDebugLayerSettings& Settings : DebugLayerSettings)
        {
            if (Settings.LayerTag == LayerTag)
            {
                return &Settings;
            }
        }
        return nullptr;
    }

    FGameplayTagContainer GetActiveLayers()
    {
        FGameplayTagContainer Result;

        if (bOverrideAll)
        {
            for (const FDebugLayerSettings& Settings : DebugLayerSettings)
            {
                if (Settings.LayerTag.IsValid())
                {
                    Result.AddTag(Settings.LayerTag);
                }
            }
        }
        else if (DebugLayerSettingsOverride.bIsActive)
        {
            for (const FDebugLayerSettings& Settings : DebugLayerSettings)
            {
                if (Settings.LayerTag.IsValid())
                {
                    Result.AddTag(Settings.LayerTag);
                }
            }
        }
        else
        {
            for (const FDebugLayerSettings& Settings : DebugLayerSettings)
            {
                if (Settings.bIsActive && Settings.LayerTag.IsValid())
                {
                    Result.AddTag(Settings.LayerTag);
                }
            }
        }
    
        return Result;
    }

    void LogActiveLayers(bool bLog = true, EDebugVerbosity InDebugVerbosity = EDebugVerbosity::Info, 
        bool bAddOnScreenDebugMessage = false, float OnScreenDebugMessageDuration = DebugDefaultsConstants::DEBUG_ONSCREENMESSAGEDURATION)
    {
        if (!bLog) return;
    
        FString Message = FString::Printf(TEXT("Active Layers: %s"), *ActiveDebugLayers.ToString());
    
        switch (InDebugVerbosity)
        {
        case EDebugVerbosity::Critical:
            UE_LOG(LogDebugSystem, Fatal, TEXT("%s"), *Message);
            break;
        case EDebugVerbosity::Error:
            UE_LOG(LogDebugSystem, Error, TEXT("%s"), *Message);
            break;
        case EDebugVerbosity::Warning:
            UE_LOG(LogDebugSystem, Warning, TEXT("%s"), *Message);
            break;
        case EDebugVerbosity::Verbose:
            UE_LOG(LogDebugSystem, Verbose, TEXT("%s"), *Message);
            break;
        case EDebugVerbosity::Info:
        default:
            UE_LOG(LogDebugSystem, Log, TEXT("%s"), *Message);
            break;
        }
    
        if (bAddOnScreenDebugMessage && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, OnScreenDebugMessageDuration, FColor::White, Message);
        }
    }

private:
    FLinearColor GetDefaultColorForTag(FGameplayTag Tag)
    {
        FString TagString = Tag.ToString();
        
        if (TagString.Contains("InventoryInteraction")) return DebugDefaultsConstants::DEBUG_LAYER_INVENTORYINTERACTION_LINEARCOLOR;
        if (TagString.Contains("Interaction")) return DebugDefaultsConstants::DEBUG_LAYER_INTERACTION_LINEARCOLOR;
        if (TagString.Contains("Inventory")) return DebugDefaultsConstants::DEBUG_LAYER_INVENTORY_LINEARCOLOR;
        if (TagString.Contains("Combat")) return DebugDefaultsConstants::DEBUG_LAYER_COMBAT_LINEARCOLOR;
        if (TagString.Contains("AI")) return DebugDefaultsConstants::DEBUG_LAYER_AI_LINEARCOLOR;
        if (TagString.Contains("CQC")) return DebugDefaultsConstants::DEBUG_LAYER_CQC_LINEARCOLOR;
        if (TagString.Contains("Parkour")) return DebugDefaultsConstants::DEBUG_LAYER_PARKOUR_LINEARCOLOR;
        if (TagString.Contains("Movement")) return DebugDefaultsConstants::DEBUG_LAYER_MOVEMENT_LINEARCOLOR;
        if (TagString.Contains("Widget")) return DebugDefaultsConstants::DEBUG_LAYER_WIDGET_LINEARCOLOR;
        
        return DebugDefaultsConstants::DEBUG_LAYER_DEBUG_LINEARCOLOR;
    }

    void UpdateActiveDebugLayers()
    {
        ActiveDebugLayers.Reset();
        DebugLayerPool.Reset();

        if (bOverrideAll)
        {
            for (const FDebugLayerSettings& Settings : DebugLayerSettings)
            {
                if (Settings.LayerTag.IsValid())
                {
                    DebugLayerPool.AddTag(Settings.LayerTag);
                    ActiveDebugLayers.AddTag(Settings.LayerTag);
                }
            }
        }
        else if (DebugLayerSettingsOverride.bIsActive)
        {
            for (const FDebugLayerSettings& Settings : DebugLayerSettings)
            {
                if (Settings.LayerTag.IsValid())
                {
                    DebugLayerPool.AddTag(Settings.LayerTag);
                    ActiveDebugLayers.AddTag(Settings.LayerTag);
                }
            }
        }
        else
        {
            for (const FDebugLayerSettings& Settings : DebugLayerSettings)
            {
                if (Settings.LayerTag.IsValid())
                {
                    DebugLayerPool.AddTag(Settings.LayerTag);
                
                    if (Settings.bIsActive)
                    {
                        ActiveDebugLayers.AddTag(Settings.LayerTag);
                    }
                }
            }
        }
    }

    void RegenerateLayersFromTags()
    {
        TArray<FDebugLayerSettings> ExistingSettings = DebugLayerSettings;
        DebugLayerSettings.Empty();
    
        TArray<FGameplayTag> TagArray;
        DebugLayerPool.GetGameplayTagArray(TagArray);
    
        for (const FGameplayTag& Tag : TagArray)
        {
            FDebugLayerSettings* ExistingSetting = ExistingSettings.FindByPredicate(
                [&Tag](const FDebugLayerSettings& Settings) { return Settings.LayerTag == Tag; });
        
            if (ExistingSetting)
            {
                DebugLayerSettings.Add(*ExistingSetting);
            }
            else
            {
                FDebugLayerSettings NewSettings;
                NewSettings.LayerTag = Tag;
                NewSettings.bIsActive = true;
            
                FLinearColor DefaultColor = GetDefaultColorForTag(Tag);
                NewSettings.DebugColor = DefaultColor.ToFColor(true);
                NewSettings.TraceColor = DefaultColor.ToFColor(true);
                NewSettings.TraceHitColor = DefaultColor.ToFColor(true);
                NewSettings.DebugVerbosity = EDebugVerbosity::Info;
                NewSettings.bShowOnScreenMessages = DebugDefaultsConstants::DEBUG_SHOWONSCREENMESSAGES;
                NewSettings.bShowTraces = DebugDefaultsConstants::DEBUG_SHOWTRACES;
                NewSettings.bShowCollisionShapes = DebugDefaultsConstants::DEBUG_SHOWCOLLISIONSHAPES;
                NewSettings.OnScreenMessageDuration = DebugDefaultsConstants::DEBUG_ONSCREENMESSAGEDURATION;
                NewSettings.DebugDrawDuration = DebugDefaultsConstants::DEBUG_DRAWDURATION;
                NewSettings.TraceDrawType = EDrawDebugTrace::ForDuration;
            
                DebugLayerSettings.Add(NewSettings);
            }
        }
    }
};