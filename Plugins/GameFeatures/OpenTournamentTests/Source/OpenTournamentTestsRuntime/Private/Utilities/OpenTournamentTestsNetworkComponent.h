// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#define ENABLE_OpenTournamentTests_NETWORK_TEST WITH_EDITOR && (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)

#if ENABLE_OpenTournamentTests_NETWORK_TEST

#include "Character/UR_Character.h"
#include "Commands/TestCommandBuilder.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "GameModes/UR_ExperienceManagerComponent.h"
#include "GameModes/UR_GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "OpenTournamentTestsActorTestHelper.h"
#include "UnrealEdGlobals.h"

class FOpenTournamentTestsActorInputTestHelper;

/**
 * Struct which handles the PIE session's world and player information.
 * Both the server and client will keep a reference to their own respective state which will be tested against.
 *
 * @see FPIENetworkComponentState
 */
template <typename NetworkActorType>
struct FOpenTournamentTestsNetworkState
{
	/** Reference to the session's world. */
	UWorld* World = nullptr;

	/** Reference to the local player in the world. */
	TUniquePtr<NetworkActorType> LocalPlayer{ nullptr };

	/** Reference to the network player in the world. */
	TUniquePtr<NetworkActorType> NetworkPlayer{ nullptr };
};

/**
 * Component which acts as a latent command manager for networked PIE sessions.
 * The component stores a FOpenTournamentTestsNetworkState object for each server and client PIE session and uses the state to access player or world information within the latent command.
 *
 * @see FPIENetworkComponent
 */
template <typename NetworkActorType = FOpenTournamentTestsActorTestHelper>
class FOpenTournamentTestsNetworkComponent
{
public:
	/**
	 * Construct the Network Component.
	 *
	 * @param InTestRunner - Pointer to the TestRunner used for test reporting.
	 * @param InCommandBuilder - Reference to the latent command manager.
	 *
	 * @see FPIENetworkComponent
	 */
	FOpenTournamentTestsNetworkComponent(FAutomationTestBase* InTestRunner, FTestCommandBuilder& InCommandBuilder, bool bIsInitializing)
		: TestRunner(InTestRunner), CommandBuilder(&InCommandBuilder)
	{
		static_assert(std::is_convertible_v<NetworkActorType*, FOpenTournamentTestsActorTestHelper*>, "NetworkActorType must derive from FOpenTournamentTestsActorTestHelper");

		// Check if the framework is initializing to avoid premature creation of the network states
		if (bIsInitializing)
		{
			return;
		}

		ServerState = MakeUnique<FOpenTournamentTestsNetworkState<NetworkActorType>>();
		ClientState = MakeUnique<FOpenTournamentTestsNetworkState<NetworkActorType>>();
	}

	/**
	 * Initializes the network by starting PIE for both the server and client and handles setting up the network connections.
	 *
	 * @return a reference to this
	 *
	 * @note Must be called prior to use in order to initialize the client and server states
	 */
	FOpenTournamentTestsNetworkComponent& Start()
	{
		checkf(!bIsRunning, TEXT("Network Component cannot be started when already running."));

		CommandBuilder->Do(TEXT("Starting Network Component server"), [this] { return StartPie(); })
			.Until(TEXT("Collect PIE Worlds"), [this]() { return CollectPieWorlds(); }, LoadingScreenTimeout)
			.Until(TEXT("Await connections"), [this]() { return AwaitConnections(); })
			.Then(TEXT("Server running"), [this]() { bIsRunning = true; })
			.OnTearDown(TEXT("TearDown Network Component"), [this]() { TearDown(); });

		return *this;
	}

	/**
	 * Add a latent command to be executed on the server.
	 *
	 * @param Description - Description of the latent command
	 * @param Action - Latent command to be queued
	 *
	 * @return a reference to this
	 */
	FOpenTournamentTestsNetworkComponent& ThenServer(const TCHAR* Description, TFunction<void(FOpenTournamentTestsNetworkState<NetworkActorType>&)> Action)
	{
		CommandBuilder->Do(Description, [this, Action] { Action(*ServerState); });
		return *this;
	}

