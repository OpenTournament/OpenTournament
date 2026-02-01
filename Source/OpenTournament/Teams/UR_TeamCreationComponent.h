// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/GameStateComponent.h"

#include "UR_TeamCreationComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_ExperienceDefinition;
class AUR_TeamPublicInfo;
class AUR_TeamPrivateInfo;
class AUR_PlayerState;
class AGameModeBase;
class APlayerController;
class UUR_TeamDisplayAsset;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Blueprintable)
class UUR_TeamCreationComponent : public UGameStateComponent
{
    GENERATED_BODY()

public:
    UUR_TeamCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~UObject interface
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
    //~End of UObject interface

    //~UActorComponent interface
    virtual void BeginPlay() override;
    //~End of UActorComponent interface

private:
    void OnExperienceLoaded(const UUR_ExperienceDefinition* Experience);

protected:
    // List of teams to create (id to display asset mapping, the display asset can be left unset if desired)
    UPROPERTY(EditDefaultsOnly, Category = Teams)
    TMap<uint8, TObjectPtr<UUR_TeamDisplayAsset>> TeamsToCreate;

    UPROPERTY(EditDefaultsOnly, Category=Teams)
    TSubclassOf<AUR_TeamPublicInfo> PublicTeamInfoClass;

    UPROPERTY(EditDefaultsOnly, Category=Teams)
    TSubclassOf<AUR_TeamPrivateInfo> PrivateTeamInfoClass;

#if WITH_SERVER_CODE

protected:
    virtual void ServerCreateTeams();
    virtual void ServerAssignPlayersToTeams();

    /** Sets the team ID for the given player state. Spectator-only player states will be stripped of any team association. */
    virtual void ServerChooseTeamForPlayer(AUR_PlayerState* PS);

private:
    void OnPlayerInitialized(AGameModeBase* GameMode, AController* NewPlayer);
    void ServerCreateTeam(int32 TeamId, UUR_TeamDisplayAsset* DisplayAsset);

    /** returns the Team ID with the fewest active players, or INDEX_NONE if there are no valid teams */
    int32 GetLeastPopulatedTeamID() const;
#endif
};
