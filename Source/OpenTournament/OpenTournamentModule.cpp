// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Modules/ModuleManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * FOpenTournamentModule
 */
class FOpenTournamentModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FOpenTournamentModule, OpenTournament, "OpenTournament");
