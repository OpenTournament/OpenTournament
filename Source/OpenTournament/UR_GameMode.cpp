// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameMode.h"

#include "EngineUtils.h"    // for TActorIterator<>
#include "Engine/DamageEvents.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "UR_Character.h"
#include "UR_GameState.h"
#include "UR_InventoryComponent.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"
#include "UR_Projectile.h"
#include "UR_Weapon.h"
#include "UR_Ammo.h"
#include "UR_TeamInfo.h"

// Having to include these, only to set the default classes, makes me sad
#include "UR_Widget_ScoreboardBase.h"
#include "AI/UR_BotController.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization
/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameMode::AUR_GameMode()
{
    ScoreboardClass = UUR_Widget_ScoreboardBase::StaticClass();
    BotControllerClass = AUR_BotController::StaticClass();

    GoalScore = 10;
    TimeLimit = 300;
    OvertimeExtraTime = 120;

    MaxPlayers = 10;
    NumTeams = 0;
    TeamsFillMode = ETeamsFillMode::Even;

    SelfDamage = 1.f;
    TeamDamageDirect = 0.f;
    TeamDamageRetaliate = 0.f;
}

void AUR_GameMode::InitGame(const FString& MapName, const FString& OptionsMeh, FString& ErrorMessage)
{
    FString Options = OptionsMeh;

    if (GetWorld()->IsPlayInEditor())
    {
        // Custom additional game options seems broken in PIE - fix it!
        // We also intentionally include them in NM_Standalone where it's not supposed to be supported.
        auto Settings = FindObject<UObject>(nullptr, TEXT("/Script/UnrealEd.Default__LevelEditorPlaySettings"));
        FProperty* Prop = Settings->GetClass()->FindPropertyByName("AdditionalServerGameOptions");
        FString AdditionalOptions = *Prop->ContainerPtrToValuePtr<FString>(Settings);
        if (AdditionalOptions.Len())
        {
            UE_LOG(LogTemp, Log, TEXT("[PIE-Fix] Add URL options: %s"), *AdditionalOptions);
            Options.Append(AdditionalOptions);
        }
    }

    Super::InitGame(MapName, Options, ErrorMessage);

    GoalScore = UGameplayStatics::GetIntOption(Options, TEXT("GoalScore"), GoalScore);
    TimeLimit = UGameplayStatics::GetIntOption(Options, TEXT("TimeLimit"), TimeLimit);
    OvertimeExtraTime = UGameplayStatics::GetIntOption(Options, TEXT("OvertimeExtraTime"), OvertimeExtraTime);
    MaxPlayers = UGameplayStatics::GetIntOption(Options, TEXT("MaxPlayers"), MaxPlayers);
    BotFill = UGameplayStatics::GetIntOption(Options, TEXT("BotFill"), BotFill);
    NumTeams = UGameplayStatics::GetIntOption(Options, TEXT("NumTeams"), NumTeams);
    TeamsFillMode = UGameplayStatics::ParseOption(Options, TEXT("TeamsFillMode"));
    SelfDamage = UUR_FunctionLibrary::GetFloatOption(Options, TEXT("SelfDamage"), SelfDamage);
    TeamDamageDirect = UUR_FunctionLibrary::GetFloatOption(Options, TEXT("TeamDamageDirect"), TeamDamageDirect);
    TeamDamageRetaliate = UUR_FunctionLibrary::GetFloatOption(Options, TEXT("TeamDamageRetaliate"), TeamDamageRetaliate);

    if (NumTeams > 0)
    {
        DesiredTeamSize = FMath::CeilToInt((float)MaxPlayers / (float)NumTeams);
    }
    else
    {
        NumTeams = 0;
        DesiredTeamSize = 1;
    }
}

void AUR_GameMode::InitGameState()
{
    Super::InitGameState();

    if (AUR_GameState* GS = GetGameState<AUR_GameState>())
    {
        for (auto Team : GS->Teams)
        {
            Team->Destroy();
        }
        GS->Teams.Empty();

        // Only create 2 teams to start with, others will be added as they fill up.
        for (int32 i = 0; i < FMath::Min(NumTeams, 2); i++)
        {
            GS->AddNewTeam();
        }
    }
}

