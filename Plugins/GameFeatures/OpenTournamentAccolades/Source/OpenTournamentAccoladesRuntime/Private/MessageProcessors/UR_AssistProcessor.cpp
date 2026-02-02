// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "MessageProcessors/UR_AssistProcessor.h"

#include "NativeGameplayTags.h"
#include "GameFramework/PlayerState.h"

#include "Messages/GameVerbMessage.h"
#include "Messages/GameVerbMessageHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AssistProcessor)

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace AssistProcessor
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Game_Elimination_Message, "Game.Elimination.Message");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Game_Damage_Message, "Game.Damage.Message");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Game_Assist_Message, "Game.Assist.Message");
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AssistProcessor::StartListening()
{
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    AddListenerHandle(MessageSubsystem.RegisterListener(AssistProcessor::TAG_Game_Elimination_Message, this, &ThisClass::OnEliminationMessage));
    AddListenerHandle(MessageSubsystem.RegisterListener(AssistProcessor::TAG_Game_Damage_Message, this, &ThisClass::OnDamageMessage));
}


void UUR_AssistProcessor::OnDamageMessage(FGameplayTag Channel, const FGameVerbMessage& Payload)
{
    if (Payload.Instigator != Payload.Target)
    {
        if (APlayerState* InstigatorPS = UGameVerbMessageHelpers::GetPlayerStateFromObject(Payload.Instigator))
        {
            if (APlayerState* TargetPS = UGameVerbMessageHelpers::GetPlayerStateFromObject(Payload.Target))
            {
                FPlayerAssistDamageTracking& Damage = DamageHistory.FindOrAdd(TargetPS);
                float& DamageTotalFromTarget = Damage.AccumulatedDamageByPlayer.FindOrAdd(InstigatorPS);
                DamageTotalFromTarget += Payload.Magnitude;
            }
        }
    }
}


void UUR_AssistProcessor::OnEliminationMessage(FGameplayTag Channel, const FGameVerbMessage& Payload)
{
    if (APlayerState* TargetPS = Cast<APlayerState>(Payload.Target))
    {
        // Grant an assist to each player who damaged the target but wasn't the instigator
        if (FPlayerAssistDamageTracking* DamageOnTarget = DamageHistory.Find(TargetPS))
        {
            for (const auto& KVP : DamageOnTarget->AccumulatedDamageByPlayer)
            {
                if (APlayerState* AssistPS = KVP.Key)
                {
                    if (AssistPS != Payload.Instigator)
                    {
                        FGameVerbMessage AssistMessage;
                        AssistMessage.Verb = AssistProcessor::TAG_Game_Assist_Message;
                        AssistMessage.Instigator = AssistPS;
                        //@TODO: Get default tags from a player state or save off most recent tags during assist damage?
                        //AssistMessage.InstigatorTags = ;
                        AssistMessage.Target = TargetPS;
                        AssistMessage.TargetTags = Payload.TargetTags;
                        AssistMessage.ContextTags = Payload.ContextTags;
                        AssistMessage.Magnitude = KVP.Value;

                        UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
                        MessageSubsystem.BroadcastMessage(AssistMessage.Verb, AssistMessage);
                    }
                }
            }

            // Clear the damage log for the eliminated player
            DamageHistory.Remove(TargetPS);
        }
    }
}
