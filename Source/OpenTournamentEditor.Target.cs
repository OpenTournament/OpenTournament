// Copyright (c) Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OpenTournamentEditorTarget : TargetRules
{
    public OpenTournamentEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;

        ExtraModuleNames.AddRange(new string[] { "OpenTournament", "OpenTournamentEditor" });

        OpenTournamentTarget.ApplySharedOpenTournamentTargetSettings(this);
    }
}
