// Copyright 2019 Open Tournament Project, All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OpenTournamentTarget : TargetRules
{
	public OpenTournamentTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("OpenTournament");
	}
}
