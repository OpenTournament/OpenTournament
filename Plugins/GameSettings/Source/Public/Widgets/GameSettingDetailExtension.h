// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"

#include "GameSettingDetailExtension.generated.h"

enum class EGameSettingChangeReason : uint8;

class UGameSetting;
class UObject;

/**
 * 
 */
UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingDetailExtension : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSetting(UGameSetting* InSetting);
	
protected:
	virtual void NativeSettingAssigned(UGameSetting* InSetting);
	virtual void NativeSettingValueChanged(UGameSetting* InSetting, EGameSettingChangeReason Reason);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingAssigned(UGameSetting* InSetting);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingValueChanged(UGameSetting* InSetting);

protected:
	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> Setting;
};
