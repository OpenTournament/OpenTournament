// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;

public class OpenTournamentEditorTarget : TargetRules
{
    public OpenTournamentEditorTarget(TargetInfo Target) : base(Target)
    {
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        Type = TargetType.Editor;
        LinkType = TargetLinkType.Modular;
        ExtraModuleNames.Add("OpenTournament");
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
    }
}
