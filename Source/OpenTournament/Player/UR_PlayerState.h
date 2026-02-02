// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ModularPlayerState.h>

#include <AbilitySystemInterface.h>
#include <GameplayTagContainer.h>

#include "GameplayTagStack.h"
#include "UR_TeamAgentInterface.h"
#include "Character/UR_CharacterCustomization.h"
#include "Interfaces/UR_TeamInterface.h"

#include "UR_PlayerState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_PawnData;
class UUR_ExperienceDefinition;
class UUR_AbilitySystemComponent;
class AUR_PlayerState;
class AUR_TeamInfo;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Event dispatcher for team change.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTeamChangedSignature, AUR_PlayerState*, PS, int32, OldTeamIndex, int32, NewTeamIndex);


/**
 *
 */
UCLASS(Config = Game)
class OPENTOURNAMENT_API AUR_PlayerState
    : public AModularPlayerState
    , public IAbilitySystemInterface
    , public IUR_TeamAgentInterface
    , public IUR_TeamInterface
{
    GENERATED_BODY()

public:
    AUR_PlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    //~AActor interface
    virtual void PreInitializeComponents() override;
    virtual void PostInitializeComponents() override;
    //~End of AActor interface

    //~APlayerState interface
    virtual void BeginPlay() override;
    //~APlayerState interface

    //~AbilitySystem interface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    //~End of AbilitySystem interface


    UFUNCTION()
    virtual void OnRep_ReplicatedTeamIndex();

public:

    UFUNCTION(BlueprintPure, Category = "OT|PlayerState")
    UUR_AbilitySystemComponent* GetGameAbilitySystemComponent() const { return AbilitySystemComponent; }

    template <class T>
    const T* GetPawnData() const { return Cast<T>(PawnData); }

    void SetPawnData(const UUR_PawnData* InPawnData);

    static const FName NAME_GameAbilityReady;

    /** Returns the Squad ID of the squad the player belongs to. */
    UFUNCTION(BlueprintPure)
    int32 GetSquadId() const
    {
        return MySquadID;
    }

    /** Returns the Team ID of the team the player belongs to. */
    UFUNCTION(BlueprintPure)
    int32 GetTeamId() const
    {
        return GenericTeamIdToInteger(MyTeamID);
    }

private:
    void OnExperienceLoaded(const UUR_ExperienceDefinition* CurrentExperience);

protected:
    UFUNCTION()
    void OnRep_PawnData();

    UPROPERTY(ReplicatedUsing = OnRep_PawnData)
    TObjectPtr<const UUR_PawnData> PawnData;

    //
public:
    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 Kills;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 Deaths;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 Suicides;

    UPROPERTY(BlueprintReadOnly)
    float LastKillTime;

    UPROPERTY(BlueprintReadOnly)
    int32 MultiKillCount;

    /** Exact kills count for the spree */
    UPROPERTY(BlueprintReadOnly)
    int32 SpreeCount;

    /** Spree level (incremented every 5 kills) */
    UPROPERTY(BlueprintReadOnly)
    int32 SpreeLevel;

    /** Track last killer for "revenge" */
    UPROPERTY(BlueprintReadOnly)
    APawn* LastKiller;

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void RegisterKill(AController* Victim, UPARAM(Ref) FGameplayTagContainer& OutTags);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void RegisterDeath(AController* Killer, UPARAM(Ref) FGameplayTagContainer& OutTags);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void RegisterSuicide(UPARAM(Ref) FGameplayTagContainer& OutTags);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void AddScore(int32 Value);

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:
    UPROPERTY()
    int32 TeamIndex;

    UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTeamIndex)
    int32 ReplicatedTeamIndex;

    /**
    * Cache Team here for easier c++ access because interfaces are pain in the ass.
    * Blueprints should use interface methods GetTeam() and GetTeamIndex().
    */
    UPROPERTY()
    AUR_TeamInfo* Team;

    //~ Begin TeamInterface
    virtual int32 GetTeamIndex_Implementation() override;

    virtual void SetTeamIndex_Implementation(int32 NewTeamIndex) override;
    //~ End TeamInterface

    UPROPERTY(BlueprintAssignable)
    FTeamChangedSignature OnTeamChanged;

    UFUNCTION()
    virtual void InternalOnTeamChanged(AUR_PlayerState* PS, int32 OldTeamIndex, int32 NewTeamIndex);

    UFUNCTION(BlueprintPure, BlueprintCosmetic)
    virtual FLinearColor GetColor();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Valid to call after end of game, and after GameState->Winner has been replicated.
    * Return true if player has won the match.
    */
    UFUNCTION(BlueprintPure)
    bool IsAWinner();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CharacterCustomization)
    FCharacterCustomization CharacterCustomization;

    UFUNCTION(Server, Reliable)
    void ServerSetCharacterCustomization(const FCharacterCustomization& InCustomization);

    UFUNCTION()
    virtual void OnRep_CharacterCustomization();

    UFUNCTION()
    virtual void InternalOnPawnSet(APlayerState* PS, APawn* NewPawn, APawn* OldPawn);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // The ability system component sub-object used by player characters.
    UPROPERTY(VisibleAnywhere, Category = "Game|PlayerState")
    TObjectPtr<UUR_AbilitySystemComponent> AbilitySystemComponent;

    // Health attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<const class UUR_HealthSet> HealthSet;

    // Combat attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<const class UUR_CombatSet> CombatSet;

    UPROPERTY()
    FOnGameTeamIndexChangedDelegate OnTeamChangedDelegate;

    UPROPERTY(ReplicatedUsing=OnRep_MyTeamID)
    FGenericTeamId MyTeamID;

    UPROPERTY(ReplicatedUsing=OnRep_MySquadID)
    int32 MySquadID;

    UPROPERTY(Replicated)
    FGameplayTagStackContainer StatTags;

    UPROPERTY(Replicated)
    FRotator ReplicatedViewRotation;

private:
    UFUNCTION()
    void OnRep_MyTeamID(FGenericTeamId OldTeamID);

    UFUNCTION()
    void OnRep_MySquadID();
};
