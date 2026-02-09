// Copyright Windwalker Productions. All Rights Reserved.

using UnrealBuildTool;

public class ModularEconomyPlugin : ModuleRules
{
	public ModularEconomyPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
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

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
