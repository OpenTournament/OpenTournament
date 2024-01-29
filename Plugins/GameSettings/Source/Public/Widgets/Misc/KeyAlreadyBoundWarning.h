// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingPressAnyKey.h"
#include "KeyAlreadyBoundWarning.generated.h"

class UTextBlock;

/**
 * UKeyAlreadyBoundWarning
 * Press any key screen with text blocks for warning users when a key is already bound
 */
UCLASS(Abstract)
class GAMESETTINGS_API UKeyAlreadyBoundWarning : public UGameSettingPressAnyKey
{
	GENERATED_BODY()

public:
	void SetWarningText(const FText& InText);

	void SetCancelText(const FText& InText);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UTextBlock> WarningText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UTextBlock> CancelText;
};
