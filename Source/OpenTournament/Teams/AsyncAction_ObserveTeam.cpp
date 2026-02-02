// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Teams/AsyncAction_ObserveTeam.h"

#include "Teams/UR_TeamAgentInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ObserveTeam)

/////////////////////////////////////////////////////////////////////////////////////////////////

UAsyncAction_ObserveTeam::UAsyncAction_ObserveTeam(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Noop
}

UAsyncAction_ObserveTeam* UAsyncAction_ObserveTeam::ObserveTeam(UObject* TeamAgent)
{
    return InternalObserveTeamChanges(TeamAgent);
}

UAsyncAction_ObserveTeam* UAsyncAction_ObserveTeam::InternalObserveTeamChanges(TScriptInterface<IUR_TeamAgentInterface> TeamActor)
{
    UAsyncAction_ObserveTeam* Action = nullptr;

    if (TeamActor != nullptr)
    {
        Action = NewObject<UAsyncAction_ObserveTeam>();
        Action->TeamInterfacePtr = TeamActor;
        Action->RegisterWithGameInstance(TeamActor.GetObject());
    }

    return Action;
}

void UAsyncAction_ObserveTeam::SetReadyToDestroy()
{
    Super::SetReadyToDestroy();

    // If we're being canceled we need to unhook everything we might have tried listening to.
    if (IUR_TeamAgentInterface* TeamInterface = TeamInterfacePtr.Get())
    {
        TeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
    }
}

void UAsyncAction_ObserveTeam::Activate()
{
    bool bCouldSucceed = false;
    int32 CurrentTeamIndex = INDEX_NONE;

    if (IUR_TeamAgentInterface* TeamInterface = TeamInterfacePtr.Get())
    {
        CurrentTeamIndex = GenericTeamIdToInteger(TeamInterface->GetGenericTeamId());

        TeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnWatchedAgentChangedTeam);

        bCouldSucceed = true;
    }

    // Broadcast once so users get the current state
    OnTeamChanged.Broadcast(CurrentTeamIndex != INDEX_NONE, CurrentTeamIndex);

    // We weren't able to bind to a delegate so we'll never get any additional updates
    if (!bCouldSucceed)
    {
        SetReadyToDestroy();
    }
}

void UAsyncAction_ObserveTeam::OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
    OnTeamChanged.Broadcast(NewTeam != INDEX_NONE, NewTeam);
}
