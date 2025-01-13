// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonUserBasicPresence.generated.h"

class UCommonSessionSubsystem;
enum class ECommonSessionInformationState : uint8;

//////////////////////////////////////////////////////////////////////
// UCommonUserBasicPresence

/**
 * This subsystem plugs into the session subsystem and pushes its information to the presence interface.
 * It is not intended to be a full featured rich presence implementation, but can be used as a proof-of-concept
 * for pushing information from the session subsystem to the presence system
 */
UCLASS(BlueprintType, Config = Engine)
class COMMONUSER_API UCommonUserBasicPresence : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UCommonUserBasicPresence();


	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;

	/** False is a general purpose killswitch to stop this class from pushing presence*/
	UPROPERTY(Config)
	bool bEnableSessionsBasedPresence = false;

	/** Maps the presence status "In-game" to a backend key*/
	UPROPERTY(Config)
	FString PresenceStatusInGame;

	/** Maps the presence status "Main Menu" to a backend key*/
	UPROPERTY(Config)
	FString PresenceStatusMainMenu;

	/** Maps the presence status "Matchmaking" to a backend key*/
	UPROPERTY(Config)
	FString PresenceStatusMatchmaking;

	/** Maps the "Game Mode" rich presence entry to a backend key*/
	UPROPERTY(Config)
	FString PresenceKeyGameMode;

	/** Maps the "Map Name" rich presence entry to a backend key*/
	UPROPERTY(Config)
	FString PresenceKeyMapName;

	void OnNotifySessionInformationChanged(ECommonSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName);
	FString SessionStateToBackendKey(ECommonSessionInformationState SessionStatus);
};