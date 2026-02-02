// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "GameSettingRegistry.h"
#include "GameSettingRegistryChangeTracker.h"

#include "GameSettingScreen.generated.h"

#define UE_API GAMESETTINGS_API

class UGameSetting;
class UGameSettingCollection;
class UGameSettingPanel;
class UObject;
class UWidget;
struct FFrame;

enum class EGameSettingChangeReason : uint8;

/**
 * 
 */
UCLASS(MinimalAPI, Abstract, meta = (Category = "Settings", DisableNativeTick))
class UGameSettingScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()
public:

protected:
	UE_API virtual void NativeOnInitialized() override;
	UE_API virtual void NativeOnActivated() override;
	UE_API virtual void NativeOnDeactivated() override;
	UE_API virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UFUNCTION(BlueprintCallable)
	UE_API void NavigateToSetting(FName SettingDevName);
	
	UFUNCTION(BlueprintCallable)
	UE_API void NavigateToSettings(const TArray<FName>& SettingDevNames);

	UFUNCTION(BlueprintNativeEvent)
	UE_API void OnSettingsDirtyStateChanged(bool bSettingsDirty);
	virtual void OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty) { }

	UFUNCTION(BlueprintCallable)
	UE_API bool AttemptToPopNavigation();

	UFUNCTION(BlueprintCallable)
	UE_API UGameSettingCollection* GetSettingCollection(FName SettingDevName, bool& HasAnySettings); 

protected:
	virtual UGameSettingRegistry* CreateRegistry() PURE_VIRTUAL(, return nullptr;);

	template <typename GameSettingRegistryT = UGameSettingRegistry>
	GameSettingRegistryT* GetRegistry() const { return Cast<GameSettingRegistryT>(const_cast<UGameSettingScreen*>(this)->GetOrCreateRegistry()); }

	UFUNCTION(BlueprintCallable)
	UE_API virtual void CancelChanges();

	UFUNCTION(BlueprintCallable)
	UE_API virtual void ApplyChanges();

	UFUNCTION(BlueprintCallable)
	bool HaveSettingsBeenChanged() const { return ChangeTracker.HaveSettingsBeenChanged(); }

	UE_API void ClearDirtyState();

	UE_API void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);

	FGameSettingRegistryChangeTracker ChangeTracker;

private:
	UE_API UGameSettingRegistry* GetOrCreateRegistry();

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameSettingPanel> Settings_Panel;

	UPROPERTY(Transient)
	mutable TObjectPtr<UGameSettingRegistry> Registry;
};

#undef UE_API
