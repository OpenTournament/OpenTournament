// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerState.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

#include "UR_TeamInfo.h"
#include "UR_GameState.h"
#include "UR_MPC_Global.h"
#include "UR_FunctionLibrary.h"
#include "UR_Character.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_PlayerState::AUR_PlayerState()
{
    TeamIndex = -1;
    ReplicatedTeamIndex = -1;

    OnTeamChanged.AddUniqueDynamic(this, &AUR_PlayerState::InternalOnTeamChanged);
}

void AUR_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams Params;
    Params.bIsPushBased = true;

    DOREPLIFETIME_WITH_PARAMS_FAST(AUR_PlayerState, Kills, Params);
    DOREPLIFETIME_WITH_PARAMS_FAST(AUR_PlayerState, Deaths, Params);
    DOREPLIFETIME_WITH_PARAMS_FAST(AUR_PlayerState, Suicides, Params);

    Params.RepNotifyCondition = REPNOTIFY_OnChanged;
    DOREPLIFETIME_WITH_PARAMS_FAST(AUR_PlayerState, ReplicatedTeamIndex, Params);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerState::AddKill(AController* Victim)
{
    Kills++;
    MARK_PROPERTY_DIRTY_FROM_NAME(AUR_PlayerState, Kills, this);

    //TODO: count multi kills here
    //TODO: count sprees here
    //NOTE: can do "revenge" here
}

void AUR_PlayerState::AddDeath(AController* Killer)
{
    Deaths++;
    MARK_PROPERTY_DIRTY_FROM_NAME(AUR_PlayerState, Deaths, this);

    //TODO: spree ended by killer here
}

void AUR_PlayerState::AddSuicide()
{
    Suicides++;
    MARK_PROPERTY_DIRTY_FROM_NAME(AUR_PlayerState, Suicides, this);
}

