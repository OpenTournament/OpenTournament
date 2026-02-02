// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GenericTeamAgentInterface.h"

#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"

#include "UR_TeamAgentInterface.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

template <typename InterfaceType>
class TScriptInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGameTeamIndexChangedDelegate, UObject*, ObjectChangingTeam, int32, OldTeamID, int32, NewTeamID);

/////////////////////////////////////////////////////////////////////////////////////////////////

inline int32 GenericTeamIdToInteger(FGenericTeamId ID)
{
    return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : static_cast<int32>(ID);
}

inline FGenericTeamId IntegerToGenericTeamId(int32 ID)
{
    return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId(static_cast<uint8>(ID));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

/** Interface for actors which can be associated with teams */
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UUR_TeamAgentInterface : public UGenericTeamAgentInterface
{
    GENERATED_UINTERFACE_BODY()
};

class OPENTOURNAMENT_API IUR_TeamAgentInterface : public IGenericTeamAgentInterface
{
    GENERATED_IINTERFACE_BODY()
    virtual FOnGameTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate()
    {
        return nullptr;
    }

    static void ConditionalBroadcastTeamChanged(TScriptInterface<IUR_TeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);

    FOnGameTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
    {
        FOnGameTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
        check(Result);
        return *Result;
    }
};