void AUR_GameMode::BroadcastSystemMessage(const FString& Msg)
{
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (It->IsValid())
            It->Get()->ClientMessage(Msg);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Players and bots flow
/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameMode::OnPostLogin(AController* NewPlayer)
{
    Super::OnPostLogin(NewPlayer);

    if (NewPlayer)
    {
        if (auto PS = NewPlayer->GetPlayerState<APlayerState>())
        {
            BroadcastSystemMessage(FString::Printf(TEXT("%s has joined."), *PS->GetPlayerName()));

            if (PS->IsABot())
            {
                NumBots++;
            }
        }

        if (NewPlayer->IsA<APlayerController>())
            CheckBotsDeferred();
    }
}

void AUR_GameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    if (Exiting)
    {
        if (auto PS = Exiting->GetPlayerState<APlayerState>())
        {
            BroadcastSystemMessage(FString::Printf(TEXT("%s has left."), *PS->GetPlayerName()));

            if (PS->IsABot())
            {
                NumBots--;
            }
        }

        if (Exiting->IsA<APlayerController>())
            CheckBotsDeferred();
    }
}

void AUR_GameMode::GenericPlayerInitialization(AController* C)
{
    Super::GenericPlayerInitialization(C);

    if (C && C->PlayerState && !C->PlayerState->IsOnlyASpectator())
    {
        if (AUR_PlayerState* PS = Cast<AUR_PlayerState>(C->PlayerState))
        {
            if (NumTeams > 0)
            {
                AssignDefaultTeam(PS);
            }
        }
    }
}

void AUR_GameMode::AssignDefaultTeam(AUR_PlayerState* PS)
{
    if (NumTeams == 0)
    {
        IUR_TeamInterface::Execute_SetTeamIndex(PS, -1);
        return;
    }

    if (NumTeams == 1)
    {
        IUR_TeamInterface::Execute_SetTeamIndex(PS, 0);
        return;
    }

    if (AUR_GameState* GS = GetGameState<AUR_GameState>())
    {
        bool bCreateNewTeam = false;

        if (TeamsFillMode.Equals(ETeamsFillMode::Squads))
        {
            // Create a new squad only if all existing squads are full
            bCreateNewTeam = true;
            for (const auto Team : GS->Teams)
            {
                if (Team->Players.Num() < DesiredTeamSize)
                {
                    bCreateNewTeam = false;
                    break;
                }
            }
        }
        else
        {
            // Create a new team if we don't have NumTeams yet
            if (GS->Teams.Num() < NumTeams)
            {
                bCreateNewTeam = true;
                // Unless one existing team has 0 players
                for (const auto Team : GS->Teams)
                {
                    if (Team->Players.Num() == 0)
                    {
                        bCreateNewTeam = false;
                        break;
                    }
                }
            }
        }

        if (bCreateNewTeam)
        {
            GS->AddNewTeam();
        }

        // Fit player into the smallest team

        AUR_TeamInfo* SmallestTeam = GS->Teams[0];
        for (const auto Team : GS->Teams)
        {
            if (Team->Players.Num() < SmallestTeam->Players.Num())
            {
                SmallestTeam = Team;
            }
        }

        SmallestTeam->AddPlayer(PS);
    }
}

void AUR_GameMode::CheckBotsDeferred()
{
    // Combine calls, avoid re-entrancy
    static FTimerHandle CheckBotsTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(CheckBotsTimerHandle, FTimerDelegate::CreateUObject(this, &AUR_GameMode::CheckBots), 0.1f, false);
}

void AUR_GameMode::CheckBots()
{
    if (!IsMatchInProgress())
        return;

    for (int32 i = NumPlayers + NumBots; i > BotFill; i--)
    {
        RemoveBot();
    }
    for (int32 i = NumPlayers + NumBots; i < BotFill; i++)
    {
        AddBot();
    }
    //TODO: Team balance
}

void AUR_GameMode::AddBot()
{
    if (auto BotController = GetWorld()->SpawnActor<AController>(BotControllerClass))
    {
        GenericPlayerInitialization(BotController);
        DispatchPostLogin(BotController);
        RestartPlayer(BotController);
    }
}

