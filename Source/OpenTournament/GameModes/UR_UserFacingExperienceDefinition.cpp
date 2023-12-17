// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_UserFacingExperienceDefinition.h"

#include <CommonSessionSubsystem.h>
#include <Containers/UnrealString.h>
#include <UObject/NameTypes.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_UserFacingExperienceDefinition)

/////////////////////////////////////////////////////////////////////////////////////////////////

UCommonSession_HostSessionRequest* UUR_UserFacingExperienceDefinition::CreateHostingRequest() const
{
	const FString ExperienceName = ExperienceID.PrimaryAssetName.ToString();
	const FString UserFacingExperienceName = GetPrimaryAssetId().PrimaryAssetName.ToString();
	UCommonSession_HostSessionRequest* Result = NewObject<UCommonSession_HostSessionRequest>();
	Result->OnlineMode = ECommonSessionOnlineMode::Online;
	Result->bUseLobbies = true;
	Result->MapID = MapID;
	Result->ModeNameForAdvertisement = UserFacingExperienceName;
	Result->ExtraArgs = ExtraArgs;
	Result->ExtraArgs.Add(TEXT("Experience"), ExperienceName);
	Result->MaxPlayerCount = MaxPlayerCount;

    // @! TODO : Add Replays to string

	return Result;
}

