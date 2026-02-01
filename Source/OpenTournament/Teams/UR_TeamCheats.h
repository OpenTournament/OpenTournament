// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/CheatManager.h"

#include "UR_TeamCheats.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

/** Cheats related to teams */
UCLASS()
class UUR_TeamCheats : public UCheatManagerExtension
{
    GENERATED_BODY()

public:
    // Moves this player to the next available team, wrapping around to the
    // first team if at the end of the list of teams
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    virtual void CycleTeam();

    // Moves this player to the specified team
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    virtual void SetTeam(int32 TeamID);

    // Prints a list of all of the teams
    UFUNCTION(Exec)
    virtual void ListTeams();
};