void AUR_PlayerState::AddScore(const int32 Value)
{
    SetScore(GetScore() + Value);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int32 AUR_PlayerState::GetTeamIndex_Implementation()
{
    return TeamIndex;
}

void AUR_PlayerState::SetTeamIndex_Implementation(int32 NewTeamIndex)
{
    if (NewTeamIndex != TeamIndex)
    {
        int32 OldTeamIndex = TeamIndex;
        AUR_TeamInfo* OldTeam = Team;
        if (OldTeam)
        {
            if (OldTeam->Players.Remove(this) > 0)
            {
                // team left event
                OldTeam->OnPlayerLeftTeam.Broadcast(OldTeam, this);
            }
        }

        TeamIndex = NewTeamIndex;

        if (HasAuthority())
        {
            ReplicatedTeamIndex = TeamIndex;
            MARK_PROPERTY_DIRTY_FROM_NAME(AUR_PlayerState, ReplicatedTeamIndex, this);
        }

        Team = AUR_TeamInfo::GetTeamFromIndex(this, TeamIndex);

        if (Team)
        {
            if (Team->Players.AddUnique(this) >= 0)
            {
                // team joined event
                Team->OnPlayerJoinedTeam.Broadcast(Team, this);
            }
        }

        if (AUR_GameState* GS = GetWorld()->GetGameState<AUR_GameState>())
        {
            GS->TrimTeams();
        }

        // team changed event
        OnTeamChanged.Broadcast(this, OldTeamIndex, TeamIndex);
    }
}

void AUR_PlayerState::OnRep_ReplicatedTeamIndex()
{
    if (!HasAuthority() && ReplicatedTeamIndex != TeamIndex)
    {
        //WARNING: it is very possible to receive ReplicatedTeamIndex before receiving the corresponding TeamInfo actor
        if (ReplicatedTeamIndex >= 0 && !AUR_TeamInfo::GetTeamFromIndex(this, ReplicatedTeamIndex))
        {
            return; // We will catch up from UR_TeamInfo::BeginPlay
        }

        // Maintains all the Team->Players arrays on client-side & trigger events appropriately.
        IUR_TeamInterface::Execute_SetTeamIndex(this, ReplicatedTeamIndex);
    }
}

void AUR_PlayerState::InternalOnTeamChanged(AUR_PlayerState* PS, int32 OldTeamIndex, int32 NewTeamIndex)
{
    // When local player changes team, update MPC's TeamColor# mappings accordingly
    if (UUR_FunctionLibrary::IsLocallyControlled(PS))
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("InternalOnTeamChanged: %i -> %i"), OldTeamIndex, NewTeamIndex));

        const auto Params = GetDefault<UUR_MPC_Global>();
        TArray<FName> ParamNames = { Params->P_TeamColor0, Params->P_TeamColor1, Params->P_TeamColor2, Params->P_TeamColor3 };
        if (NewTeamIndex < 0 || NewTeamIndex > 3)
        {
            // if we have no team or an unhandled team number, try to prioritize the standard,
            // aka. Enemy on team0 and Enemy2 on team1. Ally ends up on team3 which is wrong though.
            FName Last = ParamNames.Pop(false);
            ParamNames.Insert(Last, 0);
        }
        if (NewTeamIndex >= 1)
        {
            FName MyTeamParam = ParamNames[NewTeamIndex];
            ParamNames.RemoveAt(NewTeamIndex);
            ParamNames.Insert(MyTeamParam, 0);
        }
        /*
        UUR_MPC_Global::SetVector(this, ParamNames[0], UUR_MPC_Global::GetVector(this, Params->P_AllyColor));
        UUR_MPC_Global::SetVector(this, ParamNames[1], UUR_MPC_Global::GetVector(this, Params->P_EnemyColor));
        UUR_MPC_Global::SetVector(this, ParamNames[2], UUR_MPC_Global::GetVector(this, Params->P_EnemyColor2));
        UUR_MPC_Global::SetVector(this, ParamNames[3], UUR_MPC_Global::GetVector(this, Params->P_EnemyColor3));
        */
        // Parameters mapping for the win
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("MPC: Map %s to AllyColor"), *ParamNames[0].ToString()), false);
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("MPC: Map %s to EnemyColor"), *ParamNames[1].ToString()), false);
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("MPC: Map %s to EnemyColor2"), *ParamNames[2].ToString()), false);
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("MPC: Map %s to EnemyColor3"), *ParamNames[3].ToString()), false);
        UUR_MPC_Global::MapParameter(this, ParamNames[0], Params->P_AllyColor);
        UUR_MPC_Global::MapParameter(this, ParamNames[1], Params->P_EnemyColor);
        UUR_MPC_Global::MapParameter(this, ParamNames[2], Params->P_EnemyColor2);
        UUR_MPC_Global::MapParameter(this, ParamNames[3], Params->P_EnemyColor3);
    }

    if (auto Char = GetPawn<AUR_Character>())
    {
        Char->UpdateTeamColor();
    }
}

FLinearColor AUR_PlayerState::GetColor()
{
    if (IsOnlyASpectator())
    {
        return UUR_FunctionLibrary::GetSpectatorDisplayTextColor();
    }

    const auto Params = GetDefault<UUR_MPC_Global>();
    if (TeamIndex >= 0 && TeamIndex <= 3)
    {
        FName ParamName = TArray<FName>({ Params->P_TeamColor0, Params->P_TeamColor1, Params->P_TeamColor2, Params->P_TeamColor3 })[TeamIndex];
        return UUR_MPC_Global::GetVector(this, ParamName);
    }
    else
    {
        const auto PC = UUR_FunctionLibrary::GetLocalPlayerController(this);
        if (IUR_TeamInterface::Execute_IsAlly(this, (UObject*)PC))
        {
            return UUR_MPC_Global::GetVector(this, Params->P_AllyColor);
        }
    }
    return UUR_MPC_Global::GetVector(this, Params->P_EnemyColor);
}
