// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ModularAIController.h"
#include "Teams/UR_TeamAgentInterface.h"

#include "UR_PlayerBotController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ETeamAttitude
{
    enum Type : int;
}

struct FGenericTeamId;

class APlayerState;
class UAIPerceptionComponent;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * AUR_PlayerBotController
 *
 *	The controller class used by player bots in this project.
 */
UCLASS(Blueprintable)
class AUR_PlayerBotController : public AModularAIController, public IUR_TeamAgentInterface
{
    GENERATED_BODY()

public:
    AUR_PlayerBotController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~IUR_TeamAgentInterface interface
    virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

    virtual FGenericTeamId GetGenericTeamId() const override;

    virtual FOnGameTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;

    ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

    //~End of IUR_TeamAgentInterface interface

    // Attempts to restart this controller (e.g., to respawn it)
    void ServerRestartController();

    //Update Team Attitude for the AI
    UFUNCTION(BlueprintCallable, Category = "OT AI Player Controller")
    void UpdateTeamAttitude(UAIPerceptionComponent* AIPerception);

    virtual void OnUnPossess() override;

private:
    UFUNCTION()
    void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
    // Called when the player state is set or cleared
    virtual void OnPlayerStateChanged();

private:
    void BroadcastOnPlayerStateChanged();

protected:
    //~AController interface
    virtual void InitPlayerState() override;

    virtual void CleanupPlayerState() override;

    virtual void OnRep_PlayerState() override;

    //~End of AController interface

private:
    UPROPERTY()
    FOnGameTeamIndexChangedDelegate OnTeamChangedDelegate;

    UPROPERTY()
    TObjectPtr<APlayerState> LastSeenPlayerState;
};
