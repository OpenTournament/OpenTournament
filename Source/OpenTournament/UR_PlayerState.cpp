// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerState.h"

#include "UnrealNetwork.h"
#include "Engine/World.h"

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

void AUR_PlayerState::AddScore(int32 Value)
{
    Score += Value;
    ForceNetUpdate();
}
