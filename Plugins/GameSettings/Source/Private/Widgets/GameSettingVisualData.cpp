// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/GameSettingVisualData.h"

#include "GameSetting.h"
#include "Widgets/GameSettingListEntry.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingVisualData)

TSubclassOf<UGameSettingListEntryBase> UGameSettingVisualData::GetEntryForSetting(UGameSetting* InSetting)
{
	if (InSetting == nullptr)
	{
		return TSubclassOf<UGameSettingListEntryBase>();
	}

	// Check if there's a custom logic for finding this setting's visual element
	TSubclassOf<UGameSettingListEntryBase> CustomEntry = GetCustomEntryForSetting(InSetting);
	if (CustomEntry)
	{
		return CustomEntry;
	}

	// Check if there's a specific entry widget for a setting by name.  Hopefully this is super rare.
	{
		TSubclassOf<UGameSettingListEntryBase> EntryWidgetClassPtr = EntryWidgetForName.FindRef(InSetting->GetDevName());
		if (EntryWidgetClassPtr)
		{
			return EntryWidgetClassPtr;
		}
	}

	// Finally check to see if there's an entry for this setting following the classes we have entries for.
	// we use the super chain of the setting classes to find the most applicable entry widget for this class
	// of setting.
	for (UClass* Class = InSetting->GetClass(); Class; Class = Class->GetSuperClass())
	{
		if (TSubclassOf<UGameSetting> SettingClass = TSubclassOf<UGameSetting>(Class))
		{
			TSubclassOf<UGameSettingListEntryBase> EntryWidgetClassPtr = EntryWidgetForClass.FindRef(SettingClass);
			if (EntryWidgetClassPtr)
			{
				return EntryWidgetClassPtr;
			}
		}
	}

	return TSubclassOf<UGameSettingListEntryBase>();
}

TArray<TSoftClassPtr<UGameSettingDetailExtension>> UGameSettingVisualData::GatherDetailExtensions(UGameSetting* InSetting)
{
	TArray<TSoftClassPtr<UGameSettingDetailExtension>> Extensions;

	// Find extensions by setting name
	FGameSettingNameExtensions* ExtensionsWithName = ExtensionsForName.Find(InSetting->GetDevName());
	if (ExtensionsWithName)
	{
		Extensions.Append(ExtensionsWithName->Extensions);
		if (!ExtensionsWithName->bIncludeClassDefaultExtensions)
		{
			return Extensions;
		}
	}

	// Find extensions for it using the super chain of the setting so that we get any
	// class based extensions for this setting.
	for (UClass* Class = InSetting->GetClass(); Class; Class = Class->GetSuperClass())
	{
		if (TSubclassOf<UGameSetting> SettingClass = TSubclassOf<UGameSetting>(Class))
		{
			FGameSettingClassExtensions* ExtensionForClass = ExtensionsForClasses.Find(SettingClass);
			if (ExtensionForClass)
			{
				Extensions.Append(ExtensionForClass->Extensions);
			}
		}
	}

	return Extensions;
}

TSubclassOf<UGameSettingListEntryBase> UGameSettingVisualData::GetCustomEntryForSetting(UGameSetting* InSetting)
{
	return TSubclassOf<UGameSettingListEntryBase>();
}
