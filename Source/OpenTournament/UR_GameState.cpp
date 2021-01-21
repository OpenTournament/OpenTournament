// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameState.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "UR_GameMode.h"
#include "UR_TeamInfo.h"
#include "UR_LocalPlayer.h"
#include "UR_MessageHistory.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameState::AUR_GameState()
{
    MultiKillEventNames = {
        FName(TEXT("Multi1")),  /* double */
        FName(TEXT("Multi2")),  /* triple */
        FName(TEXT("Multi3")),  /* mega */
        FName(TEXT("Multi4")),  /* ultra */
        FName(TEXT("Multi5")),  /* monster*/
    };
    SpreeEventNames = {
        FName(TEXT("Spree1")),  /* spree */
        FName(TEXT("Spree2")),  /* rampage */
        FName(TEXT("Spree3")),  /* unstoppable */
        FName(TEXT("Spree4")),  /* godlike */
    };
}

void AUR_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AUR_GameState, TimeLimit);
    DOREPLIFETIME(AUR_GameState, ClockReferencePoint);
    DOREPLIFETIME(AUR_GameState, MatchSubState);
    DOREPLIFETIME(AUR_GameState, Winner);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameState::SetMatchSubState(FName NewState)
{
    if (HasAuthority())
    {
        UE_LOG(LogGameState, Log, TEXT("Match Sub-State changing from %s to %s"), *MatchSubState.ToString(), *NewState.ToString());
        if (NewState == MatchSubState)
        {
            MulticastMatchSubState(NewState);
        }
        else
        {
            MatchSubState = NewState;
            OnRep_MatchSubState();
        }
    }
}

void AUR_GameState::MulticastMatchSubState_Implementation(FName NewState)
{
    MatchSubState = NewState;
    OnRep_MatchSubState();
}

void AUR_GameState::OnRep_MatchState()
{
    OnMatchStateChanged.Broadcast(this);
    
    Super::OnRep_MatchState();
}

void AUR_GameState::OnRep_MatchSubState()
{
    OnMatchSubStateChanged.Broadcast(this);
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

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameState::MulticastPickupEvent(AUR_Pickup* Pickup, AUR_PlayerState* Recipient)
{
    if (IsNetMode(NM_DedicatedServer))
    {
        PickupEvent.Broadcast(Pickup, Recipient);
    }

    for (auto PCIt = GetWorld()->GetPlayerControllerIterator(); PCIt; ++PCIt)
    {
        if (auto PC = Cast<AUR_PlayerController>(PCIt->Get()))
        {
            if (PC->PlayerState && (PC->PlayerState == Recipient || PC->PlayerState->IsOnlyASpectator()))
            {
                PC->ClientReceivePickupEvent(Pickup, Recipient);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameState::OnRep_Winner()
{
    OnWinnerAssigned.Broadcast(this);
}
