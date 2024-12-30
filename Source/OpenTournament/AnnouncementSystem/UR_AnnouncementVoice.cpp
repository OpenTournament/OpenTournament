// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AnnouncementVoice.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AnnouncementVoice::UUR_AnnouncementVoice()
    : Super()
{
}

#if WITH_EDITOR
EDataValidationResult UUR_AnnouncementVoice::IsDataValid(FDataValidationContext& Context) const
{
    return Super::IsDataValid(Context);
}
#endif // WITH_EDITOR

USoundBase* UUR_AnnouncementVoice::GetAnnouncementSound(const FGameplayTag& GameplayTag)
{
    if (TagAnnouncementMap.Contains(GameplayTag))
    {
        return TagAnnouncementMap.FindRef(GameplayTag);
    }
    return nullptr;
}
