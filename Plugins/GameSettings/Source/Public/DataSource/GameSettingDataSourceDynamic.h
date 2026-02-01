// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingDataSource.h"
#include "PropertyPathHelpers.h"

#define UE_API GAMESETTINGS_API

class ULocalPlayer;

//--------------------------------------
// FGameSettingDataSourceDynamic
//--------------------------------------

class FGameSettingDataSourceDynamic : public FGameSettingDataSource
{
public:
	UE_API FGameSettingDataSourceDynamic(const TArray<FString>& InDynamicPath);

	UE_API virtual bool Resolve(ULocalPlayer* InLocalPlayer) override;

	UE_API virtual FString GetValueAsString(ULocalPlayer* InLocalPlayer) const override;

	UE_API virtual void SetValue(ULocalPlayer* InLocalPlayer, const FString& Value) override;

	UE_API virtual FString ToString() const override;

private:
	FCachedPropertyPath DynamicPath;
};

#undef UE_API
