// Copyright (c) 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameMode_DM.h"

#include "UR_PlayerState.h"
#include "UR_GameState.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameMode_DM::PlayerKilled_Implementation(AController* Victim, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser)
{
    RegisterKill(Victim, Killer, DamageEvent, DamageCauser);
    ScoreKill(Victim, Killer, DamageEvent, DamageCauser);
}

void AUR_GameMode_DM::ScoreKill_Implementation(AController* Victim, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser)
{
    // This may look a little bit redundant compared to RegisterKill, but here's the thing,
    // RegisterKill is the generic handling for stats & messages/announcements,
    // whereas this handles scoring which is heavily gamemode-dependent.

    // This could be completely different in a gimmick gamemode like Mutant for example,
    // while the core (RegisterKill) remains always the same.

    if (Victim)
    {
        if (!Killer)
        {
            Killer = Victim;
        }

        AUR_PlayerState* KillerPS = Killer->GetPlayerState<AUR_PlayerState>();
        if (!KillerPS)
            return;

        if (Killer == Victim)
        {
            KillerPS->AddScore(-1);
            CheckEndGame(nullptr);
        }
        else
        {
            KillerPS->AddScore(1);
            CheckEndGame(Killer);
        }
    }
}

AActor* AUR_GameMode_DM::IsThereAWinner_Implementation()
{
    AUR_GameState* GS = GetGameState<AUR_GameState>();

    // If we are out of time / sudden death overtime, we can use "highest score" default implem.
    if (GS && GS->RemainingTime == 0)
    {
        return Super::IsThereAWinner_Implementation();
    }

    // Else, check with goal score
    for (APlayerState* PS : GS->PlayerArray)
    {
        if (PS->Score >= GoalScore)
        {
            return PS;
        }
    }

    return nullptr;
}
