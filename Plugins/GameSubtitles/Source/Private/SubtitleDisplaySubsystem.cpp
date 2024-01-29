// Copyright Epic Games, Inc. All Rights Reserved.

#include "SubtitleDisplaySubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SubtitleDisplaySubsystem)

class FSubsystemCollectionBase;

USubtitleDisplaySubsystem* USubtitleDisplaySubsystem::Get(const ULocalPlayer* LocalPlayer)
{
	return LocalPlayer ? LocalPlayer->GetGameInstance()->GetSubsystem<USubtitleDisplaySubsystem>() : nullptr;
}

USubtitleDisplaySubsystem::USubtitleDisplaySubsystem()
{
}

void USubtitleDisplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{

}

void USubtitleDisplaySubsystem::Deinitialize()
{

}

const FSubtitleFormat& USubtitleDisplaySubsystem::GetSubtitleDisplayOptions() const
{
	return SubtitleFormat;
}

void USubtitleDisplaySubsystem::SetSubtitleDisplayOptions(const FSubtitleFormat& InOptions)
{
	SubtitleFormat = InOptions;
	DisplayFormatChangedEvent.Broadcast(SubtitleFormat);
}

