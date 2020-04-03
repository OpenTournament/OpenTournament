// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class OpenTournament : ModuleRules
{
    public OpenTournament(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange
        (
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "UMG",
                "Slate",
                "SlateCore",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks"
            }
        );
    }
}
