// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AnnouncementVoice.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AnnouncementVoice::UUR_AnnouncementVoice() : Super()
{
	// Initialize TagAnnouncementMap?
}

USoundBase* UUR_AnnouncementVoice::GetAnnouncementSound(const FGameplayTag& GameplayTag)
{
	return *TagAnnouncementMap.Find(GameplayTag);
}
