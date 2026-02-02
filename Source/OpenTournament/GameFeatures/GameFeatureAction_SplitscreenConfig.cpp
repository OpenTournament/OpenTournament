// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameFeatureAction_SplitscreenConfig.h"

#include <Engine/GameInstance.h>
#include <Engine/GameViewportClient.h>

#include "GameFeatures/GameFeatureAction_WorldActionBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_SplitscreenConfig)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTGameFeatures"

/////////////////////////////////////////////////////////////////////////////////////////////////

TMap<FObjectKey, int32> UGameFeatureAction_SplitscreenConfig::GlobalDisableVotes;

/////////////////////////////////////////////////////////////////////////////////////////////////

void UGameFeatureAction_SplitscreenConfig::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
    Super::OnGameFeatureDeactivating(Context);

    for (int32 i = LocalDisableVotes.Num() - 1; i >= 0; i--)
    {
        FObjectKey ViewportKey = LocalDisableVotes[i];
        UGameViewportClient* GameViewportClient = Cast<UGameViewportClient>(ViewportKey.ResolveObjectPtr());
        const FWorldContext* WorldContext = GEngine->GetWorldContextFromGameViewport(GameViewportClient);
        if (GameViewportClient && WorldContext)
        {
            if (!Context.ShouldApplyToWorldContext(*WorldContext))
            {
                // Wrong context so ignore it, dead objects count as part of this context
                continue;
            }
        }

        int32& VoteCount = GlobalDisableVotes[ViewportKey];
        if (VoteCount <= 1)
        {
            GlobalDisableVotes.Remove(ViewportKey);

            if (GameViewportClient && WorldContext)
            {
                GameViewportClient->SetForceDisableSplitscreen(false);
            }
        }
        else
        {
            --VoteCount;
        }
        LocalDisableVotes.RemoveAt(i);
    }
}

void UGameFeatureAction_SplitscreenConfig::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
    if (bDisableSplitscreen)
    {
        if (const UGameInstance* GameInstance = WorldContext.OwningGameInstance)
        {
            if (UGameViewportClient* VC = GameInstance->GetGameViewportClient())
            {
                const FObjectKey ViewportKey(VC);

                LocalDisableVotes.Add(ViewportKey);

                int32& VoteCount = GlobalDisableVotes.FindOrAdd(ViewportKey);
                VoteCount++;
                if (VoteCount == 1)
                {
                    VC->SetForceDisableSplitscreen(true);
                }
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE
