// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingValue.h"

#include "GameSettingValueDiscrete.generated.h"

class UObject;
struct FFrame;

UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingValueDiscrete : public UGameSettingValue
{
	GENERATED_BODY()

public:
	UGameSettingValueDiscrete();

	/** UGameSettingValueDiscrete */
	virtual void SetDiscreteOptionByIndex(int32 Index) PURE_VIRTUAL(,);
	
	UFUNCTION(BlueprintCallable)
	virtual int32 GetDiscreteOptionIndex() const PURE_VIRTUAL(,return INDEX_NONE;);

	/** Optional */
	UFUNCTION(BlueprintCallable)
	virtual int32 GetDiscreteOptionDefaultIndex() const { return INDEX_NONE; }

	UFUNCTION(BlueprintCallable)
	virtual TArray<FText> GetDiscreteOptions() const PURE_VIRTUAL(,return TArray<FText>(););

	virtual FString GetAnalyticsValue() const;
};
