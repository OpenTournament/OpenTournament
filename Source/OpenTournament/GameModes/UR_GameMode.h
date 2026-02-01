// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_GameModeBase.h"

#include "UR_GameMode.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Character;
class AUR_PlayerState;
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
    {}
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

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

#pragma region AGameModeBaseInterface
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
    virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
    virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
    virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
    virtual void InitGameState() override;
    virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
    virtual void GenericPlayerInitialization(AController* NewPlayer) override;
    virtual void FailedToRestartPlayer(AController* NewPlayer) override;
#pragma endregion AGameModeBaseInterface

    // Restart (respawn) the specified player or bot next frame
    // - If bForceReset is true, the controller will be reset this frame (abandoning the currently possessed pawn, if any)
    UFUNCTION(BlueprintCallable)
    void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

    // Agnostic version of PlayerCanRestart that can be used for both player bots and players
    virtual bool ControllerCanRestart(AController* Controller);

    // Delegate called on player initialization, described above
    FOnGameModePlayerInitialized OnGameModePlayerInitialized;

    UFUNCTION(BlueprintCallable, Category = "OT|Pawn")
    const UUR_PawnData* GetPawnDataForController(const AController* InController) const;


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Classes
    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes", meta = (DeprecatedProperty))
    TSubclassOf<AUR_TeamInfo> TeamInfoClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes", meta = (DeprecatedProperty))
    TSubclassOf<UUR_Widget_ScoreboardBase> ScoreboardClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes", meta = (DeprecatedProperty))
    TSubclassOf<AUR_BotController> BotControllerClass;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Configuration
    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters", meta = (DeprecatedProperty))
    int32 GoalScore;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters", meta = (DeprecatedProperty))
    int32 TimeLimit;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters", meta = (DeprecatedProperty))
    int32 OvertimeExtraTime;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters", meta = (DeprecatedProperty))
    TArray<FStartingWeaponEntry> StartingWeapons;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters", meta = (DeprecatedProperty))
    int32 MaxPlayers;

    /**
    * Fill match with bots (if necessary) to reach this total amount of players.
    *  0 = disable
    * -1 = use map minimum recommended players from map settings (TODO)
    */
    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|Bots", meta = (DeprecatedProperty))
    int32 BotFill;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|TeamGame", meta = (DeprecatedProperty))
    int32 NumTeams;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|TeamGame", meta = (DeprecatedProperty))
    FString TeamsFillMode;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters", meta = (DeprecatedProperty))
    float SelfDamage;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|TeamGame", meta = (DeprecatedProperty))
    float TeamDamageDirect;

    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Parameters|TeamGame", meta = (DeprecatedProperty))
    float TeamDamageRetaliate;

    UPROPERTY(BlueprintReadOnly, meta = (DeprecatedProperty))
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

    virtual void OnExperienceLoaded(const UUR_ExperienceDefinition* CurrentExperience) override;

    virtual bool IsExperienceLoaded() const override;
};
