// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"

#include "CommonGameInstance.generated.h"

#define UE_API COMMONGAME_API

enum class ECommonUserAvailability : uint8;
enum class ECommonUserPrivilege : uint8;

class FText;
class UCommonUserInfo;
class UCommonSession_SearchResult;
struct FOnlineResultInformation;
class ULocalPlayer;
class USocialManager;
class UObject;
struct FFrame;
struct FGameplayTag;

UCLASS(MinimalAPI, Abstract, Config = Game)
class UCommonGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UE_API UCommonGameInstance(const FObjectInitializer& ObjectInitializer);
	
	/** Handles errors/warnings from CommonUser, can be overridden per game */
	UFUNCTION()
	UE_API virtual void HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message);

	UFUNCTION()
	UE_API virtual void HandlePrivilegeChanged(const UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability, ECommonUserAvailability NewAvailability);

	UFUNCTION()
	UE_API virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

	/** Call to reset user and session state, usually because a player has been disconnected */
	UE_API virtual void ResetUserAndSessionState();

	/**
	 * Requested Session Flow
	 *   Something requests the user to join a specific session (for example, a platform overlay via OnUserRequestedSession).
	 *   This request is handled in SetRequestedSession.
	 *   Check if we can join the requested session immediately (CanJoinRequestedSession).  If we can, join the requested session (JoinRequestedSession)
	 *   If not, cache the requested session and instruct the game to get into a state where the session can be joined (ResetGameAndJoinRequestedSession)
	 */
	/** Handles user accepting a session invite from an external source (for example, a platform overlay). Intended to be overridden per game. */
	UE_API virtual void OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult);

	/** Handles OSS request that the session be destroyed */
	UE_API virtual void OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);

	/** Get the requested session */
	UCommonSession_SearchResult* GetRequestedSession() const { return RequestedSession; }
	/** Set (or clear) the requested session. When this is set, the requested session flow begins. */
	UE_API virtual void SetRequestedSession(UCommonSession_SearchResult* InRequestedSession);
	/** Checks if the requested session can be joined. Can be overridden per game. */
	UE_API virtual bool CanJoinRequestedSession() const;
	/** Join the requested session */
	UE_API virtual void JoinRequestedSession();
	/** Get the game into a state to join the requested session */
	UE_API virtual void ResetGameAndJoinRequestedSession();
	
	UE_API virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	UE_API virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
	UE_API virtual void Init() override;
	UE_API virtual void ReturnToMainMenu() override;

private:
	/** This is the primary player*/
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;
	/** Session the player has requested to join */
	UPROPERTY()
	TObjectPtr<UCommonSession_SearchResult> RequestedSession;
};

#undef UE_API
