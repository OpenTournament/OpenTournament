// Copyright Epic Games, Inc. All Rights Reserved.

#include "DataSource/GameSettingDataSourceDynamic.h"

#include "Engine/LocalPlayer.h"

//--------------------------------------
// FGameSettingDataSourceDynamic
//--------------------------------------

FGameSettingDataSourceDynamic::FGameSettingDataSourceDynamic(const TArray<FString>& InDynamicPath)
	: DynamicPath(InDynamicPath)
{
}

bool FGameSettingDataSourceDynamic::Resolve(ULocalPlayer* InLocalPlayer)
{
	return DynamicPath.Resolve(InLocalPlayer);
}

FString FGameSettingDataSourceDynamic::GetValueAsString(ULocalPlayer* InLocalPlayer) const
{
	FString OutStringValue;

	const bool bSuccess = PropertyPathHelpers::GetPropertyValueAsString(InLocalPlayer, DynamicPath, OutStringValue);
	ensure(bSuccess);

	return OutStringValue;
}

void FGameSettingDataSourceDynamic::SetValue(ULocalPlayer* InLocalPlayer, const FString& InStringValue)
{
	const bool bSuccess = PropertyPathHelpers::SetPropertyValueFromString(InLocalPlayer, DynamicPath, InStringValue);
	ensure(bSuccess);
}

FString FGameSettingDataSourceDynamic::ToString() const
{
	return DynamicPath.ToString();
}
