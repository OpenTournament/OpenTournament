// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "UObject/ScriptInterface.h"
#include "UObject/WeakInterfacePtr.h"

#include "AsyncAction_ObserveTeam.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class IUR_TeamAgentInterface;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamObservedAsyncDelegate, bool, bTeamSet, int32, TeamId);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Watches for team changes in the specified object
 */
UCLASS()
class UAsyncAction_ObserveTeam : public UCancellableAsyncAction
{
    GENERATED_UCLASS_BODY()

public:
    // Watches for team changes on the specified team agent
    //  - It will will fire once immediately to give the current team assignment
    //  - For anything that can ever belong to a team (implements IUR_TeamAgentInterface),
    //    it will also listen for team assignment changes in the future
    UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Keywords="Watch"))
    static UAsyncAction_ObserveTeam* ObserveTeam(UObject* TeamAgent);

    //~UBlueprintAsyncActionBase interface
    virtual void Activate() override;
    virtual void SetReadyToDestroy() override;
    //~End of UBlueprintAsyncActionBase interface

public:
    // Called when the team is set or changed
    UPROPERTY(BlueprintAssignable)
    FTeamObservedAsyncDelegate OnTeamChanged;

private:
    // Watches for team changes on the specified team actor
    static UAsyncAction_ObserveTeam* InternalObserveTeamChanges(TScriptInterface<IUR_TeamAgentInterface> TeamActor);

private:
    UFUNCTION()
    void OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

    TWeakInterfacePtr<IUR_TeamAgentInterface> TeamInterfacePtr;
};
