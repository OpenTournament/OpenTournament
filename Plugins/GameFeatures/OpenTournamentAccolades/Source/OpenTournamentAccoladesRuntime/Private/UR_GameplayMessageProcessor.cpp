// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayMessageProcessor.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayMessageProcessor)

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_GameplayMessageProcessor::BeginPlay()
{
    Super::BeginPlay();

    StartListening();
}

void UUR_GameplayMessageProcessor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    StopListening();

    // Remove any listener handles
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    for (FGameplayMessageListenerHandle& Handle : ListenerHandles)
    {
        MessageSubsystem.UnregisterListener(Handle);
    }
    ListenerHandles.Empty();
}

void UUR_GameplayMessageProcessor::StartListening()
{
    // ...
}

void UUR_GameplayMessageProcessor::StopListening()
{
    // ...
}

void UUR_GameplayMessageProcessor::AddListenerHandle(FGameplayMessageListenerHandle&& Handle)
{
    ListenerHandles.Add(MoveTemp(Handle));
}

double UUR_GameplayMessageProcessor::GetServerTime() const
{
    if (const AGameStateBase* GameState = GetWorld()->GetGameState())
    {
        return GameState->GetServerWorldTimeSeconds();
    }
    else
    {
        return 0.0;
    }
}
