// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "OpenTournament.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

FCollisionResponseParams WorldResponseParams = []()
{
    FCollisionResponseParams Response(ECR_Ignore);

    Response.CollisionResponse.WorldStatic = ECR_Block;
    Response.CollisionResponse.WorldDynamic = ECR_Block;

    return Response;
}();
