// Copyright (c) Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;

public class OpenTournamentTarget : TargetRules
{
    public OpenTournamentTarget(TargetInfo Target) : base(Target)
    {
	    IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        Type = TargetType.Game;
        LinkType = TargetLinkType.Modular;
        ExtraModuleNames.Add("OpenTournament");
    }
}
