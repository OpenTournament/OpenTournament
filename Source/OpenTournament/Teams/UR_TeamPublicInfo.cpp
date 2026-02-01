// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TeamPublicInfo.h"

#include "Net/UnrealNetwork.h"
#include "Teams/UR_TeamInfoBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TeamPublicInfo)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_TeamPublicInfo::AUR_TeamPublicInfo(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Noop
}

void AUR_TeamPublicInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ThisClass, TeamDisplayAsset, COND_InitialOnly);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_TeamDisplayAsset* AUR_TeamPublicInfo::GetTeamDisplayAsset() const
{
    return TeamDisplayAsset;
}

void AUR_TeamPublicInfo::SetTeamDisplayAsset(TObjectPtr<UUR_TeamDisplayAsset> NewDisplayAsset)
{
    check(HasAuthority());
    check(TeamDisplayAsset == nullptr);

    TeamDisplayAsset = NewDisplayAsset;

    TryRegisterWithTeamSubsystem();
}

void AUR_TeamPublicInfo::OnRep_TeamDisplayAsset()
{
    TryRegisterWithTeamSubsystem();
}
