// Copyright (c) Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;

public class OpenTournament : ModuleRules
{
    public OpenTournament(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        ShadowVariableWarningLevel = WarningLevel.Error;
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
                "OpenTournament/Camera",
                "OpenTournament/Character",
                "OpenTournament/Data",
                "OpenTournament/Enums",
                "OpenTournament/GAS",
                "OpenTournament/GameFeatures",
                "OpenTournament/GameModes",
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
            new []
            {
                "AIModule",
                "ApplicationCore",
                "CinematicCamera",
                "ControlFlows",
                "Core",
                "CoreOnline",
                "CoreUObject",
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
                "SoundFieldRendering",
                "UIExtension",
                "UMG"
            }
        );

        PrivateDependencyModuleNames.AddRange
        (
            new []
            {
                "AudioMixer",
                "AudioModulation",
                "ClientPilot",
                "CommonGame",
                "CommonInput",
                "CommonLoadingScreen",
                "CommonUI",
                "CommonUser",
                "DTLSHandlerComponent",
                "EnhancedInput",
                "GameFeatures",
                "GameSettings",
                "GameSubtitles",
                "GameplayMessageRuntime",
                "Gauntlet",
                "ModularGameplay",
                "ModularGameplayActors",
                "NetworkReplayStreaming",
                "Projects",
                "RHI",
                "RenderCore"
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
