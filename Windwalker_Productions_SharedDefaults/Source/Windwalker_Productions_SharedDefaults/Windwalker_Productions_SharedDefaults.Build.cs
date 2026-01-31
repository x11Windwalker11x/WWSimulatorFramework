// Copyright Windwalker Productions. All Rights Reserved.

using UnrealBuildTool;

public class Windwalker_Productions_SharedDefaults : ModuleRules
{
	public Windwalker_Productions_SharedDefaults(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags", 
			"AIModule"
		});
        
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});
	}
}