// Copyright (c) Open Tournament Project, All Rights Reserved.


#include "UR_GameplayMessageDisplay.h"
#include "GameFramework/GameplayMessageSubsystem.h"

void UUR_GameplayMessageDisplay::NativeOnActivated()
{
    Super::NativeOnActivated();

    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());

    for(const auto Tag : GameplayMessagesToListenFor)
    {
        ListenerHandles.Add(MessageSystem.RegisterListener(Tag, this, &UUR_GameplayMessageDisplay::MessageReceived));
    }
}

void UUR_GameplayMessageDisplay::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();
    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());

    for(const auto ListenerHandle : ListenerHandles)
        MessageSystem.UnregisterListener(ListenerHandle);
}
