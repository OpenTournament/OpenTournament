// Copyright (c) 2019 Open Tournament Project, All Rights Reserved.

#include "UR_GameMode.h"

#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/LocalMessage.h"
#include "EngineUtils.h"    // for TActorIterator<>

#include "UR_PlayerState.h"
#include "UR_GameState.h"
#include "UR_LocalMessage.h"
#include "UR_Projectile.h"
#include "UR_PlayerController.h"
#include "UR_Character.h"
#include "UR_InventoryComponent.h"

AUR_GameMode::AUR_GameMode()
{
    DeathMessageClass = UUR_LocalMessage::StaticClass();

    GoalScore = 10;
    TimeLimit = 300;
    OvertimeExtraTime = 120;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Match
/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    AUR_GameState* GS = GetGameState<AUR_GameState>();
    if (GS)
    {
        if (TimeLimit > 0)
        {
            GS->SetTimeLimit(TimeLimit);
            GS->OnTimeUp.AddUniqueDynamic(this, &AUR_GameMode::OnMatchTimeUp);
        }
        else
        {
            GS->ResetClock();
        }
    }
}

void AUR_GameMode::OnMatchTimeUp_Implementation(AUR_GameState* GS)
{
    // Unbind match delegate
    GS->OnTimeUp.RemoveDynamic(this, &AUR_GameMode::OnMatchTimeUp);

    if (!CheckEndGame(nullptr))
    {
        // Overtime

        //TODO: msg class
        BroadcastLocalized(this, UUR_LocalMessage::StaticClass(), 0, nullptr, nullptr, GS);

        if (OvertimeExtraTime > 0)
        {
            GS->SetTimeLimit(OvertimeExtraTime);
            GS->OnTimeUp.AddUniqueDynamic(this, &AUR_GameMode::OnMatchTimeUp);
        }
        else
        {
            // Start counting up
            GS->ResetClock();
        }
    }
}

void AUR_GameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
    if (AUR_Character* Char = Cast<AUR_Character>(PlayerPawn))
    {
        //NOTE: Technically RestartPlayer() supports restarting a player that is not dead.
        // In that case, the existing Pawn is not touched, but this method is still called.
        // I'm not sure if we should try to support that.
        if (Char->InventoryComponent)
        {
            Char->InventoryComponent->Clear();

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            SpawnParams.Owner = Char;
            SpawnParams.Instigator = Char;
            for (const FStartingWeaponEntry& Entry : StartingWeapons)
            {
                AUR_Weapon* Weap = GetWorld()->SpawnActor<AUR_Weapon>(Entry.WeaponClass, Char->GetActorLocation(), Char->GetActorRotation(), SpawnParams);
                if (Weap)
                {
                    Weap->ammoCount = Entry.Ammo;
                    Weap->GiveTo(Char);
                }
            }
        }
    }
    Super::SetPlayerDefaults(PlayerPawn);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Killing
/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_GameMode::PreventDeath_Implementation(AController* Killed, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser)
{
    return false;
}

/**
* Things to do and to see how to structure the whole thing :
*
* - kills, deaths, suicides, message   => the message could be sent from PS->AddKill / AddSuicide ?
*
* - multi kills, counting & message    => can do in PS->AddKill(Victim)
* - sprees, counting & message         => can do in PS->AddKill(Victim)
* - ending someone's spree             => can do in PS->AddDeath(Killer)
* - first blood ?
* - revenge ?                          => can do in PS->AddKill(Victim)
*
* - stats for every single type of damage
*
* - update score based on kill ? this is heavily gamemode-dependent
* - check end game ? again gamemode-dependent.
* 
* In case of DM, Score = Kills-Suicides.
* In case of CTF, Score = Caps.
* Also need to update TeamScores somehow.
* This could all fit in PS->AddKill / AddSuicide,
* But ideally we also want to avoid having extra PS subclasses whenever possible.
*
* We have to be careful as Blueprints cannot choose specifically which Super() method to call.
*/

void AUR_GameMode::PlayerKilled_Implementation(AController* Victim, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser)
{
    RegisterKill(Victim, Killer, DamageEvent, DamageCauser);
}

