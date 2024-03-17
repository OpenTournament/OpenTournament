// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "OpenTournament.h"
#include "Modules/ModuleManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class FOpenTournamentGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FOpenTournamentGameModule, OpenTournament, "OpenTournament" );

/////////////////////////////////////////////////////////////////////////////////////////////////

FCollisionResponseParams WorldResponseParams = []()
{
    FCollisionResponseParams Response(ECR_Ignore);

    Response.CollisionResponse.WorldStatic = ECR_Block;
    Response.CollisionResponse.WorldDynamic = ECR_Block;

    return Response;
}();
