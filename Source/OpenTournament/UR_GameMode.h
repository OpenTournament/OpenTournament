// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UR_GameModeBase.h"
#include "UR_GameMode.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_GameState;
class AUR_Weapon;
class ULocalMessage;
class UUR_Widget_ScoreboardBase;
class AUR_TeamInfo;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FStartingWeaponEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TSubclassOf<AUR_Weapon> WeaponClass;

    UPROPERTY(EditAnywhere)
    int32 Ammo;
};

namespace ETeamsFillMode
{
    static const FString Even = TEXT("Even");
    static const FString Squads = TEXT("Squads");
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * OpenTournament Game Mode
 */
UCLASS(Config = Game, Abstract, Blueprintable, HideCategories = (Tick, LOD, Cooking))
class OPENTOURNAMENT_API AUR_GameMode : public AUR_GameModeBase
{
    GENERATED_BODY()

public:

    AUR_GameMode();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * TeamInfo class.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
    TSubclassOf<AUR_TeamInfo> TeamInfoClass;

    /**
    * Scoreboard widget class
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
    TSubclassOf<UUR_Widget_ScoreboardBase> ScoreboardClass;

    /**
    * LocalMessage class to use for death/suicide messages.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
    TSubclassOf<ULocalMessage> DeathMessageClass;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters")
    int32 GoalScore;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters")
    int32 TimeLimit;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters")
    int32 OvertimeExtraTime;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters")
    TArray<FStartingWeaponEntry> StartingWeapons;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters")
    int32 MaxPlayers;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|TeamGame")
    int32 NumTeams;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|TeamGame")
    FString TeamsFillMode;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters")
    float SelfDamage;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|TeamGame")
    float TeamDamageDirect;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|TeamGame")
    float TeamDamageRetaliate;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(BlueprintReadOnly)
    int32 DesiredTeamSize;

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void InitGameState() override;

    virtual void GenericPlayerInitialization(AController* C) override;

    UFUNCTION()
    virtual void AssignDefaultTeam(AUR_PlayerState* PS);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Match
    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void HandleMatchHasStarted() override;

    UFUNCTION(BlueprintNativeEvent)
    void OnMatchTimeUp(AUR_GameState* GS);

    virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Damage & Kill
    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Damage hook.
    */
    UFUNCTION(BlueprintNativeEvent)
    void ModifyDamage(float& Damage, float& KnockbackPower, AUR_Character* Victim, AController* DamageInstigator, const FDamageEvent& DamageEvent, AActor* DamageCauser);

    /**
    * Early hook to prevent player from dying by damage.
    * If returning true, health should be set back to a positive value.
    */
    UFUNCTION(BlueprintNativeEvent)
    bool PreventDeath(AController* Victim, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser);

    /**
    * If PreventDeath returned false, this function is then called.
    * Since blueprint cannot choose specifically which Super() method to call,
    * This function should remain a simple bundle of sub-functions to call.
    */
    UFUNCTION(BlueprintNativeEvent)
    void PlayerKilled(AController* Victim, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser);

    /**
    * Core implementation for PlayerKilled.
    * Updates URPlayerStates with kills/deaths, and broadcasts death message.
    * This is typically something you want in all gamemodes, regardless of whether frags matter or not.
    */
    UFUNCTION(BlueprintCallable)
    virtual void RegisterKill(AController* Victim, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // End Game
    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Entry point to check and trigger end game.
    * Executes IsThereAWinner and EndGame if applicable.
    * If no Focus is passed, one will be calculated from the result of IsThereAWinner.
    */
    UFUNCTION(BlueprintCallable)
    virtual bool CheckEndGame(AActor* Focus);

    /**
    * Check whether there is a winner and the match should end.
    * - In non team game modes, return value should be the winning player.
    * - In team game modes, return value should be the winning team.
    * If no Focus was passed to CheckEndGame, one will be calculated from the return value.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    AActor* IsThereAWinner();

    /**
    * Try to find a good end-game focus given a winner.
    * Winner may be a pawn, controller, player state, or even a team.
    * Ideally should return a pawn, or something that supports third person.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    AActor* ResolveEndGameFocus(AActor* Winner);

    /**
    * Set the end game variables and triggers match ending.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void TriggerEndMatch(AActor* Winner, AActor* Focus);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void AnnounceWinner(AActor* Winner);

    UFUNCTION()
    virtual void OnEndGameTimeUp(AUR_GameState* GS);

protected:

    virtual void HandleMatchHasEnded() override;

};
