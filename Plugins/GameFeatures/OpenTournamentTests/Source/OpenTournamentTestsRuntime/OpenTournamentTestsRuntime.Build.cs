// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OpenTournamentTestsRunTime : ModuleRules
{
	public OpenTournamentTestsRunTime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"OpenTournament",
				"GameplayTags",
				"GameplayAbilities",
				"ModularGameplay"
				// ... add other public dependencies that you statically link with here ...
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"InputCore",
				"EnhancedInput",
				"CQTest",
				"CQTestEnhancedInput",
				// ... add private dependencies that you statically link with here ...
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"EngineSettings",
					"LevelEditor",
					"UnrealEd"
			});
		}
	}
}
