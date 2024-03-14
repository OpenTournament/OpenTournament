// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "OpenTournament.h"
#include "Modules/ModuleManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, OpenTournament, "OpenTournament" );

/////////////////////////////////////////////////////////////////////////////////////////////////

FCollisionResponseParams WorldResponseParams = []()
{
    FCollisionResponseParams Response(ECR_Ignore);

    Response.CollisionResponse.WorldStatic = ECR_Block;
    Response.CollisionResponse.WorldDynamic = ECR_Block;

    return Response;
}();
