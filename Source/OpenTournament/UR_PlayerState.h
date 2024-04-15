// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ModularPlayerState.h>

#include "GameFramework/PlayerState.h"
#include "Interfaces/UR_TeamInterface.h"

#include "GameplayTagContainer.h"

#include "Character/UR_CharacterCustomization.h"

#include "UR_PlayerState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

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
UCLASS()
class OPENTOURNAMENT_API AUR_PlayerState
    : public AModularPlayerState
    , public IUR_TeamInterface
{
    GENERATED_BODY()

    AUR_PlayerState();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnRep_ReplicatedTeamIndex();

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
};
