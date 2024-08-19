// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UR_GameModeBase.h"
#include "UR_GameMode.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_PawnData;
class UUR_ExperienceDefinition;
class AUR_GameState;
class AUR_Weapon;
class UUR_Widget_ScoreboardBase;
class AUR_TeamInfo;
class AUR_BotController;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FStartingWeaponEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TSoftClassPtr<AUR_Weapon> WeaponClass;

    UPROPERTY(EditAnywhere)
    int32 Ammo;

    FStartingWeaponEntry()
        : Ammo(0)
    {
    }
};

namespace ETeamsFillMode
{
    static const FString Even = TEXT("Even");
    static const FString Squads = TEXT("Squads");
};

/**
* Sub-states for the InProgress match state.
*
* We don't want to mess with the existing framework provided for MatchState.
* GameMode.h specifically states the following :
* MatchState::InProgress = Normal gameplay is occurring. Specific games will have their own state machine inside this state
*/
namespace MatchSubState
{
    extern OPENTOURNAMENT_API const FName Warmup;
    extern OPENTOURNAMENT_API const FName Countdown;
    extern OPENTOURNAMENT_API const FName Match;
    extern OPENTOURNAMENT_API const FName Overtime;
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

#pragma region AGameModeBaseInterface

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

    virtual void InitGameState() override;

    virtual void GenericPlayerInitialization(AController* C) override;

#pragma endregion AGameModeBaseInterface


    UFUNCTION(BlueprintCallable, Category = "OT|Pawn")
    const UUR_PawnData* GetPawnDataForController(const AController* InController) const;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Classes
    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
    TSubclassOf<AUR_TeamInfo> TeamInfoClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
    TSubclassOf<UUR_Widget_ScoreboardBase> ScoreboardClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
    TSubclassOf<AUR_BotController> BotControllerClass;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Configuration
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

    /**
    * Fill match with bots (if necessary) to reach this total amount of players.
    *  0 = disable
    * -1 = use map minimum recommended players from map settings (TODO)
    */
    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|Bots")
    int32 BotFill;

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

    UPROPERTY(BlueprintReadOnly)
    int32 DesiredTeamSize;

    UFUNCTION(BlueprintCallable)
    void BroadcastSystemMessage(const FString& Msg);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Players and bots flow
    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void OnPostLogin(AController* NewPlayer) override;

    virtual void Logout(AController* Exiting) override;

    UFUNCTION()
    virtual void AssignDefaultTeam(AUR_PlayerState* PS);

    /**
    * Add or remove bots according to game parameters and current players.
    * Call this whenever something changes (player join, player leave, player change team).
    */
    UFUNCTION(BlueprintCallable)
    void CheckBotsDeferred();

    virtual void CheckBots();

    /**
    * Create and initialize a new AI player using BotControllerClass.
    */
    UFUNCTION(BlueprintCallable)
    virtual void AddBot();

    /**
    * Find a non-player controller with an active playerstate and destroy it.
    */
    UFUNCTION(BlueprintCallable)
    virtual void RemoveBot();

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

    virtual void HandleMatchHasEnded() override;

    //

    void OnExperienceLoaded(const UUR_ExperienceDefinition* CurrentExperience);

    bool IsExperienceLoaded() const;
};
