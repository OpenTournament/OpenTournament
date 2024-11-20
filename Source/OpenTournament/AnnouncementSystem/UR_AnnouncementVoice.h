// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Engine/DataAsset.h>

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
class OPENTOURNAMENT_API UUR_AnnouncementVoice
    : public UPrimaryDataAsset
{
	GENERATED_BODY()

	/////////////////////////////////////////////////////////////////////////////////////////////////

	UUR_AnnouncementVoice();

	/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

	public:

    /**
     * UniqueID of the Announcer
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTags")
    FName AnnouncerID;

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
