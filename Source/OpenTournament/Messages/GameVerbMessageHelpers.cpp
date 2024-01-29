// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameVerbMessageHelpers.h"

#include <GameplayEffectTypes.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerController.h>
#include <GameFramework/PlayerState.h>
#include <Messages/GameVerbMessage.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameVerbMessageHelpers)

//////////////////////////////////////////////////////////////////////
// FGameVerbMessage

FString FGameVerbMessage::ToString() const
{
	FString HumanReadableMessage;
	FGameVerbMessage::StaticStruct()->ExportText(/*out*/ HumanReadableMessage, this, /*Defaults=*/ nullptr, /*OwnerObject=*/ nullptr, PPF_None, /*ExportRootScope=*/ nullptr);
	return HumanReadableMessage;
}

//////////////////////////////////////////////////////////////////////
//

APlayerState* UGameVerbMessageHelpers::GetPlayerStateFromObject(UObject* Object)
{
	if (const APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC->PlayerState;
	}

	if (APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS;
	}

	if (const APawn* TargetPawn = Cast<APawn>(Object))
	{
		if (APlayerState* TargetPS = TargetPawn->GetPlayerState())
		{
			return TargetPS;
		}
	}
	return nullptr;
}

APlayerController* UGameVerbMessageHelpers::GetPlayerControllerFromObject(UObject* Object)
{
	if (APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC;
	}

	if (const APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS->GetPlayerController();
	}

	if (const APawn* TargetPawn = Cast<APawn>(Object))
	{
		return Cast<APlayerController>(TargetPawn->GetController());
	}

	return nullptr;
}

FGameplayCueParameters UGameVerbMessageHelpers::VerbMessageToCueParameters(const FGameVerbMessage& Message)
{
	FGameplayCueParameters Result;

	Result.OriginalTag = Message.Verb;
	Result.Instigator = Cast<AActor>(Message.Instigator);
	Result.EffectCauser = Cast<AActor>(Message.Target);
	Result.AggregatedSourceTags = Message.InstigatorTags;
	Result.AggregatedTargetTags = Message.TargetTags;
	//@TODO: = Message.ContextTags;
	Result.RawMagnitude = Message.Magnitude;

	return Result;
}

FGameVerbMessage UGameVerbMessageHelpers::CueParametersToVerbMessage(const FGameplayCueParameters& Params)
{
	FGameVerbMessage Result;

	Result.Verb = Params.OriginalTag;
	Result.Instigator = Params.Instigator.Get();
	Result.Target = Params.EffectCauser.Get();
	Result.InstigatorTags = Params.AggregatedSourceTags;
	Result.TargetTags = Params.AggregatedTargetTags;
	//@TODO: Result.ContextTags = ???;
	Result.Magnitude = Params.RawMagnitude;

	return Result;
}

