// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtrTemplates.h"

enum class EGameSettingChangeReason : uint8;

class UGameSetting;
class UGameSettingRegistry;
struct FObjectKey;

/**
 * 
 */
class GAMESETTINGS_API FGameSettingRegistryChangeTracker : public FNoncopyable
{
public:
	FGameSettingRegistryChangeTracker();
	~FGameSettingRegistryChangeTracker();

	void WatchRegistry(UGameSettingRegistry* InRegistry);
	void StopWatchingRegistry();

	void ApplyChanges();

	void RestoreToInitial();

	void ClearDirtyState();

	bool IsRestoringSettings() const { return bRestoringSettings; }
	bool HaveSettingsBeenChanged() const { return bSettingsChanged; }

private:
	void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);

	bool bSettingsChanged = false;
	bool bRestoringSettings = false;

	TWeakObjectPtr<UGameSettingRegistry> Registry;
	TMap<FObjectKey, TWeakObjectPtr<UGameSetting>> DirtySettings;
};
