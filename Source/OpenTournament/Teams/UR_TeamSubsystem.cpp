// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Teams/UR_TeamSubsystem.h"

#include <AbilitySystemGlobals.h>
#include <GameFramework/Controller.h>
#include <GameFramework/Pawn.h>

#include "UR_LogChannels.h"
#include "UR_TeamAgentInterface.h"
#include "UR_TeamCheats.h"
#include "UR_TeamPrivateInfo.h"
#include "UR_TeamPublicInfo.h"
#include "Player/UR_PlayerState.h"
#include "Teams/UR_TeamInfoBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TeamSubsystem)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FSubsystemCollectionBase;

/////////////////////////////////////////////////////////////////////////////////////////////////
// FGameTeamTrackingInfo

void FGameTeamTrackingInfo::SetTeamInfo(AUR_TeamInfoBase* Info)
{
    if (AUR_TeamPublicInfo* NewPublicInfo = Cast<AUR_TeamPublicInfo>(Info))
    {
        ensure((PublicInfo == nullptr) || (PublicInfo == NewPublicInfo));
        PublicInfo = NewPublicInfo;

        UUR_TeamDisplayAsset* OldDisplayAsset = DisplayAsset;
        DisplayAsset = NewPublicInfo->GetTeamDisplayAsset();

        if (OldDisplayAsset != DisplayAsset)
        {
            OnTeamDisplayAssetChanged.Broadcast(DisplayAsset);
        }
    }
    else if (AUR_TeamPrivateInfo* NewPrivateInfo = Cast<AUR_TeamPrivateInfo>(Info))
    {
        ensure((PrivateInfo == nullptr) || (PrivateInfo == NewPrivateInfo));
        PrivateInfo = NewPrivateInfo;
    }
    else
    {
        checkf(false, TEXT("Expected a public or private team info but got %s"), *GetPathNameSafe(Info))
    }
}

