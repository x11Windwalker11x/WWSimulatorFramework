// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class ModularPlayerController : ModuleRules
{
    public ModularPlayerController(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "../../../ModularInventorySystem/Source/ModularInventorySystem/Public"),
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "../../../Windwalker_Productions_SharedDefaults/Source/Windwalker_Productions_SharedDefaults/Public"),
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "../../../AdvancedWidgetFramework/Source/AdvancedWidgetFramework/Public"),
                Path.Combine(ModuleDirectory, "Public")

                //Path.Combine(ModuleDirectory, "Public/Interface")
                    
                
            }
        );


        PrivateIncludePaths.AddRange(
            new string[] {
                "ModularPlayerController/Private"
            }
        );

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "AIModule",
            "Slate",
            "SlateCore",
            "UMG",
            "InputCore",
            "EnhancedInput",
            "ChaosModularVehicleEngine",
            "OnlineSubsystem",     
            "OnlineSubsystemUtils",
            "GameplayTags",
            
            //
            "Windwalker_Productions_SharedDefaults",
            "ModularSystemsBase",
            "ModularInventorySystem",
            "ModularInteractionSystem",
            "ModularCheatManager",
            "AdvancedWidgetFramework",
            "ModularSaveGameSystem",
            "ModularSpawnSystem" 
        });
    


PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Slate",
                "SlateCore"
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                // Add dynamically loaded modules here if any
            }
        );
    }
}
