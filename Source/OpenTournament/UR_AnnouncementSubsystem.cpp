// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AnnouncementSubsystem.h"

#include "Kismet/GameplayStatics.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AnnouncementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize TagAnnouncementMap
}

USoundBase* UUR_AnnouncementSubsystem::GetAnnouncementSound(const FGameplayTag& GameplayTag)
{
	// auto AnnouncerVoice = GetAnnouncerVoice();
	// return AnnouncerVoice->GetAnnouncementAudio(GameplayTag);
	
	return TagAnnouncementMap.Find(GameplayTag);
}

void UUR_AnnouncementSubsystem::PlayAnnouncement(const FGameplayTag& InAnnouncement)
{
	USoundBase* AnnouncementSound = GetAnnouncementSound(InAnnouncement);
	UGameplayStatics::PlaySound2D(this, AnnouncementSound);
}
