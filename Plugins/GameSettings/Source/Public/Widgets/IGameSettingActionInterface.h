// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UObject/Interface.h"

#include "IGameSettingActionInterface.generated.h"

class UGameSetting;
class UObject;
struct FFrame;

UINTERFACE(MinimalAPI, meta = (BlueprintType))
class UGameSettingActionInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class GAMESETTINGS_API IGameSettingActionInterface
{
	GENERATED_BODY()

public:
	/** */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ExecuteActionForSetting(FGameplayTag ActionTag, UGameSetting* InSetting);
};

