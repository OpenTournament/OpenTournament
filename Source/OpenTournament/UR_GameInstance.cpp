// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameInstance.h"

#include <CommonSessionSubsystem.h>
#include <CommonUserSubsystem.h>
#include <MoviePlayer.h>
#include <Components/GameFrameworkComponentManager.h>
#include <Misc/Paths.h>

#include "UR_GameplayTags.h"
#include "UR_LocalPlayer.h"
#include "UR_PlayerController.h"
#include "Slate/SUR_LoadingScreenWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameInstance)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameInstance::UUR_GameInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UUR_GameInstance::Init()
{
    Super::Init();

    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UUR_GameInstance::BeginLoadingScreen);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UUR_GameInstance::EndLoadingScreen);

    // Register our custom init states
    UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

    if (ensure(ComponentManager))
    {
        ComponentManager->RegisterInitState(URGameplayTags::InitState_Spawned, false, FGameplayTag());
        ComponentManager->RegisterInitState(URGameplayTags::InitState_DataAvailable, false, URGameplayTags::InitState_Spawned);
        ComponentManager->RegisterInitState(URGameplayTags::InitState_DataInitialized, false, URGameplayTags::InitState_DataAvailable);
        ComponentManager->RegisterInitState(URGameplayTags::InitState_GameplayReady, false, URGameplayTags::InitState_DataInitialized);
    }
}

void UUR_GameInstance::Shutdown()
{
    if (UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
    {
        SessionSubsystem->OnPreClientTravelEvent.RemoveAll(this);
    }

    Super::Shutdown();
}

AUR_PlayerController* UUR_GameInstance::GetPrimaryPlayerController() const
{
    return Cast<AUR_PlayerController>(Super::GetPrimaryPlayerController(false));
}

bool UUR_GameInstance::CanJoinRequestedSession() const
{
    // Temporary first pass:  Always return true
    // This will be fleshed out to check the player's state
    if (!Super::CanJoinRequestedSession())
    {
        return false;
    }
    return true;
}

void UUR_GameInstance::HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
    Super::HandlerUserInitialized(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);

    // If login succeeded, tell the local player to load their settings
    if (bSuccess && ensure(UserInfo))
    {
        UUR_LocalPlayer* LocalPlayer = Cast<UUR_LocalPlayer>(GetLocalPlayerByIndex(UserInfo->LocalPlayerIndex));

        // There will not be a local player attached to the dedicated server user
        if (LocalPlayer)
        {
            //LocalPlayer->LoadSharedSettingsFromDisk();
        }
    }
}


void UUR_GameInstance::FTest_AddLocalPlayer(int32 ControllerId)
{
#if !UE_BUILD_SHIPPING
    FString OutError{ };
    ULocalPlayer* NewPlayer = CreateLocalPlayer(0, OutError, true);
#endif
}

void UUR_GameInstance::BeginLoadingScreen(const FString& InMapName)
{
    if (!IsRunningDedicatedServer())
    {
        FLoadingScreenAttributes LoadingScreen;
        LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
        LoadingScreen.WidgetLoadingScreen = SUR_LoadingScreenWidget::Create(FPaths::GetBaseFilename(InMapName));

#if !UE_BUILD_SHIPPING
        LoadingScreen.MinimumLoadingScreenDisplayTime = 2.f;
#endif

        GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
    }
}

void UUR_GameInstance::EndLoadingScreen(UWorld* InLoadedWorld)
{
}
