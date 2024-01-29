// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"

#include "GameSettingAction.generated.h"

//--------------------------------------
// UGameSettingAction
//--------------------------------------

class ULocalPlayer;

DECLARE_DELEGATE_TwoParams(UGameSettingCustomAction, UGameSetting* /*Setting*/, ULocalPlayer* /*LocalPlayer*/)

/**
 * 
 */
UCLASS()
class GAMESETTINGS_API UGameSettingAction : public UGameSetting
{
	GENERATED_BODY()

public:
	UGameSettingAction();

public:

	DECLARE_EVENT_TwoParams(UGameSettingAction, FOnExecuteNamedAction, UGameSetting* /*Setting*/, FGameplayTag /*GameSettings_Action_Tag*/);
	FOnExecuteNamedAction OnExecuteNamedActionEvent;

public:

	FText GetActionText() const { return ActionText; }
	void SetActionText(FText Value) { ActionText = Value; }
#if !UE_BUILD_SHIPPING
	void SetActionText(const FString& Value) { SetActionText(FText::FromString(Value)); }
#endif

	FGameplayTag GetNamedAction() const { return NamedAction; }
	void SetNamedAction(FGameplayTag Value) { NamedAction = Value; }

	bool HasCustomAction() const { return CustomAction.IsBound(); }
	void SetCustomAction(UGameSettingCustomAction InAction) { CustomAction = InAction; }
	void SetCustomAction(TFunction<void(ULocalPlayer*)> InAction);

	/**
	 * By default actions don't dirty the settings, since the majority of them either do things you can't
	 * revert, or they're things like show the credits or Eula.  However if that's not true, set this flag
	 * to true to fire the change event when this action is fired.
	 */
	void SetDoesActionDirtySettings(bool Value) { bDirtyAction = Value; }

	virtual void ExecuteAction();

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;

protected:
	FText ActionText;
	FGameplayTag NamedAction;
	UGameSettingCustomAction CustomAction;
	bool bDirtyAction = false;
};
