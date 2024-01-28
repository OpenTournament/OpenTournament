// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;

public class OpenTournamentClientTarget : TargetRules
{
    public OpenTournamentClientTarget(TargetInfo Target) : base(Target)
    {
	    IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        Type = TargetType.Client;
        LinkType = TargetLinkType.Modular;
        ExtraModuleNames.Add("OpenTournament");
    }
}
