// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "UR_AnnouncementVoice.h"
#include "Subsystems/LocalPlayerSubsystem.h"

#include "UR_AnnouncementSubsystem.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class USoundBase;
class UUR_AnnouncementVoice;
struct FGameplayTag;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
* AnnouncementSubsystem.
* Handles playing Game Announcements for Local Player
*/
UCLASS(Config = OTUserSettings)
class OPENTOURNAMENT_API UUR_AnnouncementSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

	/////////////////////////////////////////////////////////////////////////////////////////////////

	UUR_AnnouncementSubsystem();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	public:

	/**
	* Set the AnnouncementVoice
	*/
	UFUNCTION(BlueprintCallable, Category = "AnnouncementSystem")
	void SetAnnouncementVoice(const TSubclassOf<UUR_AnnouncementVoice> InAnnouncementVoiceClass);

	/**
	* Get the Announcement Audio for the given GameplayTag
	*/
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "AnnouncementSystem")
	USoundBase* GetAnnouncementSound(const FGameplayTag& GameplayTag);

	/**
	* Play an Announcement corresponding to the given GameplayTag
	*/
	UFUNCTION(BlueprintCallable, Category = "AnnouncementSystem")
	void PlayAnnouncement(const FGameplayTag& InAnnouncement);

	/**
	* Class of AnnouncementVoice used
	*/
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "AnnouncementSystem")
	TSoftObjectPtr<UUR_AnnouncementVoice> AnnouncementVoiceClass;

	/**
	* Announcement Voice
	*/
	UPROPERTY(BlueprintReadWrite, Category = "AnnouncementSystem")
	UUR_AnnouncementVoice* AnnouncementVoice;

	/**
	* Announcement Volume
	*/
	UPROPERTY(Config, BlueprintReadWrite, Category = "AnnouncementSystem")
	float AnnouncementVolume;
};
