// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UR_TeamInterface.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_TeamInfo;

/////////////////////////////////////////////////////////////////////////////////////////////////

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UUR_TeamInterface : public UInterface
{
    GENERATED_BODY()
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Implement this interface to give the object a sense of belonging.
 */
class OPENTOURNAMENT_API IUR_TeamInterface
{
    GENERATED_BODY()

public:
    /**
    * Function to implement
    * Resolve team index.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    int32 GetTeamIndex();

    virtual int32 GetTeamIndex_Implementation();

    /**
    * Function to implement (optional)
    * Change team index.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SetTeamIndex(int32 NewTeamIndex);

    virtual void SetTeamIndex_Implementation(int32 NewTeamIndex);

    /**
    * Utility
    * Resolve TeamInfo object using TeamIndex.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    AUR_TeamInfo* GetTeam();

    virtual AUR_TeamInfo* GetTeam_Implementation();

    /**
    * Utility
    * Set TeamIndex from a TeamInfo object.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SetTeam(AUR_TeamInfo* NewTeam);

    virtual void SetTeam_Implementation(AUR_TeamInfo* NewTeam);

    /**
    * Utility (overridable)
    * Resolve whether another object is an ally of mine.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool IsAlly(UObject* Other);

    virtual bool IsAlly_Implementation(UObject* Other);

    /**
    * Utility (overridable)
    * Resolve whether another object is an enemy of mine.
    * By default, anything that isn't an ally, is an enemy.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool IsEnemy(UObject* Other);

    virtual bool IsEnemy_Implementation(UObject* Other);
};
