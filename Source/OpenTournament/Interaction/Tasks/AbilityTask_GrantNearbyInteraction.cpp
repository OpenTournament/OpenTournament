// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "AbilityTask_GrantNearbyInteraction.h"

#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"

#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionOption.h"
#include "Interaction/InteractionQuery.h"
#include "Interaction/InteractionStatics.h"
#include "Physics/UR_CollisionChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_GrantNearbyInteraction)

/////////////////////////////////////////////////////////////////////////////////////////////////

UAbilityTask_GrantNearbyInteraction::UAbilityTask_GrantNearbyInteraction(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

/////////////////////////////////////////////////////////////////////////////////////////////////

UAbilityTask_GrantNearbyInteraction* UAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors(UGameplayAbility* OwningAbility, float InteractionScanRange, float InteractionScanRate)
{
    UAbilityTask_GrantNearbyInteraction* MyObj = NewAbilityTask<UAbilityTask_GrantNearbyInteraction>(OwningAbility);
    MyObj->InteractionScanRange = InteractionScanRange;
    MyObj->InteractionScanRate = InteractionScanRate;
    return MyObj;
}

void UAbilityTask_GrantNearbyInteraction::Activate()
{
    SetWaitingOnAvatar();

    UWorld* World = GetWorld();
    World->GetTimerManager().SetTimer(QueryTimerHandle, this, &ThisClass::QueryInteractables, InteractionScanRate, true);
}

void UAbilityTask_GrantNearbyInteraction::OnDestroy(bool AbilityEnded)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(QueryTimerHandle);
    }

    Super::OnDestroy(AbilityEnded);
}

void UAbilityTask_GrantNearbyInteraction::QueryInteractables()
{
    UWorld* World = GetWorld();
    AActor* ActorOwner = GetAvatarActor();

    if (World && ActorOwner)
    {
        FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_GrantNearbyInteraction), false);

        TArray<FOverlapResult> OverlapResults;
        World->OverlapMultiByChannel(OUT OverlapResults, ActorOwner->GetActorLocation(), FQuat::Identity, Game_TraceChannel_Interaction, FCollisionShape::MakeSphere(InteractionScanRange), Params);

        if (OverlapResults.Num() > 0)
        {
            TArray<TScriptInterface<IInteractableTarget>> InteractableTargets;
            UInteractionStatics::AppendInteractableTargetsFromOverlapResults(OverlapResults, OUT InteractableTargets);

            FInteractionQuery InteractionQuery;
            InteractionQuery.RequestingAvatar = ActorOwner;
            InteractionQuery.RequestingController = Cast<AController>(ActorOwner->GetOwner());

            TArray<FInteractionOption> Options;
            for (TScriptInterface<IInteractableTarget>& InteractiveTarget : InteractableTargets)
            {
                FInteractionOptionBuilder InteractionBuilder(InteractiveTarget, Options);
                InteractiveTarget->GatherInteractionOptions(InteractionQuery, InteractionBuilder);
            }

            // Check if any of the options need to grant the ability to the user before they can be used.
            for (FInteractionOption& Option : Options)
            {
                if (Option.InteractionAbilityToGrant)
                {
                    // Grant the ability to the GAS, otherwise it won't be able to do whatever the interaction is.
                    FObjectKey ObjectKey(Option.InteractionAbilityToGrant);
                    if (!InteractionAbilityCache.Find(ObjectKey))
                    {
                        FGameplayAbilitySpec Spec(Option.InteractionAbilityToGrant, 1, INDEX_NONE, this);
                        FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);
                        InteractionAbilityCache.Add(ObjectKey, Handle);
                    }
                }
            }
        }
    }
}
