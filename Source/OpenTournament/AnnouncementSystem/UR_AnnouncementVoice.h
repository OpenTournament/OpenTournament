// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Sound/SoundBase.h"

#include "UR_AnnouncementVoice.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
* AnnouncementVoice.
* Handles playing Game Announcements for Local Player
*/
UCLASS(Blueprintable, BlueprintType)
class OPENTOURNAMENT_API UUR_AnnouncementVoice : public UObject
{
	GENERATED_BODY()
	
	/////////////////////////////////////////////////////////////////////////////////////////////////

	UUR_AnnouncementVoice();
	
	/////////////////////////////////////////////////////////////////////////////////////////////////


	public:

	/**
	* Get the Sound associated with this GameplayTag
	*/
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "AnnouncementSystem")
	USoundBase* GetAnnouncementSound(const FGameplayTag& GameplayTag);

	/**
	* Map of Tags to Audio
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTags")
	TMap<FGameplayTag, USoundBase*> TagAnnouncementMap;
};
