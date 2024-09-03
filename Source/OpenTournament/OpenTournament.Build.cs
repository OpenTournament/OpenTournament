// Copyright (c) Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;

public class OpenTournament : ModuleRules
{
    public OpenTournament(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        ShadowVariableWarningLevel = WarningLevel.Error;

        PrivatePCHHeaderFile = "OpenTournament.h";

        // Reference:
        // https://docs.unrealengine.com/en-US/Programming/BuildTools/UnrealBuildTool/ModuleFiles/index.html
        PublicIncludePaths.AddRange
        (
            new string[]
            {
                "OpenTournament",
                "OpenTournament/AbilitySystem",
                "OpenTournament/AnnouncementSystem",
                "OpenTournament/AI",
                "OpenTournament/Camera",
                "OpenTournament/Character",
                "OpenTournament/Data",
                "OpenTournament/Enums",
                "OpenTournament/GameFeatures",
                "OpenTournament/GameModes",
                "OpenTournament/GAS",
                "OpenTournament/Input",
                "OpenTournament/Interfaces",
                "OpenTournament/Messages",
                "OpenTournament/Player",
                //"OpenTournament/Settings",
                "OpenTournament/Slate",
                "OpenTournament/System",
                "OpenTournament/Teams",
                "OpenTournament/UI",
                "OpenTournament/Utilities",
                //"OpenTournament/Weapons",
                "OpenTournament/Widgets"
            }
        );

        PrivateIncludePaths.AddRange
        (
            new string[] {
            }
        );

        PublicDependencyModuleNames.AddRange
        (
            new string[]
            {
                "AIModule",
                "ApplicationCore",
                "Core",
                "CoreOnline",
                "CoreUObject",
                "CinematicCamera",
                "DeveloperSettings",
                "Engine",
                "EngineSettings",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "InputCore",
                "MoviePlayer",
                "NavigationSystem",
                "NetCore",
                "Niagara",
                "Paper2D",
                "PhysicsCore",
                "ReplicationGraph",
                "Slate",
                "SlateCore",
                "SoundFieldRendering", // Linux needs a symbold that it cannot find so we try to link this library by force.
                "UIExtension",
                "UMG"
            }
        );

        PrivateDependencyModuleNames.AddRange
        (
            new string[]
            {
                "InputCore",
                "Slate",
                "SlateCore",
                "RenderCore",
                "DeveloperSettings",
                "EnhancedInput",
                "RHI",
                "Projects",
                "Gauntlet",
                "UMG",
                "CommonUI",
                "CommonInput",
                "CommonGame",
                "CommonUser",
                "GameSettings",
                "GameSubtitles",
                "GameplayMessageRuntime",
                "AudioMixer",
                "NetworkReplayStreaming",
                "UIExtension",
                "ClientPilot",
                "AudioModulation",
                "EngineSettings",
                "DTLSHandlerComponent",
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
                //"AudioModulation",
                "CommonGame",
                "CommonInput",
                "CommonLoadingScreen",
                "CommonUI",
                "CommonUser",
                "EnhancedInput",
                "GameFeatures",
                "GameplayMessageRuntime",
                "ModularGameplay",
                "ModularGameplayActors"
            }
        );

        SetupGameplayDebuggerSupport(Target);
        SetupIrisSupport(Target);
    }
}
