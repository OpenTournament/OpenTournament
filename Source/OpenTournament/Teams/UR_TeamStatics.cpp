// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TeamStatics.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

#include "UR_LogChannels.h"
#include "Teams/UR_TeamDisplayAsset.h"
#include "Teams/UR_TeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TeamStatics)

/////////////////////////////////////////////////////////////////////////////////////////////////

class UTexture;

//////////////////////////////////////////////////////////////////////

void UUR_TeamStatics::FindTeamFromObject(const UObject* Agent, bool& bIsPartOfTeam, int32& TeamId, UUR_TeamDisplayAsset*& DisplayAsset, bool bLogIfNotSet)
{
	bIsPartOfTeam = false;
	TeamId = INDEX_NONE;
	DisplayAsset = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(Agent, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UUR_TeamSubsystem* TeamSubsystem = World->GetSubsystem<UUR_TeamSubsystem>())
		{
			TeamId = TeamSubsystem->FindTeamFromObject(Agent);
			if (TeamId != INDEX_NONE)
			{
				bIsPartOfTeam = true;

				DisplayAsset = TeamSubsystem->GetTeamDisplayAsset(TeamId, INDEX_NONE);

				if ((DisplayAsset == nullptr) && bLogIfNotSet)
				{
					UE_LOG(LogGameTeams, Log, TEXT("FindTeamFromObject(%s) called too early (found team %d but no display asset set yet"), *GetPathNameSafe(Agent), TeamId);
				}
			}
		}
		else
		{
			UE_LOG(LogGameTeams, Error, TEXT("FindTeamFromObject(%s) failed: Team subsystem does not exist yet"), *GetPathNameSafe(Agent));
		}
	}
}

UUR_TeamDisplayAsset* UUR_TeamStatics::GetTeamDisplayAsset(const UObject* WorldContextObject, int32 TeamId)
{
	UUR_TeamDisplayAsset* Result = nullptr;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UUR_TeamSubsystem* TeamSubsystem = World->GetSubsystem<UUR_TeamSubsystem>())
		{
			return TeamSubsystem->GetTeamDisplayAsset(TeamId, INDEX_NONE);
		}
	}
	return Result;
}

float UUR_TeamStatics::GetTeamScalarWithFallback(UUR_TeamDisplayAsset* DisplayAsset, FName ParameterName, float DefaultValue)
{
	if (DisplayAsset)
	{
		if (float* Value = DisplayAsset->ScalarParameters.Find(ParameterName))
		{
			return *Value;
		}
	}
	return DefaultValue;
}

FLinearColor UUR_TeamStatics::GetTeamColorWithFallback(UUR_TeamDisplayAsset* DisplayAsset, FName ParameterName, FLinearColor DefaultValue)
{
	if (DisplayAsset)
	{
		if (FLinearColor* Color = DisplayAsset->ColorParameters.Find(ParameterName))
		{
			return *Color;
		}
	}
	return DefaultValue;
}

UTexture* UUR_TeamStatics::GetTeamTextureWithFallback(UUR_TeamDisplayAsset* DisplayAsset, FName ParameterName, UTexture* DefaultValue)
{
	if (DisplayAsset)
	{
		if (TObjectPtr<UTexture>* Texture = DisplayAsset->TextureParameters.Find(ParameterName))
		{
			return *Texture;
		}
	}
	return DefaultValue;
}

