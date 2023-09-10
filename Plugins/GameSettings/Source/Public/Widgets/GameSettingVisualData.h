// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "GameSettingVisualData.generated.h"

class FName;
class UGameSetting;
class UGameSettingDetailExtension;
class UGameSettingListEntryBase;
class UObject;

USTRUCT(BlueprintType)
struct FGameSettingClassExtensions
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Extensions)
	TArray<TSoftClassPtr<UGameSettingDetailExtension>> Extensions;
};

USTRUCT(BlueprintType)
struct FGameSettingNameExtensions
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Extensions)
	bool bIncludeClassDefaultExtensions = false;

	UPROPERTY(EditAnywhere, Category = Extensions)
	TArray<TSoftClassPtr<UGameSettingDetailExtension>> Extensions;
};

/**
 * 
 */
UCLASS(BlueprintType)
class GAMESETTINGS_API UGameSettingVisualData : public UDataAsset
{
	GENERATED_BODY()

public:
	TSubclassOf<UGameSettingListEntryBase> GetEntryForSetting(UGameSetting* InSetting);

	virtual TArray<TSoftClassPtr<UGameSettingDetailExtension>> GatherDetailExtensions(UGameSetting* InSetting);
	
protected:
	virtual TSubclassOf<UGameSettingListEntryBase> GetCustomEntryForSetting(UGameSetting* InSetting);

protected:
	UPROPERTY(EditDefaultsOnly, Category = ListEntries, meta = (AllowAbstract))
	TMap<TSubclassOf<UGameSetting>, TSubclassOf<UGameSettingListEntryBase>> EntryWidgetForClass;

	UPROPERTY(EditDefaultsOnly, Category = ListEntries, meta = (AllowAbstract))
	TMap<FName, TSubclassOf<UGameSettingListEntryBase>> EntryWidgetForName;

	UPROPERTY(EditDefaultsOnly, Category = Extensions, meta = (AllowAbstract))
	TMap<TSubclassOf<UGameSetting>, FGameSettingClassExtensions> ExtensionsForClasses;

	UPROPERTY(EditDefaultsOnly, Category = Extensions)
	TMap<FName, FGameSettingNameExtensions> ExtensionsForName;
};
