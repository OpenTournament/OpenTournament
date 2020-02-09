// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameState.h"

#include "UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "UR_GameMode.h"

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
