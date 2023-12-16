// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

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
                "DataValidation",
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
                "Niagara",
                "SoundFieldRendering", // Linux needs a symbold that it cannot find so we try to link this library by force.
                "Paper2D",
                "CinematicCamera",
                "AIModule",
                "NavigationSystem"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "GameplayMessageRuntime",
                "ModularGameplay",
                "CommonLoadingScreen",
                "ModularGameplayActors",
                "GameFeatures"
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
