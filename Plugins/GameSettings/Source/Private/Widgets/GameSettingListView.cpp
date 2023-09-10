// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/GameSettingListView.h"
#include "Widgets/GameSettingListEntry.h"
#include "Widgets/GameSettingVisualData.h"


#include "GameSettingCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingListView)

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif

#define LOCTEXT_NAMESPACE "GameSetting"

UGameSettingListView::UGameSettingListView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR

void UGameSettingListView::ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const
{
	Super::ValidateCompiledDefaults(InCompileLog);

	if (!VisualData)
	{
		InCompileLog.Error(FText::Format(FText::FromString("{0} has no VisualData defined."), FText::FromString(GetName())));
	}
}

#endif

UUserWidget& UGameSettingListView::OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	UGameSetting* SettingItem = Cast<UGameSetting>(Item);

	TSubclassOf<UGameSettingListEntryBase> SettingEntryClass = TSubclassOf<UGameSettingListEntryBase>(DesiredEntryClass);
	if (VisualData)
	{
		if (const TSubclassOf<UGameSettingListEntryBase> EntryClassSetting = VisualData->GetEntryForSetting(SettingItem))
		{
			SettingEntryClass = EntryClassSetting;
		}
		else
		{
			//UE_LOG(LogGameSettings, Error, TEXT("UGameSettingListView: No Entry Class Found!"));
		}
	}
	else
	{
		//UE_LOG(LogGameSettings, Error, TEXT("UGameSettingListView: No VisualData Defined!"));
	}

	UGameSettingListEntryBase& EntryWidget = GenerateTypedEntry<UGameSettingListEntryBase>(SettingEntryClass, OwnerTable);

	if (!IsDesignTime())
	{
		if (const FText* Override = NameOverrides.Find(SettingItem->GetDevName()))
		{
			EntryWidget.SetDisplayNameOverride(*Override);
		}

		EntryWidget.SetSetting(SettingItem);
	}

	return EntryWidget;
}

bool UGameSettingListView::OnIsSelectableOrNavigableInternal(UObject* SelectedItem)
{
	if (const UGameSettingCollection* CollectionItem = Cast<UGameSettingCollection>(SelectedItem))
	{
		return CollectionItem->IsSelectable();
	}

	return true;
}

void UGameSettingListView::AddNameOverride(const FName& DevName, const FText& OverrideName)
{
	NameOverrides.Add(DevName, OverrideName);
}

#undef LOCTEXT_NAMESPACE
