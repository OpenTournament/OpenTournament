// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TeamInfo.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "UR_GameState.h"
#include "UR_PlayerState.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_TeamInfo::AUR_TeamInfo()
{
    // from PlayerState.cpp
    SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicatingMovement(false);
    NetUpdateFrequency = 1;
    bNetLoadOnClient = false;
}

void AUR_TeamInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ThisClass, TeamIndex, COND_InitialOnly);

    FDoRepLifetimeParams Params;
    Params.bIsPushBased = true;
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Score, Params);
}

void AUR_TeamInfo::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority() && TeamIndex >= 0)
    {
        if (AUR_GameState* GS = GetWorld()->GetGameState<AUR_GameState>())
        {
            // Add self to GameState Teams array
            GS->Teams.SetNumZeroed(FMath::Max(TeamIndex + 1, GS->Teams.Num()));
            GS->Teams[TeamIndex] = this;

            // Catch up on clients who received TeamIndex before the TeamInfo actor
            for (APlayerState* PS : GS->PlayerArray)
            {
                if (AUR_PlayerState* URPS = Cast<AUR_PlayerState>(PS))
                {
                    if (URPS->ReplicatedTeamIndex == TeamIndex && URPS->TeamIndex != TeamIndex)
                    {
                        IUR_TeamInterface::Execute_SetTeamIndex(URPS, TeamIndex);
                    }
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

//NOTE: These methods are just alternative entry points for manipulating a player's team
// The full logic lies in URPlayerState->SetTeamIndex
// The Players array is manipulated directly by URPlayerState
// The events are triggered by URPlayerState as well
// This is necessary to ensure consistency in ordering of events, no matter which entry point we use.

void AUR_TeamInfo::AddPlayer(AUR_PlayerState* Player)
{
    if (Player)
    {
        IUR_TeamInterface::Execute_SetTeamIndex(Player, TeamIndex);
    }
}

void AUR_TeamInfo::RemovePlayer(AUR_PlayerState* Player)
{
    if (Player)
    {
        IUR_TeamInterface::Execute_SetTeamIndex(Player, -1);
    }
}


void AUR_TeamInfo::AddScore(const int32 Value)
{
    Score += Value;
    //MARK_PROPERTY_DIRTY_FROM_NAME(AUR_TeamInfo, Score, this);
    ForceNetUpdate();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int32 AUR_TeamInfo::GetTeamIndex_Implementation()
{
    return TeamIndex;
}

void AUR_TeamInfo::SetTeamIndex_Implementation(int32 NewTeamIndex)
{
    TeamIndex = NewTeamIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_TeamInfo* AUR_TeamInfo::GetTeamFromIndex(const UObject* WorldContextObject, int32 InTeamIndex)
{
    if (InTeamIndex >= 0)
    {
        if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
        {
            if (AUR_GameState* GameState = World->GetGameState<AUR_GameState>())
            {
                if (GameState->Teams.IsValidIndex(InTeamIndex))
                {
                    return GameState->Teams[InTeamIndex];
                }
            }
        }
    }
    return nullptr;
}
