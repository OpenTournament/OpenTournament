// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

using UnrealBuildTool;

public class OpenTournament : ModuleRules
{
    public OpenTournament(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        IWYUSupport = IWYUSupport.KeepAsIs;

        // Reference:
        // https://docs.unrealengine.com/en-US/Programming/BuildTools/UnrealBuildTool/ModuleFiles/index.html
        PublicIncludePaths.AddRange
        (
            new []
            {
                "OpenTournament",
                "OpenTournament/AI",
                "OpenTournament/AbilitySystem",
                "OpenTournament/AnnouncementSystem",
                "OpenTournament/AI",
                "OpenTournament/Camera",
                "OpenTournament/Character",
                "OpenTournament/Data",
                "OpenTournament/Enums",
                "OpenTournament/GAS",
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
                "OpenTournament/Tags",
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
            new []
            {
                "AIModule",
                "ApplicationCore",
                "AsyncMixin",
				"CommonLoadingScreen",
                "ControlFlows",
                "Core",
                "CoreOnline",
                "CoreUObject",
                "CinematicCamera",
				"DataRegistry",
                "DeveloperSettings",
                "Engine",
                "EngineSettings",
                "GameFeatures",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "Hotfix",
                "InputCore",
                "ModularGameplay",
                "ModularGameplayActors",
                "MoviePlayer",
                "NavigationSystem",
                "NetCore",
                "Niagara",
                "Paper2D",
                "PhysicsCore",
                "PropertyPath",
                "ReplicationGraph",
                "SignificanceManager",
                "Slate",
                "SlateCore",
                "SignificanceManager",
                "SoundFieldRendering", // Linux needs a symbol that it cannot find so we try to link this library by force.
                "UIExtension",
                "UMG"
            }
        );

        PrivateDependencyModuleNames.AddRange
        (
            new []
            {
                //AudioModulation
                "AudioMixer",
                "AudioModulation",
                "ClientPilot",
                "CommonGame",
                "CommonInput",
                "CommonUI",
                "CommonUser",
                "DeveloperSettings",
                "DTLSHandlerComponent",
                "EngineSettings",
                "EnhancedInput",
                "GameFeatures",
                "GameplayMessageRuntime",
                "GameSettings",
                "GameSubtitles",
                "Json",
                "ModularGameplay",
                "ModularGameplayActors",
                "NetCore",
                "NetworkReplayStreaming",
                "Projects",
                "RenderCore",
                "RHI",
                "Slate",
                "SlateCore",
                "UIExtension",
                "UMG"
            }
        );

		if (Target.bBuildEditor)
		{
            PublicDependencyModuleNames.AddRange(
				new []
				{
					"UnrealEd",
					"DataValidation"
				}
			);
		}

        SetupGameplayDebuggerSupport(Target);
        SetupIrisSupport(Target);

        // Generate compile errors if using DrawDebug functions in test/shipping builds.
        PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");
    }
}
