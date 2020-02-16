// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OpenTournamentTarget : TargetRules
{
    public OpenTournamentTarget(TargetInfo Target) : base(Target)
    {
        DefaultBuildSettings = BuildSettingsVersion.V2;
        bEnforceIWYU = true;
        ShadowVariableWarningLevel = WarningLevel.Error;

        Type = TargetType.Game;
        ExtraModuleNames.Add("OpenTournament");
    }
}