void FGameTeamTrackingInfo::RemoveTeamInfo(AUR_TeamInfoBase* Info)
{
    if (PublicInfo == Info)
    {
        PublicInfo = nullptr;
    }
    else if (PrivateInfo == Info)
    {
        PrivateInfo = nullptr;
    }
    else
    {
        ensureMsgf(false, TEXT("Expected a previously registered team info but got %s"), *GetPathNameSafe(Info));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// UUR_TeamSubsystem

UUR_TeamSubsystem::UUR_TeamSubsystem()
{
    // Noop
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_TeamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    auto AddTeamCheats = [](UCheatManager* CheatManager)
    {
        CheatManager->AddCheatManagerExtension(NewObject<UUR_TeamCheats>(CheatManager));
    };

    CheatManagerRegistrationHandle = UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(AddTeamCheats));
}

void UUR_TeamSubsystem::Deinitialize()
{
    UCheatManager::UnregisterFromOnCheatManagerCreated(CheatManagerRegistrationHandle);

    Super::Deinitialize();
}

bool UUR_TeamSubsystem::RegisterTeamInfo(AUR_TeamInfoBase* TeamInfo)
{
    if (!ensure(TeamInfo))
    {
        return false;
    }

    const int32 TeamId = TeamInfo->GetTeamId();
    if (ensure(TeamId != INDEX_NONE))
    {
        FGameTeamTrackingInfo& Entry = TeamMap.FindOrAdd(TeamId);
        Entry.SetTeamInfo(TeamInfo);

        return true;
    }

    return false;
}

bool UUR_TeamSubsystem::UnregisterTeamInfo(AUR_TeamInfoBase* TeamInfo)
{
    if (!ensure(TeamInfo))
    {
        return false;
    }

    const int32 TeamId = TeamInfo->GetTeamId();
    if (ensure(TeamId != INDEX_NONE))
    {
        FGameTeamTrackingInfo* Entry = TeamMap.Find(TeamId);

        // If it couldn't find the entry, this is probably a leftover actor from a previous world, ignore it
        if (Entry)
        {
            Entry->RemoveTeamInfo(TeamInfo);

            return true;
        }
    }

    return false;
}

bool UUR_TeamSubsystem::ChangeTeamForActor(AActor* ActorToChange, const int32 NewTeamIndex) const
{
    const FGenericTeamId NewTeamID = IntegerToGenericTeamId(NewTeamIndex);
    if (AUR_PlayerState* PS = const_cast<AUR_PlayerState*>(FindPlayerStateFromActor(ActorToChange)))
    {
        PS->SetGenericTeamId(NewTeamID);
        return true;
    }
    else if (IUR_TeamAgentInterface* TeamActor = Cast<IUR_TeamAgentInterface>(ActorToChange))
    {
        TeamActor->SetGenericTeamId(NewTeamID);
        return true;
    }
    else
    {
        return false;
    }
}

int32 UUR_TeamSubsystem::FindTeamFromObject(const UObject* TestObject) const
{
    // See if it's directly a team agent
    if (const IUR_TeamAgentInterface* ObjectWithTeamInterface = Cast<IUR_TeamAgentInterface>(TestObject))
    {
        return GenericTeamIdToInteger(ObjectWithTeamInterface->GetGenericTeamId());
    }

    if (const AActor* TestActor = Cast<const AActor>(TestObject))
    {
        // See if the instigator is a team actor
        if (const IUR_TeamAgentInterface* InstigatorWithTeamInterface = Cast<IUR_TeamAgentInterface>(TestActor->GetInstigator()))
        {
            return GenericTeamIdToInteger(InstigatorWithTeamInterface->GetGenericTeamId());
        }

        // TeamInfo actors don't actually have the team interface, so they need a special case
        if (const AUR_TeamInfoBase* TeamInfo = Cast<AUR_TeamInfoBase>(TestActor))
        {
            return TeamInfo->GetTeamId();
        }

        // Fall back to finding the associated player state
        if (const AUR_PlayerState* PS = FindPlayerStateFromActor(TestActor))
        {
            return PS->GetTeamId();
        }
    }

    return INDEX_NONE;
}

const AUR_PlayerState* UUR_TeamSubsystem::FindPlayerStateFromActor(const AActor* PossibleTeamActor) const
{
    if (PossibleTeamActor != nullptr)
    {
        if (const APawn* Pawn = Cast<const APawn>(PossibleTeamActor))
        {
            //@TODO: Consider an interface instead or have team actors register with the subsystem and have it maintain a map? (or LWC style)
            if (const AUR_PlayerState* PS = Pawn->GetPlayerState<AUR_PlayerState>())
            {
                return PS;
            }
        }
        else if (const AController* PC = Cast<const AController>(PossibleTeamActor))
        {
            if (const AUR_PlayerState* PS = Cast<AUR_PlayerState>(PC->PlayerState))
            {
                return PS;
            }
        }
        else if (const AUR_PlayerState* PS = Cast<const AUR_PlayerState>(PossibleTeamActor))
        {
            return PS;
        }

        // Try the instigator
        // 		if (AActor* Instigator = PossibleTeamActor->GetInstigator())
        // 		{
        // 			if (ensure(Instigator != PossibleTeamActor))
        // 			{
        // 				return FindPlayerStateFromActor(Instigator);
        // 			}
        // 		}
    }

    return nullptr;
}

EGameTeamComparison UUR_TeamSubsystem::CompareTeams(const UObject* A, const UObject* B, int32& TeamIdA, int32& TeamIdB) const
{
    TeamIdA = FindTeamFromObject(Cast<const AActor>(A));
    TeamIdB = FindTeamFromObject(Cast<const AActor>(B));

    if ((TeamIdA == INDEX_NONE) || (TeamIdB == INDEX_NONE))
    {
        return EGameTeamComparison::InvalidArgument;
    }
    else
    {
        return (TeamIdA == TeamIdB) ? EGameTeamComparison::OnSameTeam : EGameTeamComparison::DifferentTeams;
    }
}

EGameTeamComparison UUR_TeamSubsystem::CompareTeams(const UObject* A, const UObject* B) const
{
    int32 TeamIdA;
    int32 TeamIdB;
    return CompareTeams(A, B, /*out*/ TeamIdA, /*out*/ TeamIdB);
}

void UUR_TeamSubsystem::FindTeamFromActor(const UObject* TestObject, bool& bIsPartOfTeam, int32& TeamId) const
{
    TeamId = FindTeamFromObject(TestObject);
    bIsPartOfTeam = TeamId != INDEX_NONE;
}

void UUR_TeamSubsystem::AddTeamTagStack(int32 TeamId, FGameplayTag Tag, int32 StackCount)
{
    auto FailureHandler = [&](const FString& ErrorMessage)
    {
        UE_LOG(LogGameTeams, Error, TEXT("AddTeamTagStack(TeamId: %d, Tag: %s, StackCount: %d) %s"), TeamId, *Tag.ToString(), StackCount, *ErrorMessage);
    };

    if (FGameTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
    {
        if (Entry->PublicInfo)
        {
            if (Entry->PublicInfo->HasAuthority())
            {
                Entry->PublicInfo->TeamTags.AddStack(Tag, StackCount);
            }
            else
            {
                FailureHandler(TEXT("failed because it was called on a client"));
            }
        }
        else
        {
            FailureHandler(TEXT("failed because there is no team info spawned yet (called too early, before the experience was ready)"));
        }
    }
    else
    {
        FailureHandler(TEXT("failed because it was passed an unknown team id"));
    }
}

void UUR_TeamSubsystem::RemoveTeamTagStack(int32 TeamId, FGameplayTag Tag, int32 StackCount)
{
    auto FailureHandler = [&](const FString& ErrorMessage)
    {
        UE_LOG(LogGameTeams, Error, TEXT("RemoveTeamTagStack(TeamId: %d, Tag: %s, StackCount: %d) %s"), TeamId, *Tag.ToString(), StackCount, *ErrorMessage);
    };

    if (FGameTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
    {
        if (Entry->PublicInfo)
        {
            if (Entry->PublicInfo->HasAuthority())
            {
                Entry->PublicInfo->TeamTags.RemoveStack(Tag, StackCount);
            }
            else
            {
                FailureHandler(TEXT("failed because it was called on a client"));
            }
        }
        else
        {
            FailureHandler(TEXT("failed because there is no team info spawned yet (called too early, before the experience was ready)"));
        }
    }
    else
    {
        FailureHandler(TEXT("failed because it was passed an unknown team id"));
    }
}

int32 UUR_TeamSubsystem::GetTeamTagStackCount(int32 TeamId, FGameplayTag Tag) const
{
    if (const FGameTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
    {
        const int32 PublicStackCount = (Entry->PublicInfo != nullptr) ? Entry->PublicInfo->TeamTags.GetStackCount(Tag) : 0;
        const int32 PrivateStackCount = (Entry->PrivateInfo != nullptr) ? Entry->PrivateInfo->TeamTags.GetStackCount(Tag) : 0;
        return PublicStackCount + PrivateStackCount;
    }
    else
    {
        UE_LOG(LogGameTeams, Verbose, TEXT("GetTeamTagStackCount(TeamId: %d, Tag: %s) failed because it was passed an unknown team id"), TeamId, *Tag.ToString());
        return 0;
    }
}

bool UUR_TeamSubsystem::TeamHasTag(int32 TeamId, FGameplayTag Tag) const
{
    return GetTeamTagStackCount(TeamId, Tag) > 0;
}

bool UUR_TeamSubsystem::DoesTeamExist(int32 TeamId) const
{
    return TeamMap.Contains(TeamId);
}

TArray<int32> UUR_TeamSubsystem::GetTeamIDs() const
{
    TArray<int32> Result;
    TeamMap.GenerateKeyArray(Result);
    Result.Sort();
    return Result;
}

bool UUR_TeamSubsystem::CanCauseDamage(const UObject* Instigator, const UObject* Target, bool bAllowDamageToSelf) const
{
    if (bAllowDamageToSelf)
    {
        if ((Instigator == Target) || (FindPlayerStateFromActor(Cast<AActor>(Instigator)) == FindPlayerStateFromActor(Cast<AActor>(Target))))
        {
            return true;
        }
    }

    int32 InstigatorTeamId;
    int32 TargetTeamId;
    const EGameTeamComparison Relationship = CompareTeams(Instigator, Target, /*out*/ InstigatorTeamId, /*out*/ TargetTeamId);
    if (Relationship == EGameTeamComparison::DifferentTeams)
    {
        return true;
    }
    else if ((Relationship == EGameTeamComparison::InvalidArgument) && (InstigatorTeamId != INDEX_NONE))
    {
        // Allow damaging non-team actors for now, as long as they have an ability system component
        //@TODO: This is temporary until the target practice dummy has a team assignment
        return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Cast<const AActor>(Target)) != nullptr;
    }

    return false;
}

UUR_TeamDisplayAsset* UUR_TeamSubsystem::GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId)
{
    // Currently ignoring ViewerTeamId

    if (FGameTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
    {
        return Entry->DisplayAsset;
    }

    return nullptr;
}

UUR_TeamDisplayAsset* UUR_TeamSubsystem::GetEffectiveTeamDisplayAsset(int32 TeamId, UObject* ViewerTeamAgent)
{
    return GetTeamDisplayAsset(TeamId, FindTeamFromObject(ViewerTeamAgent));
}

void UUR_TeamSubsystem::NotifyTeamDisplayAssetModified(UUR_TeamDisplayAsset* /*ModifiedAsset*/)
{
    // Broadcasting to all observers when a display asset is edited right now, instead of only the edited one
    for (const auto& KVP : TeamMap)
    {
        //const int32 TeamId = KVP.Key;
        const FGameTeamTrackingInfo& TrackingInfo = KVP.Value;

        TrackingInfo.OnTeamDisplayAssetChanged.Broadcast(TrackingInfo.DisplayAsset);
    }
}

FOnGameTeamDisplayAssetChangedDelegate& UUR_TeamSubsystem::GetTeamDisplayAssetChangedDelegate(int32 TeamId)
{
    return TeamMap.FindOrAdd(TeamId).OnTeamDisplayAssetChanged;
}
