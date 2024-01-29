// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Kismet/BlueprintAsyncActionBase.h>

#include "AsyncAction_ExperienceReady.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AGameStateBase;
class UUR_ExperienceDefinition;
class UWorld;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExperienceReadyAsyncDelegate);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Asynchronously waits for the game state to be ready and valid and then calls the OnReady event.  Will call OnReady
 * immediately if the game state is valid already.
 */
UCLASS()
class UAsyncAction_ExperienceReady : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	// Waits for the experience to be determined and loaded
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UAsyncAction_ExperienceReady* WaitForExperienceReady(UObject* WorldContextObject);

	virtual void Activate() override;

public:

	// Called when the experience has been determined and is ready/loaded
	UPROPERTY(BlueprintAssignable)
	FExperienceReadyAsyncDelegate OnReady;

private:
	void Step1_HandleGameStateSet(AGameStateBase* GameState);
	void Step2_ListenToExperienceLoading(AGameStateBase* GameState);
	void Step3_HandleExperienceLoaded(const UUR_ExperienceDefinition* CurrentExperience);
	void Step4_BroadcastReady();

	TWeakObjectPtr<UWorld> WorldPtr;
};