	/**
	 * Add a latent command to be executed on the client.
	 *
	 * @param Description - Description of the latent command
	 * @param Action - Latent command to be queued
	 *
	 * @return a reference to this
	 */
	FOpenTournamentTestsNetworkComponent& ThenClient(const TCHAR* Description, TFunction<void(FOpenTournamentTestsNetworkState<NetworkActorType>&)> Action)
	{
		CommandBuilder->Do(Description, [this, Action]() { Action(*ClientState); });
		return *this;
	}

	/**
	 * Add a latent command to be executed until completion or timed out on the server.
	 *
	 * @param Description - Description of the latent command
	 * @param Action - Latent command to be queued
	 * @param Timeout - Duration that the latent command can execute before reporting an error
	 *
	 * @return a reference to this
	 */
	FOpenTournamentTestsNetworkComponent& UntilServer(const TCHAR* Description, TFunction<bool(FOpenTournamentTestsNetworkState<NetworkActorType>&)> Query, FTimespan Timeout = FTimespan::FromSeconds(10))
	{
		CommandBuilder->Until(Description, [this, Query]() { return Query(*ServerState); }, Timeout);
		return *this;
	}

	/**
	 * Add a latent command to be executed until completion or timed out on the client.
	 *
	 * @param Description - Description of the latent command
	 * @param Action - Latent command to be queued
	 * @param Timeout - Duration that the latent command can execute for before reporting an error
	 *
	 * @return a reference to this
	 */
	FOpenTournamentTestsNetworkComponent& UntilClient(const TCHAR* Description, TFunction<bool(FOpenTournamentTestsNetworkState<NetworkActorType>&)> Query, FTimespan Timeout = FTimespan::FromSeconds(10))
	{
		CommandBuilder->Until(Description, [this, Query]() { return Query(*ClientState); }, Timeout);
		return *this;
	}

	/**
	 * Finds the local player on the server and waits until the player is fully spawned in.
	 *
	 * @return a reference to this
	 */
	FOpenTournamentTestsNetworkComponent& PrepareAndWaitForServerPlayerSpawn()
	{
		WaitForLocalPlayerSpawn(*ServerState);
		return *this;
	}

	/**
	 * Finds the local player on the client and waits until the player is fully spawned in.
	 *
	 * @return a reference to this
	 */
	FOpenTournamentTestsNetworkComponent& PrepareAndWaitForClientPlayerSpawn()
	{
		WaitForLocalPlayerSpawn(*ClientState);
		return *this;
	}

	/**
	 * Finds the connected client player on the server.
	 *
	 * @return a reference to this
	 */
	FOpenTournamentTestsNetworkComponent& FetchConnectedPlayerOnServer()
	{
		CommandBuilder->Do(TEXT("Fetch the connected client pawn"), [this]() { FetchConnectedPlayer(*ServerState); });
		return *this;
	}

	/**
	 * Finds the connected server player on the client.
	 *
	 * @return a reference to this
	 */
	FOpenTournamentTestsNetworkComponent& FetchConnectedPlayerOnClient()
	{
		CommandBuilder->Do(TEXT("Fetch the connected server pawn"), [this]() { FetchConnectedPlayer(*ClientState); });
		return *this;
	}

private:
	/** Setup settings for a network session and start PIE sessions for both the server and client with the network settings applied. */
	void StartPie()
	{
		if (bIsRunning)
		{
			TestRunner->AddError(TEXT("Network Component cannot be started when already running."));
			return;
		}

		ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>();
		PlaySettings->SetPlayNetMode(PIE_ListenServer);

		// The listen server counts as a client, so we need to add one more to get a real client as well
		PlaySettings->SetPlayNumberOfClients(ClientCount + 1);

		PlaySettings->bLaunchSeparateServer = false;
		PlaySettings->GameGetsMouseControl = false;
		PlaySettings->SetRunUnderOneProcess(true);

		FLevelEditorModule& LevelEditorModule = FModuleManager::Get().GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

		FRequestPlaySessionParams SessionParams;
		SessionParams.WorldType = EPlaySessionWorldType::PlayInEditor;
		SessionParams.DestinationSlateViewport = LevelEditorModule.GetFirstActiveViewport();
		SessionParams.EditorPlaySettings = PlaySettings;
		SessionParams.GameModeOverride = AUR_GameMode::StaticClass();

		GUnrealEd->RequestPlaySession(SessionParams);
		GUnrealEd->StartQueuedPlaySessionRequest();
	}

