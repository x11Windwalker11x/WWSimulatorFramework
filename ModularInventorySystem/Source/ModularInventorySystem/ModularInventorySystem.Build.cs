// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ModularInventorySystem : ModuleRules
{
	public ModularInventorySystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public")
			}
		);
             
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				
			}
		);
          
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UMG",
				"Slate",
				"SlateCore",
				"InputCore",
				"Windwalker_Productions_SharedDefaults",
				"ModularSystemsBase",
				"ModularSpawnSystem",
				"ModularInteractionSystem",
				"Projects",
				"GameplayTags",
				"Json",
				"JsonUtilities",
			}
		);
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"Projects", // Remove duplicate if you want
				"AssetRegistry"
			}
		);
       
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
		
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] { 
					"UnrealEd", 
					"EditorStyle",
				}
			);
			PrivateIncludePathModuleNames.AddRange(
				new string[] { "UnrealEd", "AssetTools", "EditorStyle" }
			);
		}
	}
}