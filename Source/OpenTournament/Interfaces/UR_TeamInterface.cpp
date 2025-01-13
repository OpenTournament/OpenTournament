// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TeamInterface.h"
#include "Engine/World.h"
#include "UR_TeamInfo.h"
#include "UR_GameState.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

int32 IUR_TeamInterface::GetTeamIndex_Implementation()
{
    UE_LOG(LogTemp, Error, TEXT("GetTeamIndex() not implemented in class %s"), *Cast<UObject>(this)->StaticClass()->GetName());
    return -1;
}

void IUR_TeamInterface::SetTeamIndex_Implementation(int32 NewTeamIndex)
{
    UE_LOG(LogTemp, Error, TEXT("SetTeamIndex() not implemented in class %s"), *Cast<UObject>(this)->StaticClass()->GetName());
}

AUR_TeamInfo* IUR_TeamInterface::GetTeam_Implementation()
{
    UObject* Obj = Cast<UObject>(this);
    int32 TeamIndex = Execute_GetTeamIndex(Obj);
    return AUR_TeamInfo::GetTeamFromIndex(Obj, TeamIndex);
}

void IUR_TeamInterface::SetTeam_Implementation(AUR_TeamInfo* NewTeam)
{
    Execute_SetTeamIndex(Cast<UObject>(this), NewTeam ? NewTeam->GetTeamIndex() : -1);
}

bool IUR_TeamInterface::IsAlly_Implementation(UObject* Other)
{
    if (Other)
    {
        int32 MyTeamIndex = Execute_GetTeamIndex(Cast<UObject>(this));
        if (MyTeamIndex >= 0 && Other->Implements<UUR_TeamInterface>())
        {
            return Execute_GetTeamIndex(Other) == MyTeamIndex;
        }
    }
    return false;
}

bool IUR_TeamInterface::IsEnemy_Implementation(UObject* Other)
{
    return !Execute_IsAlly(Cast<UObject>(this), Other);
}
