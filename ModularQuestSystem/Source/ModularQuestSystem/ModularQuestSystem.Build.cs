// Copyright Windwalker Productions. All Rights Reserved.

using UnrealBuildTool;

public class ModularQuestSystem : ModuleRules
{
	public ModularQuestSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
	}
}
