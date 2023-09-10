// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"

#include "GameSettingCollection.generated.h"

struct FGameSettingFilterState;

//--------------------------------------
// UGameSettingCollection
//--------------------------------------

UCLASS()
class GAMESETTINGS_API UGameSettingCollection : public UGameSetting
{
	GENERATED_BODY()

public:
	UGameSettingCollection();

	virtual TArray<UGameSetting*> GetChildSettings() override { return Settings; }
	TArray<UGameSettingCollection*> GetChildCollections() const;

	void AddSetting(UGameSetting* Setting);
	virtual void GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings) const;

	virtual bool IsSelectable() const { return false; }

protected:
	/** The settings owned by this collection. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> Settings;
};

//--------------------------------------
// UGameSettingCollectionPage
//--------------------------------------

UCLASS()
class GAMESETTINGS_API UGameSettingCollectionPage : public UGameSettingCollection
{
	GENERATED_BODY()

public:

	DECLARE_EVENT_OneParam(UGameSettingCollectionPage, FOnExecuteNavigation, UGameSetting* /*Setting*/);
	FOnExecuteNavigation OnExecuteNavigationEvent;

public:
	UGameSettingCollectionPage();

	FText GetNavigationText() const { return NavigationText; }
	void SetNavigationText(FText Value) { NavigationText = Value; }
#if !UE_BUILD_SHIPPING
	void SetNavigationText(const FString& Value) { SetNavigationText(FText::FromString(Value)); }
#endif
	
	virtual void OnInitialized() override;
	virtual void GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings) const override;
	virtual bool IsSelectable() const override { return true; }

	/**  */
	void ExecuteNavigation();

private:
	FText NavigationText;
};