	/**
	 * Fetch all of the worlds that will be used for the networked session.
	 *
	 * @return true if an error was encountered or if the PIE sessions were created with the correct network settings, false otherwise.
	 *
	 * @note Method is expected to be used within the `Until` latent command to then wait until the worlds are ready for use.
	 */
	bool CollectPieWorlds()
	{
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		UWorld* ServerWorld = nullptr;
		UWorld* ClientWorld = nullptr;

		for (const FWorldContext& WorldContext : WorldContexts)
		{
			if (WorldContext.WorldType != EWorldType::PIE)
			{
				continue;
			}

			UWorld* World = WorldContext.World();
			if (!IsValid(World) || !IsValid(World->GetNetDriver()))
			{
				continue;
			}

			if (World->GetNetDriver()->IsServer())
			{
				ServerWorld = World;
			}
			else if (ClientWorld == nullptr)
			{
				ClientWorld = World;
			}
			else
			{
				TestRunner->AddError(TEXT("Found extra PIE session that could impact test behavior."));
				return true;
			}
		}
		if (ServerWorld == nullptr || ClientWorld == nullptr)
		{
			return false;
		}

		const int32 ClientLocalPort = ClientWorld->GetNetDriver()->GetLocalAddr()->GetPort();
		TObjectPtr<UNetConnection>* ServerConnection = ServerWorld->GetNetDriver()->ClientConnections.FindByPredicate([ClientLocalPort](UNetConnection* ClientConnection) {
			return ClientConnection->GetRemoteAddr()->GetPort() == ClientLocalPort;
		});

		if (ServerConnection == nullptr)
		{
			TestRunner->AddError(TEXT("Connection to server was not successful."));
			return true;
		}

		ServerState->World = ServerWorld;
		ClientState->World = ClientWorld;

		return true;
	}