void AUR_GameMode::RegisterKill(AController* Victim, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser)
{
    if (Victim)
    {
        AUR_PlayerState* VictimPS = Victim->GetPlayerState<AUR_PlayerState>();

        if (VictimPS)
        {
            VictimPS->AddDeath(Killer);
        }

        if (Killer && Killer != Victim)
        {
            AUR_PlayerState* KillerPS = Killer->GetPlayerState<AUR_PlayerState>();
            if (KillerPS)
            {
                KillerPS->AddKill(Victim);
            }
            BroadcastLocalized(Killer, DeathMessageClass, 0, Victim->GetPlayerState<APlayerState>(), Killer->GetPlayerState<APlayerState>(), DamageEvent.DamageTypeClass);
        }
        else
        {
            if (VictimPS)
            {
                VictimPS->AddSuicide();
            }
            BroadcastLocalized(Killer, DeathMessageClass, 1, Victim->GetPlayerState<APlayerState>(), nullptr, DamageEvent.DamageTypeClass);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// End Game
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Trying to stick with the engine game framework provided.
* However I don't like how they use Tick to check ReadyToEndMatch().
* I prefer event-based here, calling CheckEndGame whenever something happens.
*/

bool AUR_GameMode::CheckEndGame(AActor* Focus)
{
    AActor* Winner = IsThereAWinner();
    if (Winner)
    {
        if (Focus)
        {
            // Passed in focus may be a controller or a PS. Try to resolve to a pawn.
            Focus = ResolveEndGameFocus(Focus);
        }
        else
        {
            // Winner may be a controller, PS, or even a team. Try to resolve to a pawn.
            Focus = ResolveEndGameFocus(Winner);
        }

        TriggerEndMatch(Winner, Focus);
        return true;
    }
    return false;
}

AActor* AUR_GameMode::IsThereAWinner_Implementation()
{
    // By default return the highest scorer..?
    AGameState* GS = GetGameState<AGameState>();
    if (GS)
    {
        APlayerState* BestPlayer = nullptr;
        bool bTie = false;
        for (APlayerState* PS : GS->PlayerArray)
        {
            if (!BestPlayer || PS->Score > BestPlayer->Score)
            {
                BestPlayer = PS;
                bTie = false;
            }
            else if (PS->Score == BestPlayer->Score)
            {
                bTie = true;
            }
        }
        if (!bTie)
        {
            return BestPlayer;
        }
    }

    //NOTE: maybe we should put the DM implem here because it kind of makes sense.
    // GoalScore is part of this class so we should have a default handling here.
    // Also we'll want the teams implementation all merged in.

    return nullptr;
}

AActor* AUR_GameMode::ResolveEndGameFocus_Implementation(AActor* Winner)
{
    if (APlayerState* PS = Cast<APlayerState>(Winner))
    {
        return PS->GetPawn();
    }
    if (AController* C = Cast<AController>(Winner))
    {
        return C->GetPawn();
    }

    //TODO: if Winner is a team, return best player of team

    return Winner;
}

void AUR_GameMode::TriggerEndMatch_Implementation(AActor* Winner, AActor* Focus)
{
    if (AUR_GameState* GS = GetGameState<AUR_GameState>())
    {
        GS->Winner = Winner;
        GS->EndGameFocus = Focus;
    }
    SetMatchState(MatchState::WaitingPostMatch);
}

void AUR_GameMode::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();

    // Freeze the game
    //TODO: this is probably the wrong way to do it
    for (TActorIterator<AActor> ActorIt(GetWorld()); ActorIt; ++ActorIt)
    {
        AActor* Actor = *ActorIt;
        if (Cast<APawn>(Actor) || Cast<AUR_Projectile>(Actor))
        {
            Actor->CustomTimeDilation = 0.01f;
        }
    }

    AUR_GameState* GS = GetGameState<AUR_GameState>();

    // Set focus - if null, everyone will continue viewing their current view target.
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        if (APlayerController* PC = Iterator->Get())
        {
            AActor* VT = (GS && GS->EndGameFocus) ? GS->EndGameFocus : PC->GetViewTarget();
            PC->UnPossess();
            PC->SetViewTarget(VT);
        }
    }

    AnnounceWinner(GS ? GS->Winner : nullptr);

    if (GS)
    {
        GS->OnTimeUp.RemoveDynamic(this, &AUR_GameMode::OnMatchTimeUp);
        //NOTE: This system is a bit dangerous, we might change state completely and forget some TimeUp handlers.
        // It might be a good thing to clear all listeners whenever GS->SetTimeLimit is called.
        // Or something like that.
        GS->SetTimeLimit(10);
        GS->OnTimeUp.AddUniqueDynamic(this, &AUR_GameMode::OnEndGameTimeUp);
    }
}

void AUR_GameMode::AnnounceWinner_Implementation(AActor* Winner)
{
    APlayerState* WinnerPS = nullptr;
    if (APlayerState* PS = Cast<APlayerState>(Winner))
    {
        WinnerPS = PS;
    }
    else if (APawn* P = Cast<APawn>(Winner))
    {
        WinnerPS = P->GetPlayerState();
    }
    else if (AController* C = Cast<AController>(Winner))
    {
        WinnerPS = C->GetPlayerState<APlayerState>();
    }

    //TODO: msg class
    BroadcastLocalized(Winner, UUR_LocalMessage::StaticClass(), 0, WinnerPS, nullptr, Winner);
}

void AUR_GameMode::OnEndGameTimeUp(AUR_GameState* GS)
{
    if (GetNetMode() != NM_DedicatedServer)
    {
        AUR_PlayerController* LocalPC = GetWorld()->GetFirstPlayerController<AUR_PlayerController>();
        if (LocalPC)
        {
            LocalPC->ReturnToMainMenu();
            return;
        }
    }

    if (GetWorld()->IsPlayInEditor())
    {
        // Cannot RestartGame in PIE dedicated server for some reason.
        // Dunno what to do, stay stuck in endgame forever.
        return;
    }

    // else
    RestartGame();
}
