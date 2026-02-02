// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "MessageProcessors/UR_EliminationStreakProcessor.h"

#include "NativeGameplayTags.h"
#include "GameFramework/PlayerState.h"

#include "Messages/GameVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_EliminationStreakProcessor)

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ElimStreak
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Elimination_Message, "Lyra.Elimination.Message");
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_EliminationStreakProcessor::StartListening()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	AddListenerHandle(MessageSubsystem.RegisterListener(ElimStreak::TAG_Lyra_Elimination_Message, this, &ThisClass::OnEliminationMessage));
}

void UUR_EliminationStreakProcessor::OnEliminationMessage(FGameplayTag Channel, const FGameVerbMessage& Payload)
{
	// Track elimination streaks for the attacker (except for self-eliminations)
	if (Payload.Instigator != Payload.Target)
	{
		if (APlayerState* InstigatorPS = Cast<APlayerState>(Payload.Instigator))
		{
			int32& StreakCount = PlayerStreakHistory.FindOrAdd(InstigatorPS);
			StreakCount++;

			if (FGameplayTag* pTag = ElimStreakTags.Find(StreakCount))
			{
				FGameVerbMessage ElimStreakMessage;
				ElimStreakMessage.Verb = *pTag;
				ElimStreakMessage.Instigator = InstigatorPS;
				ElimStreakMessage.InstigatorTags = Payload.InstigatorTags;
				ElimStreakMessage.ContextTags = Payload.ContextTags;
 				ElimStreakMessage.Magnitude = StreakCount;

				UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
				MessageSubsystem.BroadcastMessage(ElimStreakMessage.Verb, ElimStreakMessage);
			}
		}
	}

	// End the elimination streak for the target
	if (APlayerState* TargetPS = Cast<APlayerState>(Payload.Target))
	{
		PlayerStreakHistory.Remove(TargetPS);
	}
}

