// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_LocalPlayer.h"

#include <AudioMixerBlueprintLibrary.h>
#include <Engine/World.h>

#include "UR_MessageHistory.h"
#include "Settings/UR_SettingsLocal.h"
#include "Settings/UR_SettingsShared.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_LocalPlayer)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_LocalPlayer::UUR_LocalPlayer()
    : Super()
{
    // no need to create a history for the CDO
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        MessageHistory = CreateDefaultSubobject<UUR_MessageHistory>(TEXT("MessageHistory"));
    }

    PlayerName = FString::Printf(TEXT("OTPlayer-%d"), FMath::RandRange(10, 9999));
}

void UUR_LocalPlayer::PostInitProperties()
{
    Super::PostInitProperties();

    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        if (UUR_SettingsLocal* LocalSettings = GetLocalSettings())
        {
            LocalSettings->OnAudioOutputDeviceChanged.AddUObject(this, &ThisClass::OnAudioOutputDeviceChanged);
        }
    }
}

void UUR_LocalPlayer::SwitchController(APlayerController* PC)
{
    Super::SwitchController(PC);

    OnPlayerControllerChanged(PC);
}

bool UUR_LocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
    return Super::SpawnPlayActor(URL, OutError, InWorld);
}

void UUR_LocalPlayer::InitOnlineSession()
{
    OnPlayerControllerChanged(PlayerController);

    Super::InitOnlineSession();
}

void UUR_LocalPlayer::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    IUR_TeamAgentInterface::SetGenericTeamId(NewTeamID);
}

FGenericTeamId UUR_LocalPlayer::GetGenericTeamId() const
{
    return IUR_TeamAgentInterface::GetGenericTeamId();
}

FOnGameTeamIndexChangedDelegate* UUR_LocalPlayer::GetOnTeamIndexChangedDelegate()
{
    return IUR_TeamAgentInterface::GetOnTeamIndexChangedDelegate();
}

UUR_SettingsLocal* UUR_LocalPlayer::GetLocalSettings() const
{
   // return Cast<UUR_SettingsLocal>(GEngine->GetGameUserSettings());
    return UUR_SettingsLocal::Get();
}

UUR_SettingsShared* UUR_LocalPlayer::GetSharedSettings() const
{
    if (!SharedSettings)
    {
        // On PC it's okay to use the sync load because it only checks the disk
        // This could use a platform tag to check for proper save support instead
        bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;

        if (bCanLoadBeforeLogin)
        {
            SharedSettings = UUR_SettingsShared::LoadOrCreateSettings(this);
        }
        else
        {
            // We need to wait for user login to get the real settings so return temp ones
            SharedSettings = UUR_SettingsShared::CreateTemporarySettings(this);
        }
    }

    return SharedSettings;
}

void UUR_LocalPlayer::LoadSharedSettingsFromDisk(bool bForceLoad)
{
    FUniqueNetIdRepl CurrentNetId = GetCachedUniqueNetId();
    if (!bForceLoad && SharedSettings && CurrentNetId == NetIdForSharedSettings)
    {
        // Already loaded once, don't reload
        return;
    }

    ensure(UUR_SettingsShared::AsyncLoadOrCreateSettings(this, UUR_SettingsShared::FOnSettingsLoadedEvent::CreateUObject(this, &UUR_LocalPlayer::OnSharedSettingsLoaded)));
}

void UUR_LocalPlayer::OnSharedSettingsLoaded(UUR_SettingsShared* LoadedOrCreatedSettings)
{
    // The settings are applied before it gets here
    if (ensure(LoadedOrCreatedSettings))
    {
        // This will replace the temporary or previously loaded object which will GC out normally
        SharedSettings = LoadedOrCreatedSettings;

        NetIdForSharedSettings = GetCachedUniqueNetId();
    }
}

void UUR_LocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
    FOnCompletedDeviceSwap DevicesSwappedCallback;
    DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
    UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

void UUR_LocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
    if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
    {
        UE_LOG(LogAudio, Error, TEXT("Audio Device Swap Failure!"));
    }
}

void UUR_LocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
    // Stop listening for changes from the old controller
    FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
    if (IUR_TeamAgentInterface* ControllerAsTeamProvider = Cast<IUR_TeamAgentInterface>(LastBoundPC.Get()))
    {
        OldTeamID = ControllerAsTeamProvider->GetGenericTeamId();
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
    }

    // Grab the current team ID and listen for future changes
    FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
    if (IUR_TeamAgentInterface* ControllerAsTeamProvider = Cast<IUR_TeamAgentInterface>(NewController))
    {
        NewTeamID = ControllerAsTeamProvider->GetGenericTeamId();
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
        LastBoundPC = NewController;
    }

    ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
}

void UUR_LocalPlayer::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
    ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

FString UUR_LocalPlayer::GetNickname() const
{
#if WITH_EDITOR
    if (GetWorld() && GetWorld()->IsPlayInEditor())
    {
        switch (GetWorld()->GetNetMode())
        {
            case NM_DedicatedServer:
            case NM_ListenServer:
            {
                return "Server";
            }
            case NM_Client:
            {
                if (GPlayInEditorID == 1)
                {
                    return PlayerName;
                }
                else
                {
                    return FString::Printf(TEXT("Client %d"), static_cast<int>(GPlayInEditorID));
                }
            }

            case NM_Standalone:
            {
                return PlayerName;
            }
            default:
            {
                return PlayerName;
            }
        }
    }
#endif WITH_EDITOR

    return PlayerName;
}
