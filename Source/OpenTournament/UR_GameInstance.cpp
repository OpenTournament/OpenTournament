// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameInstance.h"

#include <MoviePlayer.h>
#include <Misc/Paths.h>

#include "Slate/SUR_LoadingScreenWidget.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_GameInstance::Init()
{
    Super::Init();

    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UUR_GameInstance::BeginLoadingScreen);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UUR_GameInstance::EndLoadingScreen);
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
