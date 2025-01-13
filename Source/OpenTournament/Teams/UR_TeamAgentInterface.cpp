// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Teams/UR_TeamAgentInterface.h"

#include "UR_LogChannels.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TeamAgentInterface)

UUR_TeamAgentInterface::UUR_TeamAgentInterface(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void IUR_TeamAgentInterface::ConditionalBroadcastTeamChanged(TScriptInterface<IUR_TeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
    if (OldTeamID != NewTeamID)
    {
        const int32 OldTeamIndex = GenericTeamIdToInteger(OldTeamID);
        const int32 NewTeamIndex = GenericTeamIdToInteger(NewTeamID);

        UObject* ThisObj = This.GetObject();
        UE_LOG(LogGameTeams, Verbose, TEXT("[%s] %s assigned team %d"), *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamIndex);

        This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);
    }
}
