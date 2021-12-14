// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AnnouncementSubsystem.h"

#include "UObject/Object.h"

#include "Kismet/GameplayStatics.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AnnouncementSubsystem::UUR_AnnouncementSubsystem()
	: Super(),
	AnnouncementVoiceClass(nullptr),
	AnnouncementVoice(nullptr),
	AnnouncementVolume(1.f)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AnnouncementSubsystem::SetAnnouncementVoice(const TSubclassOf<UUR_AnnouncementVoice> InAnnouncementVoiceClass)
{
	AnnouncementVoiceClass = InAnnouncementVoiceClass;
	AnnouncementVoice = InAnnouncementVoiceClass.GetDefaultObject();
}

USoundBase* UUR_AnnouncementSubsystem::GetAnnouncementSound(const FGameplayTag& GameplayTag)
{	
	USoundBase* AnnouncementSound{};

	if (AnnouncementVoiceClass)
	{
		// Ensure the AnnouncementVoice is set
        AnnouncementVoice = AnnouncementVoiceClass->GetDefaultObject<UUR_AnnouncementVoice>();
		if (AnnouncementVoice)
		{
			AnnouncementSound = AnnouncementVoice->GetAnnouncementSound(GameplayTag);
		}
	}
	
	return AnnouncementSound;
}

void UUR_AnnouncementSubsystem::PlayAnnouncement(const FGameplayTag& InAnnouncement)
{
	USoundBase* AnnouncementSound = GetAnnouncementSound(InAnnouncement);
	UGameplayStatics::PlaySound2D(this, AnnouncementSound, AnnouncementVolume);
}
