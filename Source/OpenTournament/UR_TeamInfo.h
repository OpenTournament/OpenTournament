// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/Actor.h"

#include "Interfaces/UR_TeamInterface.h"

#include "UR_TeamInfo.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_PlayerState;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Event dispatcher player removed from team.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerLeftTeamSignature, AUR_TeamInfo*, Team, AUR_PlayerState*, PS);

/**
* Event dispatcher player added to team.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerJoinedTeamSignature, AUR_TeamInfo*, Team, AUR_PlayerState*, PS);

// @! TODO : Maybe deprecate per GameStateComponent for Team-Based Game
/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API AUR_TeamInfo
    : public AActor
    , public IUR_TeamInterface
{
    GENERATED_BODY()

    AUR_TeamInfo();

protected:
    UPROPERTY(Replicated)
    int32 TeamIndex;

    UPROPERTY(Replicated)
    int32 Score;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void BeginPlay() override;

public:
    /** Array of PlayerStates, maintained on both server and clients (PlayerStates are always relevant) */
    UPROPERTY(BlueprintReadOnly)
    TArray<AUR_PlayerState*> Players;

    UFUNCTION(BlueprintCallable)
    virtual void RemovePlayer(AUR_PlayerState* Player);

    UFUNCTION(BlueprintCallable)
    virtual void AddPlayer(AUR_PlayerState* Player);

    UFUNCTION(BlueprintPure)
    FORCEINLINE int32 GetScore() const
    {
        return Score;
    }

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void AddScore(int32 Value);

    //~ Begin TeamInterface
    virtual int32 GetTeamIndex_Implementation() override;

    virtual void SetTeamIndex_Implementation(int32 NewTeamIndex) override;

    //~ End TeamInterface

    /**
    * Utility - Resolve TeamIndex into a TeamInfo object via URGameState Teams array.
    */
    UFUNCTION(BlueprintPure, Category = "Utility", Meta = (WorldContext = "WorldContextObject"))
    static AUR_TeamInfo* GetTeamFromIndex(const UObject* WorldContextObject, int32 InTeamIndex);

    UPROPERTY(BlueprintAssignable)
    FPlayerLeftTeamSignature OnPlayerLeftTeam;

    UPROPERTY(BlueprintAssignable)
    FPlayerJoinedTeamSignature OnPlayerJoinedTeam;
};