void AUR_GameMode::RemoveBot()
{
    if (auto GS = GetGameState<AGameStateBase>())
    {
        APlayerState* Best = nullptr;
        for (auto PS : GS->PlayerArray)
        {
            if (PS->IsABot() && PS->GetOwner<AController>() && (!Best || PS->CreationTime > Best->CreationTime))
                Best = PS;
        }
        if (Best)
        {
            Best->GetOwner()->Destroy();
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Match
/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameMode::HandleMatchHasStarted()
{
    // WARNING: Parent starts recording replay here. We don't want that when we have Warmup first.
    Super::HandleMatchHasStarted();

    AUR_GameState* GS = GetGameState<AUR_GameState>();
    if (GS)
    {
        // TODO: Here we would do Warmup first
        GS->SetMatchStateTag(FGameplayTag::RequestGameplayTag(FName(TEXT("MatchState.InProgress.Match"))));

        if (TimeLimit > 0)
        {
            GS->SetTimeLimit(TimeLimit);
            GS->OnTimeUp.AddUniqueDynamic(this, &ThisClass::OnMatchTimeUp);
        }
        else
        {
            GS->ResetClock();
        }
    }

    CheckBotsDeferred();
}

void AUR_GameMode::OnMatchTimeUp_Implementation(AUR_GameState* GS)
{
    // Unbind match delegate
    GS->OnTimeUp.RemoveDynamic(this, &ThisClass::OnMatchTimeUp);

    if (!CheckEndGame(nullptr))
    {
        // Overtime

        GS->SetMatchStateTag(FGameplayTag::RequestGameplayTag(FName(TEXT("MatchState.InProgress.Overtime"))));

        if (OvertimeExtraTime > 0)
        {
            GS->SetTimeLimit(OvertimeExtraTime);
            GS->OnTimeUp.AddUniqueDynamic(this, &ThisClass::OnMatchTimeUp);
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
    if (AUR_Character* URCharacter = Cast<AUR_Character>(PlayerPawn))
    {
        //NOTE: Technically RestartPlayer() supports restarting a player that is not dead.
        // In that case, the existing Pawn is not touched, but this method is still called.
        // I'm not sure if we should try to support that.
        if (URCharacter->InventoryComponent)
        {
            URCharacter->InventoryComponent->Clear();

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            SpawnParams.Owner = URCharacter;
            SpawnParams.Instigator = URCharacter;
            for (const FStartingWeaponEntry& Entry : StartingWeapons)
            {
                if (UClass* Class = Entry.WeaponClass.LoadSynchronous())
                {
                    AUR_Weapon* StartingWeapon = GetWorld()->SpawnActor<AUR_Weapon>(Class, URCharacter->GetActorLocation(), URCharacter->GetActorRotation(), SpawnParams);
                    if (StartingWeapon)
                    {
                        StartingWeapon->GiveTo(URCharacter);
                        //TODO: Weapons with multiple ammo classes
                        if (StartingWeapon->AmmoRefs.Num() > 0 && StartingWeapon->AmmoRefs[0])
                        {
                            StartingWeapon->AmmoRefs[0]->SetAmmoCount(Entry.Ammo);
                        }
                    }
                }
            }
        }
    }
    Super::SetPlayerDefaults(PlayerPawn);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Damage & Kill
/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameMode::ModifyDamage_Implementation(float& Damage, float& KnockbackPower, AUR_Character* Victim, AController* DamageInstigator, const FDamageEvent& DamageEvent, AActor* DamageCauser)
{
    if (Victim && DamageInstigator && Victim->GetController() == DamageInstigator)
    {
        Damage *= SelfDamage;
        // Full self-knockback
    }

    //NOTE: Using InstigatorController to check team is not ideal because controller can change team.
    // Character isn't any better because it can die during projectile travel time.
    // For projectiles, we should assign a team to the projectile on spawn, and fetch that upon damage.

    if (Victim && Victim->GetController() != DamageInstigator && IUR_TeamInterface::Execute_IsAlly(Victim, DamageInstigator))
    {
        const float Retaliate = TeamDamageRetaliate * Damage;
        // Retaliation counts as self damage
        if (Retaliate > 0.f && SelfDamage > 0.f)
        {
            if (APawn* InstigatorPawn = DamageInstigator->GetPawn())
            {
                InstigatorPawn->TakeDamage(Retaliate, FDamageEvent(), DamageInstigator, DamageCauser);
            }
        }

        Damage *= TeamDamageDirect;
        KnockbackPower *= TeamDamageDirect;
    }
}

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
        AUR_PlayerState* KillerPS = NULL;
        FGameplayTagContainer EventTags;

        if (VictimPS)
        {
            VictimPS->RegisterDeath(Killer, EventTags);
        }

        if (Killer && Killer != Victim)
        {
            KillerPS = Killer->GetPlayerState<AUR_PlayerState>();
            if (KillerPS)
            {
                KillerPS->RegisterKill(Victim, EventTags);
            }
        }
        else if (VictimPS)
        {
            VictimPS->RegisterSuicide(EventTags);
        }

        if (VictimPS || KillerPS)
        {
            GetGameState<AUR_GameState>()->MulticastFragEvent(VictimPS, KillerPS, DamageEvent.DamageTypeClass, EventTags);
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
    AUR_GameState* GS = GetGameState<AUR_GameState>();
    if (GS)
    {
        if (GS->Teams.Num() > 1)
        {
            AUR_TeamInfo* BestTeam = NULL;
            bool bTie = false;
            for (AUR_TeamInfo* Team : GS->Teams)
            {
                if (!BestTeam || Team->GetScore() > BestTeam->GetScore())
                {
                    BestTeam = Team;
                    bTie = false;
                }
                else if (Team->GetScore() == BestTeam->GetScore())
                {
                    bTie = true;
                }
            }
            if (!bTie)
            {
                return BestTeam;
            }
        }
        else
        {
            APlayerState* BestPlayer = NULL;
            bool bTie = false;
            for (APlayerState* PS : GS->PlayerArray)
            {
                if (!BestPlayer || PS->GetScore() > BestPlayer->GetScore())
                {
                    BestPlayer = PS;
                    bTie = false;
                }
                else if (PS->GetScore() == BestPlayer->GetScore())
                {
                    bTie = true;
                }
            }
            if (!bTie)
            {
                return BestPlayer;
            }
        }
    }

    //NOTE: maybe we should put the DM implem here because it kind of makes sense.
    // GoalScore is part of this class so we should have a default handling here.

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
    if (AUR_TeamInfo* Team = Cast<AUR_TeamInfo>(Winner))
    {
        AUR_PlayerState* Top = NULL;
        for (AUR_PlayerState* PS : Team->Players)
        {
            if (PS && PS->GetPawn() && (!Top || PS->GetScore() > Top->GetScore()))
            {
                Top = PS;
            }
        }
        if (Top)
        {
            return Top->GetPawn();
        }
    }
    return Winner;
}

void AUR_GameMode::TriggerEndMatch_Implementation(AActor* Winner, AActor* Focus)
{
    if (AUR_GameState* GS = GetGameState<AUR_GameState>())
    {
        // Winner needs to be replicated. Should never be a controller!
        if (AController* C = Cast<AController>(Winner))
        {
            if (IsValid(C->PlayerState))
                GS->Winner = C->PlayerState;
            else
                GS->Winner = C->GetPawn();
        }
        else
        {
            GS->Winner = Winner;
        }
        GS->EndGameFocus = Focus;
        GS->OnRep_Winner(); // trigger events on server side
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
        GS->OnTimeUp.RemoveDynamic(this, &ThisClass::OnMatchTimeUp);
        //NOTE: This system is a bit dangerous, we might change state completely and forget some TimeUp handlers.
        // It might be a good thing to clear all listeners whenever GS->SetTimeLimit is called.
        // Or something like that.
        GS->SetTimeLimit(10);
        GS->OnTimeUp.AddUniqueDynamic(this, &ThisClass::OnEndGameTimeUp);
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
