// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TeamInfoBase.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

#include "Teams/UR_TeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TeamInfoBase)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_TeamInfoBase::AUR_TeamInfoBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
  , TeamId(INDEX_NONE)
{
    bReplicates = true;
    bAlwaysRelevant = true;
    NetPriority = 3.0f;
    SetReplicatingMovement(false);
}

void AUR_TeamInfoBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, TeamTags);
    DOREPLIFETIME_CONDITION(ThisClass, TeamId, COND_InitialOnly);
}

void AUR_TeamInfoBase::BeginPlay()
{
    Super::BeginPlay();

    TryRegisterWithTeamSubsystem();
}

void AUR_TeamInfoBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (TeamId != INDEX_NONE)
    {
        UUR_TeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UUR_TeamSubsystem>();
        if (TeamSubsystem)
        {
            // EndPlay can happen at weird times where the subsystem has already been destroyed
            TeamSubsystem->UnregisterTeamInfo(this);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void AUR_TeamInfoBase::RegisterWithTeamSubsystem(UUR_TeamSubsystem* Subsystem)
{
    Subsystem->RegisterTeamInfo(this);
}

void AUR_TeamInfoBase::TryRegisterWithTeamSubsystem()
{
    if (TeamId != INDEX_NONE)
    {
        UUR_TeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UUR_TeamSubsystem>();
        if (ensure(TeamSubsystem))
        {
            RegisterWithTeamSubsystem(TeamSubsystem);
        }
    }
}

void AUR_TeamInfoBase::SetTeamId(int32 NewTeamId)
{
    check(HasAuthority());
    check(TeamId == INDEX_NONE);
    check(NewTeamId != INDEX_NONE);

    TeamId = NewTeamId;

    TryRegisterWithTeamSubsystem();
}

void AUR_TeamInfoBase::OnRep_TeamId()
{
    TryRegisterWithTeamSubsystem();
}
