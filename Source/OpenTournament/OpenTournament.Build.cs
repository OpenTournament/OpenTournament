// Copyright 2019 Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;

public class OpenTournament : ModuleRules
{
	public OpenTournament(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
