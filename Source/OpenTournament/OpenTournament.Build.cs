// Copyright (c) Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;

public class OpenTournament : ModuleRules
{
    public OpenTournament(ReadOnlyTargetRules Target) : base(Target)
    {
        ShadowVariableWarningLevel = WarningLevel.Error;

        PrivatePCHHeaderFile = "OpenTournament.h";
        PublicDependencyModuleNames.AddRange
        (
            new string[]
            {
                "Core",
                "CoreUObject",
                "DeveloperSettings",
                "Engine",
                "EngineSettings",
                "InputCore",
                "UMG",
                "Slate",
                "SlateCore",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "MoviePlayer",
                "NetCore",
                "Niagara",
                "SoundFieldRendering", // Linux needs a symbold that it cannot find so we try to link this library by force.
                "Paper2D",
                "CinematicCamera",
                "AIModule",
                "NavigationSystem"
            }
        );

		if (Target.bBuildEditor)
		{
            PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"DataValidation"
				}
			);
		}

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "AudioModulation",
                "CommonInput",
                "CommonLoadingScreen",
                "CommonUI",
                "CommonUser",
                "EnhancedInput",
                "GameFeatures",
                "GameplayMessageRuntime",
                "ModularGameplay",
                "ModularGameplayActors",
            }
        );

        // Reference:
        // https://docs.unrealengine.com/en-US/Programming/BuildTools/UnrealBuildTool/ModuleFiles/index.html
        PublicIncludePaths.AddRange
        (
            new string[]
            {
                "OpenTournament",
                "OpenTournament/AnnouncementSystem",
                "OpenTournament/Data",
                "OpenTournament/Enums",
                "OpenTournament/GAS",
                "OpenTournament/Interfaces",
                "OpenTournament/Slate",
                "OpenTournament/Utilities",
                "OpenTournament/Widgets"
            }
        );
    }
}
