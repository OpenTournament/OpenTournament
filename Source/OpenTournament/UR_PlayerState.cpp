// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerState.h"

#include "Net/UnrealNetwork.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_PlayerState::AUR_PlayerState()
{
}

void AUR_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AUR_PlayerState, Kills);
    DOREPLIFETIME(AUR_PlayerState, Deaths);
    DOREPLIFETIME(AUR_PlayerState, Suicides);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerState::AddKill(AController* Victim)
{
    Kills++;

    //TODO: count multi kills here
    //TODO: count sprees here
    //NOTE: can do "revenge" here
}

void AUR_PlayerState::AddDeath(AController* Killer)
{
    Deaths++;

    //TODO: spree ended by killer here
}

void AUR_PlayerState::AddSuicide()
{
    Suicides++;
}

void AUR_PlayerState::AddScore(const int32 Value)
{
    SetScore(GetScore() + Value);
    ForceNetUpdate();
}
