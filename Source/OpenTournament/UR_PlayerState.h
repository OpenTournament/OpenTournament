// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/PlayerState.h"

#include "UR_PlayerState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API AUR_PlayerState : public APlayerState
{
    GENERATED_BODY()
    
    AUR_PlayerState();

protected:

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
};
