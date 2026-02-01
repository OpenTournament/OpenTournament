// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_BotCheats.h"

#include "Engine/World.h"
#include "GameFramework/CheatManagerDefines.h"

#include "GameModes/UR_BotCreationComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_BotCheats)

/////////////////////////////////////////////////////////////////////////////////////////////////
// UUR_BotCheats

UUR_BotCheats::UUR_BotCheats()
{
#if WITH_SERVER_CODE && UE_WITH_CHEAT_MANAGER
    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        UCheatManager::RegisterForOnCheatManagerCreated
        (FOnCheatManagerCreated::FDelegate::CreateLambda
            (
                [](UCheatManager* CheatManager)
                {
                    CheatManager->AddCheatManagerExtension(NewObject<ThisClass>(CheatManager));
                }));
    }
#endif
}

void UUR_BotCheats::AddPlayerBot()
{
#if WITH_SERVER_CODE && UE_WITH_CHEAT_MANAGER
    if (UUR_BotCreationComponent* BotComponent = GetBotComponent())
    {
        BotComponent->Cheat_AddBot();
    }
#endif
}

void UUR_BotCheats::RemovePlayerBot()
{
#if WITH_SERVER_CODE && UE_WITH_CHEAT_MANAGER
    if (UUR_BotCreationComponent* BotComponent = GetBotComponent())
    {
        BotComponent->Cheat_RemoveBot();
    }
#endif
}

UUR_BotCreationComponent* UUR_BotCheats::GetBotComponent() const
{
    if (UWorld* World = GetWorld())
    {
        if (AGameStateBase* GameState = World->GetGameState())
        {
            return GameState->FindComponentByClass<UUR_BotCreationComponent>();
        }
    }

    return nullptr;
}
