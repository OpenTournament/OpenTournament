// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"

#include "GameSettingPressAnyKey.generated.h"

struct FKey;

class UObject;

/**
 * 
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingPressAnyKey : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UGameSettingPressAnyKey(const FObjectInitializer& Initializer);

	DECLARE_EVENT_OneParam(UGameSettingPressAnyKey, FOnKeySelected, FKey);
	FOnKeySelected OnKeySelected;

	DECLARE_EVENT(UGameSettingPressAnyKey, FOnKeySelectionCanceled);
	FOnKeySelectionCanceled OnKeySelectionCanceled;

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void HandleKeySelected(FKey InKey);
	void HandleKeySelectionCanceled();

	void Dismiss(TFunction<void()> PostDismissCallback);

private:
	bool bKeySelected = false;
	TSharedPtr<class FSettingsPressAnyKeyInputPreProcessor> InputProcessor;
};
