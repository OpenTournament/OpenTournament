// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameState.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerState.h"

#include "UR_GameMode.h"
#include "UR_TeamInfo.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AUR_GameState, TimeLimit);
    DOREPLIFETIME(AUR_GameState, ClockReferencePoint);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameState::OnRep_MatchState()
{
    OnMatchStateChanged.Broadcast(this);
    
    Super::OnRep_MatchState();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameState::DefaultTimer()
{
    Super::DefaultTimer();

    //NOTE: Super only counts elapsed time during match in progress.
    // We want to count during other states as well, so we can benefit from clock in warmup and endgame.
    if ( !IsMatchInProgress() )
    {
        ++ElapsedTime;
        if (GetNetMode() != NM_DedicatedServer)
        {
            OnRep_ElapsedTime();
        }
    }

    // Clock sync
    if (HasAuthority() && (ElapsedTime % 30) == 0)
    {
        MulticastElapsedTime(ElapsedTime);
    }

    // Remaining time
    RemainingTime = FMath::Max(0, ClockReferencePoint + TimeLimit - ElapsedTime);

    // TimeUp delegate
    if (RemainingTime == 0 && !bTriggeredTimeUp)
    {
        bTriggeredTimeUp = true;    //set this before as a delegate might reset with SetTimeLimit()
        OnTimeUp.Broadcast(this);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_TeamInfo* AUR_GameState::AddNewTeam()
{
    TSubclassOf<AUR_TeamInfo> TeamInfoClass;
    if (AUR_GameMode* GM = GetWorld()->GetAuthGameMode<AUR_GameMode>())
    {
        TeamInfoClass = GM->TeamInfoClass;
    }
    if (!TeamInfoClass)
    {
        TeamInfoClass = AUR_TeamInfo::StaticClass();
    }

    FActorSpawnParameters SpawnInfo;
    SpawnInfo.Instigator = GetInstigator();
    SpawnInfo.ObjectFlags |= RF_Transient;

    AUR_TeamInfo* TeamInfo = GetWorld()->SpawnActor<AUR_TeamInfo>(TeamInfoClass, SpawnInfo);
    if (TeamInfo)
    {
        IUR_TeamInterface::Execute_SetTeamIndex(TeamInfo, Teams.Num());
        Teams.Add(TeamInfo);
    }

    return TeamInfo;
}

void AUR_GameState::TrimTeams()
{
    if (HasAuthority())
    {
        // Iterate from last index to 2
        for (int32 i = Teams.Num() - 1; i >= 2; i--)
        {
            if (Teams[i]->Players.Num() > 0 || Teams[i]->GetScore() != 0)
            {
                break;
            }
            Teams[i]->Destroy();
            Teams.RemoveAt(i);
        }
    }
}

void AUR_GameState::GetSpectators(TArray<APlayerState*>& OutSpectators)
{
    for (APlayerState* PS : PlayerArray)
    {
        if (PS && PS->IsOnlyASpectator())
        {
            OutSpectators.Add(PS);
        }
    }
}