	/**
	 * Go through all of the client connections to make sure they are connected and ready.
	 *
	 * @return true if an error was encountered or if the connections all have a valid controller, false otherwise.
	 *
	 * @note Method is expected to be used within the `Until` latent command to then wait until the worlds are ready for use.
	 */
	bool AwaitConnections()
	{
		if (!IsValid(ServerState->World))
		{
			TestRunner->AddError(TEXT("Failed to initialize Network Component."));
			return true;
		}
		if (ServerState->World->GetNetDriver()->ClientConnections.Num() != ClientCount)
		{
			return false;
		}

		for (UNetConnection* ClientConnection : ServerState->World->GetNetDriver()->ClientConnections)
		{
			if (ClientConnection->ViewTarget == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	/** Tear down the PIE sessions used by the Network Component. */
	void TearDown()
	{
		GUnrealEd->RequestEndPlayMap();
		bIsRunning = false;
	}

	/**
	 * Check to make sure that the specified world has fully loaded.
	 *
	 * @param World - Pointer to the World instance.
	 *
	 * @return true if the world has been loaded, false otherwise.
	 *
	 * @note Method is expected to be used within the `Until` latent command to then wait until the world has loaded.
	 */
	bool HasWorldLoaded(const UWorld* World)
	{
		if (!IsValid(World))
		{
			return false;
		}

		AGameStateBase* GameState = World->GetGameState();
		if (GameState == nullptr)
		{
			return false;
		}

		UUR_ExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UUR_ExperienceManagerComponent>();
		if (ExperienceComponent == nullptr)
		{
			return false;
		}

		return ExperienceComponent->IsExperienceLoaded();
	}

	/**
	* Check to make sure that the specified world has a valid local player.
	*
	* @param World - Pointer to the World instance where the local player will be fetched from.
	*
	* @return true if the Actor associated with the local player has been loaded, false otherwise.
	*
	* @note Method is expected to be used within the `Until` latent command to then wait until the Actor has been loaded into the world.
	*/
	bool HasValidLocalPlayer(const UWorld* World)
	{
		if (!IsValid(World))
		{
			return false;
		}

		ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();
		if (!IsValid(LocalPlayer))
		{
			return false;
		}

		APlayerController* PlayerController = LocalPlayer->GetPlayerController(nullptr);
		if (PlayerController == nullptr)
		{
			return false;
		}

		return IsValid(PlayerController->GetPawn());
	}

	/**
	* Get the local player for the current World.
	*
	* @param NetworkState - Reference to the network state which will store the local Player
	* @param OutError - Reference to a string for error capturing
	*
	* @return true if the local player was found and saved within the provided NetworkState, otherwise false and the error will be provided within OutError
	*/
	bool FetchLocalPlayer(FOpenTournamentTestsNetworkState<NetworkActorType>& NetworkState, FString& OutError)
	{
		if (!IsValid(NetworkState.World))
		{
			OutError = TEXT("World for the provided network state is invalid and cannot be used to fetch a local player.");
			return false;
		}

		ULocalPlayer* LocalPlayer = NetworkState.World->GetFirstLocalPlayerFromController();
		if (!IsValid(LocalPlayer))
		{
			OutError = TEXT("Local Player was not found for the current network state.");
			return false;
		}

		APlayerController* PlayerController = LocalPlayer->GetPlayerController(nullptr);
		if (PlayerController == nullptr)
		{
			OutError = TEXT("Player Controller is not valid for the local player within the provided network state.");
			return false;
		}

		NetworkState.LocalPlayer = MakeUnique<NetworkActorType>(PlayerController->GetPawn());
		return true;
	}

	/**
	* Get the connected player for the current world
	*
	* @param NetworkState - Reference to the network state of the Local Player
	*/
	void FetchConnectedPlayer(FOpenTournamentTestsNetworkState<NetworkActorType>& NetworkState)
	{
		const AUR_Character* RegisteredPlayer = NetworkState.LocalPlayer->GetGameCharacter();

		TArray<AActor*> Pawns;
		UGameplayStatics::GetAllActorsOfClass(NetworkState.World, AUR_Character::StaticClass(), Pawns);
		for (AActor* Pawn : Pawns)
		{
			AUR_Character* Player = Cast<AUR_Character>(Pawn);
			if (Player == nullptr)
			{
				continue;
			}
			if (Player != RegisteredPlayer && !Player->GetName().Equals(RegisteredPlayer->GetName()))
			{
				NetworkState.NetworkPlayer = MakeUnique<NetworkActorType>(Player);
				return;
			}
		}
	}

	/**
	 * Wait for the local player to be loaded and fully spawned in the level.
	 *
	 * @param NetworkState - Reference to the network state of the Local Player
	 */
	void WaitForLocalPlayerSpawn(FOpenTournamentTestsNetworkState<NetworkActorType>& NetworkState)
	{
		CommandBuilder->StartWhen(TEXT("Check if world is loaded"), [this, &NetworkState]() { return HasWorldLoaded(NetworkState.World); }, LoadingScreenTimeout)
			.Until(TEXT("Check if player is loaded"), [this, &NetworkState]() { return HasValidLocalPlayer(NetworkState.World); })
			.Then(TEXT("Prepare player"), [this, &NetworkState]() {
				FString Error;
				const bool bWasPlayerFound = FetchLocalPlayer(NetworkState, Error);
				TestRunner->AddErrorIfFalse(bWasPlayerFound, Error);
			})
			.Until(TEXT("Wait until player is fully spawned"), [this, &NetworkState]() { return NetworkState.LocalPlayer->IsPawnFullySpawned(); });
	}

	/** Running state of the Network Component. */
	bool bIsRunning = false;

	/**
	* Number of clients the Network Component will initialize for.
	*
	* @note This does not include the listen server as part of the number of actual clients.
	*/
	const int32 ClientCount{ 1 };

	/** Duration to allow for the Game loading screen. */
	const FTimespan LoadingScreenTimeout = FTimespan::FromSeconds(30);

	/** Server's network state. */
	TUniquePtr<FOpenTournamentTestsNetworkState<NetworkActorType>> ServerState{ nullptr };

	/** Client's network state. */
	TUniquePtr<FOpenTournamentTestsNetworkState<NetworkActorType>> ClientState{ nullptr };

	/** Pointer to the current test. */
	FAutomationTestBase* TestRunner{ nullptr };

	/** Pointer to the latent command manager. */
	FTestCommandBuilder* CommandBuilder{ nullptr };
};

#endif // ENABLE_OpenTournamentTests_NETWORK_TEST
