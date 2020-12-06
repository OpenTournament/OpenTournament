// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "UR_GameInstance.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_GameInstance : public UGameInstance
{
    GENERATED_BODY()
    
public:
    virtual void Init() override;

	// @! TODO : Restrict to certain build types / configurations?
	/**
	* FTest_AddLocalPlayer
	* Simulates creating ULocalPlayer and AController.
	* Exposed this function to Blueprint for Feature Testing
	*/
	UFUNCTION(BlueprintCallable, Category = "GameInstance")
	virtual void FTest_AddLocalPlayer(int32 ControllerId);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION()
    virtual void BeginLoadingScreen(const FString& MapName);

    UFUNCTION()
    virtual void EndLoadingScreen(UWorld* InLoadedWorld);
};
