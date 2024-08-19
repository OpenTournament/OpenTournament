// Copyright (c) Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;

public class OpenTournamentServerTarget : TargetRules
{
    public OpenTournamentServerTarget(TargetInfo Target) : base(Target)
    {
	    IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        Type = TargetType.Server;
        LinkType = TargetLinkType.Modular;
        ExtraModuleNames.Add("OpenTournament");
    }
}
