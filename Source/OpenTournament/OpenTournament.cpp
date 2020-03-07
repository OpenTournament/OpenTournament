// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "OpenTournament.h"
#include "Modules/ModuleManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, OpenTournament, "OpenTournament" );

/////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(Game);
DEFINE_LOG_CATEGORY(GameUI);
DEFINE_LOG_CATEGORY(Net);
DEFINE_LOG_CATEGORY(Weapon);

/////////////////////////////////////////////////////////////////////////////////////////////////

FCollisionResponseParams WorldResponseParams = []()
{
    FCollisionResponseParams Response(ECR_Ignore);

    Response.CollisionResponse.WorldStatic = ECR_Block;
    Response.CollisionResponse.WorldDynamic = ECR_Block;

    return Response;
}();