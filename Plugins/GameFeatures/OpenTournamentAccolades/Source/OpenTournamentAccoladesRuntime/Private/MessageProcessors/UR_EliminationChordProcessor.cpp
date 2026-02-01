// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "MessageProcessors/UR_EliminationChordProcessor.h"

#include "NativeGameplayTags.h"
#include "GameFramework/PlayerState.h"

#include "Messages/GameVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_EliminationChordProcessor)

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace EliminationChord
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Game_Elimination_Message, "Game.Elimination.Message");
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_EliminationChordProcessor::StartListening()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	AddListenerHandle(MessageSubsystem.RegisterListener(EliminationChord::TAG_Game_Elimination_Message, this, &ThisClass::OnEliminationMessage));
}

void UUR_EliminationChordProcessor::OnEliminationMessage(FGameplayTag Channel, const FGameVerbMessage& Payload)
{
	// Track elimination chords for the attacker (except for self-eliminations)
	if (Payload.Instigator != Payload.Target)
	{
		if (APlayerState* InstigatorPS = Cast<APlayerState>(Payload.Instigator))
		{
			const double CurrentTime = GetServerTime();

			FPlayerEliminationChordInfo& History = PlayerChordHistory.FindOrAdd(InstigatorPS);
			const bool bChordReset = (History.LastEliminationTime == 0.0) || (History.LastEliminationTime + ChordTimeLimit < CurrentTime);

			History.LastEliminationTime = CurrentTime;
			if (bChordReset)
			{
				History.ChordCounter = 1;
			}
			else
			{
				++History.ChordCounter;

				if (FGameplayTag* FoundTag = EliminationChordTags.Find(History.ChordCounter))
				{
					FGameVerbMessage ChordMessage;
					ChordMessage.Verb = *FoundTag;
					ChordMessage.Instigator = InstigatorPS;
					ChordMessage.InstigatorTags = Payload.InstigatorTags;
					ChordMessage.ContextTags = Payload.ContextTags;
					ChordMessage.Magnitude = History.ChordCounter;

					UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
					MessageSubsystem.BroadcastMessage(ChordMessage.Verb, ChordMessage);
				}
			}
		}
	}
}

