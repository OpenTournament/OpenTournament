// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/PlayerState.h"
#include "Interfaces/UR_TeamInterface.h"
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
class OPENTOURNAMENT_API AUR_PlayerState : public APlayerState
    , public IUR_TeamInterface
{
    GENERATED_BODY()
    
    AUR_PlayerState();

protected:

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    virtual void OnRep_ReplicatedTeamIndex();

public:

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 Kills;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 Deaths;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 Suicides;

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void AddKill(AController* Victim);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void AddDeath(AController* Killer);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void AddSuicide();

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

};
