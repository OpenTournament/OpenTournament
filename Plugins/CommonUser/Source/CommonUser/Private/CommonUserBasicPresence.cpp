// Copyright Epic Games, Inc. All Rights Reserved.
#include "CommonUserBasicPresence.h"
#include "CommonSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "CommonUserTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonUserBasicPresence)


#if COMMONUSER_OSSV1
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlinePresenceInterface.h"
#else
#include "Online/OnlineServicesEngineUtils.h"
#include "Online/Presence.h"
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogUserBasicPresence, Log, All);
DEFINE_LOG_CATEGORY(LogUserBasicPresence);

UCommonUserBasicPresence::UCommonUserBasicPresence()
{

}

void UCommonUserBasicPresence::Initialize(FSubsystemCollectionBase& Collection)
{
	UCommonSessionSubsystem* CommonSession = Collection.InitializeDependency<UCommonSessionSubsystem>();
	if(ensure(CommonSession))
	{
		CommonSession->OnSessionInformationChangedEvent.AddUObject(this, &UCommonUserBasicPresence::OnNotifySessionInformationChanged);
	}
}

void UCommonUserBasicPresence::Deinitialize()
{

}

FString UCommonUserBasicPresence::SessionStateToBackendKey(ECommonSessionInformationState SessionStatus)
{
	switch (SessionStatus)
	{
	case ECommonSessionInformationState::OutOfGame:
		return PresenceStatusMainMenu;
		break;
	case ECommonSessionInformationState::Matchmaking:
		return PresenceStatusMatchmaking;
		break;
	case ECommonSessionInformationState::InGame:
		return PresenceStatusInGame;
		break;
	default:
		UE_LOG(LogUserBasicPresence, Error, TEXT("UCommonUserBasicPresence::SessionStateToBackendKey: Found unknown enum value %d"), (uint8)SessionStatus);
		return TEXT("Unknown");
		break;

	}
}

void UCommonUserBasicPresence::OnNotifySessionInformationChanged(ECommonSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName)
{
	if (bEnableSessionsBasedPresence && !GetGameInstance()->IsDedicatedServerInstance())
	{
		// trim the map name since its a URL
		FString MapNameTruncated = MapName;
		if (!MapNameTruncated.IsEmpty())
		{
			int LastIndexOfSlash = 0;
			MapNameTruncated.FindLastChar('/', LastIndexOfSlash);
			MapNameTruncated = MapNameTruncated.RightChop(LastIndexOfSlash + 1);
		}

#if COMMONUSER_OSSV1
		IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
		if(OnlineSub)
		{
			IOnlinePresencePtr Presence = OnlineSub->GetPresenceInterface();
			if(Presence)
			{
				FOnlineUserPresenceStatus UpdatedPresence;
				UpdatedPresence.State = EOnlinePresenceState::Online; // We'll only send the presence update if the user has a valid UniqueNetId, so we can assume they are Online
				UpdatedPresence.StatusStr = *SessionStateToBackendKey(SessionStatus);
				UpdatedPresence.Properties.Emplace(PresenceKeyGameMode, GameMode);
				UpdatedPresence.Properties.Emplace(PresenceKeyMapName, MapNameTruncated);

				for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
				{
					if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId() != nullptr)
					{
						Presence->SetPresence(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), UpdatedPresence);
					}
				}
			}
		}

#else

		UE::Online::IOnlineServicesPtr OnlineServices = UE::Online::GetServices(GetWorld());
		check(OnlineServices);
		UE::Online::IPresencePtr Presence = OnlineServices->GetPresenceInterface();
		if(Presence)
		{
			for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
			{
				if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsV2())
				{
					UE::Online::FPartialUpdatePresence::Params UpdateParams;
					UpdateParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
					UpdateParams.Mutations.StatusString.Emplace(*SessionStateToBackendKey(SessionStatus));
					UpdateParams.Mutations.UpdatedProperties.AddVariant(PresenceKeyGameMode, GameMode);
					UpdateParams.Mutations.UpdatedProperties.AddVariant(PresenceKeyMapName, MapNameTruncated);

					Presence->PartialUpdatePresence(MoveTemp(UpdateParams));
				}
			}

		}
#endif
	}
}
