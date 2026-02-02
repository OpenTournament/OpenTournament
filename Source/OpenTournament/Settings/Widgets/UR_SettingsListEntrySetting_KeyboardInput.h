// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Widgets/GameSettingListEntry.h"

#include "UR_SettingsListEntrySetting_KeyboardInput.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UKeyAlreadyBoundWarning;

class UGameSetting;
class UGameSettingPressAnyKey;
class UUR_ButtonBase;
class UUR_SettingKeyboardInput;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// UUR_SettingsListEntrySetting_KeyboardInput
//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class UUR_SettingsListEntrySetting_KeyboardInput : public UGameSettingListEntry_Setting
{
    GENERATED_BODY()

public:
    virtual void SetSetting(UGameSetting* InSetting) override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeOnEntryReleased() override;
    virtual void OnSettingChanged() override;

    void HandlePrimaryKeyClicked();
    void HandleSecondaryKeyClicked();
    void HandleClearClicked();
    void HandleResetToDefaultClicked();

    void HandlePrimaryKeySelected(FKey InKey, UGameSettingPressAnyKey* PressAnyKeyPanel);
    void HandleSecondaryKeySelected(FKey InKey, UGameSettingPressAnyKey* PressAnyKeyPanel);
    void HandlePrimaryDuplicateKeySelected(FKey InKey, UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const;
    void HandleSecondaryDuplicateKeySelected(FKey InKey, UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const;
    void ChangeBinding(int32 BindSlot, FKey InKey);
    void HandleKeySelectionCanceled(UGameSettingPressAnyKey* PressAnyKeyPanel);
    void HandleKeySelectionCanceled(UKeyAlreadyBoundWarning* PressAnyKeyPanel);

    void Refresh();

private:
    UPROPERTY(Transient)
    FKey OriginalKeyToBind = EKeys::Invalid;

protected:
    UPROPERTY()
    TObjectPtr<UUR_SettingKeyboardInput> KeyboardInputSetting;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameSettingPressAnyKey> PressAnyKeyPanelClass;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UKeyAlreadyBoundWarning> KeyAlreadyBoundWarningPanelClass;

private:	// Bound Widgets
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
    TObjectPtr<UUR_ButtonBase> Button_PrimaryKey;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
    TObjectPtr<UUR_ButtonBase> Button_SecondaryKey;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
    TObjectPtr<UUR_ButtonBase> Button_Clear;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
    TObjectPtr<UUR_ButtonBase> Button_ResetToDefault;
};
