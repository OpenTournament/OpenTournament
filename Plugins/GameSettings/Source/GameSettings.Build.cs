// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameSettings : ModuleRules
{
	public GameSettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;				
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"InputCore",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"CommonInput",
				"CommonUI",
				"GameplayTags"
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"PropertyPath"
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
