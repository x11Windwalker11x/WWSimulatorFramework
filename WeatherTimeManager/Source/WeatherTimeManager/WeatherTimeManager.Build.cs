// Copyright Windwalker Productions. All Rights Reserved.

using UnrealBuildTool;

public class WeatherTimeManager : ModuleRules
{
	public WeatherTimeManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"UMG",
				"Slate",
				"SlateCore",
				"Windwalker_Productions_SharedDefaults",
				"ModularSystemsBase"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"NetCore"
			}
		);
	}
}
