// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Sound/SoundBase.h"
#include "Subsystems/LocalPlayerSubsystem.h"

#include "UR_AnnouncementSubsystem.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

// Forward Declarations
// 
/////////////////////////////////////////////////////////////////////////////////////////////////


/**
* AnnouncementSubsystem.
* Handles playing Game Announcements for Local Player
*/
UCLASS()
class OPENTOURNAMENT_API UUR_AnnouncementSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

	public:
	
	/**
	* Implement this for initialization of instances of the system
	* 
	*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	/**
	*
	*/
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "AnnouncementSystem")
	USoundBase* GetAnnouncementSound(const FGameplayTag& GameplayTag);

	/**
	*
	*/
	UFUNCTION(BlueprintCallable, Category = "AnnouncementSystem")
	void PlayAnnouncement(const FGameplayTag& InAnnouncement);

	
	TMap<FGameplayTag, USoundBase> TagAnnouncementMap;
};
