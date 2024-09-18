// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SubtitleDisplayOptions.h"
#include "UR_GamepadSensitivty.h"
#include "GameFramework/SaveGame.h"
#include "UObject/ObjectPtr.h"

#include "UR_SettingsShared.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
class UUR_LocalPlayer;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum class EColorBlindMode : uint8
{
    Off,
    // Deuteranope (green weak/blind)
    Deuteranope,
    // Protanope (red weak/blind)
    Protanope,
    // Tritanope(blue weak / bind)
    Tritanope
};

/////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum class EGameAllowBackgroundAudioSetting : uint8
{
    Off,
    AllSounds,

    Num UMETA(Hidden),
};

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * UUR_SettingsShared - The "Shared" settings are stored as part of the USaveGame system, these settings are not machine
 * specific like the local settings, and are safe to store in the cloud - and 'share' them.  Using the save game system
 * we can also store settings per player, so things like controller keybind preferences should go here, because if those
 * are stored in the local settings all users would get them.
 *
 */
UCLASS()
class UUR_SettingsShared : public ULocalPlayerSaveGame
{
    GENERATED_BODY()

public:
    DECLARE_EVENT_OneParam(UUR_SettingsShared, FOnSettingChangedEvent, UUR_SettingsShared* Settings);

    FOnSettingChangedEvent OnSettingChanged;

public:
    UUR_SettingsShared();

    //~ULocalPlayerSaveGame interface
    int32 GetLatestDataVersion() const override;
    //~End of ULocalPlayerSaveGame interface

    bool IsDirty() const
    {
        return bIsDirty;
    }

    void ClearDirtyFlag()
    {
        bIsDirty = false;
    }

    /** Creates a temporary settings object, this will be replaced by one loaded from the user's save game */
    static UUR_SettingsShared* CreateTemporarySettings(const UUR_LocalPlayer* LocalPlayer);

    /** Synchronously loads a settings object, this is not valid to call before login */
    static UUR_SettingsShared* LoadOrCreateSettings(const UUR_LocalPlayer* LocalPlayer);

    DECLARE_DELEGATE_OneParam(FOnSettingsLoadedEvent, UUR_SettingsShared* Settings);

    /** Starts an async load of the settings object, calls Delegate on completion */
    static bool AsyncLoadOrCreateSettings(const UUR_LocalPlayer* LocalPlayer, FOnSettingsLoadedEvent Delegate);

    /** Saves the settings to disk */
    void SaveSettings();

    /** Applies the current settings to the player */
    void ApplySettings();

public:
    ////////////////////////////////////////////////////////
    // Color Blind Options

    UFUNCTION()
    EColorBlindMode GetColorBlindMode() const;

    UFUNCTION()
    void SetColorBlindMode(EColorBlindMode InMode);

    UFUNCTION()
    int32 GetColorBlindStrength() const;

    UFUNCTION()
    void SetColorBlindStrength(int32 InColorBlindStrength);

private:
    UPROPERTY()
    EColorBlindMode ColorBlindMode = EColorBlindMode::Off;

    UPROPERTY()
    int32 ColorBlindStrength = 10;

    ////////////////////////////////////////////////////////
    // Gamepad Vibration
public:
    UFUNCTION()
    bool GetForceFeedbackEnabled() const
    {
        return bForceFeedbackEnabled;
    }

    UFUNCTION()
    void SetForceFeedbackEnabled(const bool NewValue)
    {
        ChangeValueAndDirty(bForceFeedbackEnabled, NewValue);
    }

private:
    /** Is force feedback enabled when a controller is being used? */
    UPROPERTY()
    bool bForceFeedbackEnabled = true;

    ////////////////////////////////////////////////////////
    // Gamepad Deadzone
public:
    /** Getter for gamepad move stick dead zone value. */
    UFUNCTION()
    float GetGamepadMoveStickDeadZone() const
    {
        return GamepadMoveStickDeadZone;
    }

    /** Setter for gamepad move stick dead zone value. */
    UFUNCTION()
    void SetGamepadMoveStickDeadZone(const float NewValue)
    {
        ChangeValueAndDirty(GamepadMoveStickDeadZone, NewValue);
    }

    /** Getter for gamepad look stick dead zone value. */
    UFUNCTION()
    float GetGamepadLookStickDeadZone() const
    {
        return GamepadLookStickDeadZone;
    }

    /** Setter for gamepad look stick dead zone value. */
    UFUNCTION()
    void SetGamepadLookStickDeadZone(const float NewValue)
    {
        ChangeValueAndDirty(GamepadLookStickDeadZone, NewValue);
    }

private:
    /** Holds the gamepad move stick dead zone value. */
    UPROPERTY()
    float GamepadMoveStickDeadZone;

    /** Holds the gamepad look stick dead zone value. */
    UPROPERTY()
    float GamepadLookStickDeadZone;

    ////////////////////////////////////////////////////////
    // Gamepad Trigger Haptics
public:
    UFUNCTION()
    bool GetTriggerHapticsEnabled() const
    {
        return bTriggerHapticsEnabled;
    }

    UFUNCTION()
    void SetTriggerHapticsEnabled(const bool NewValue)
    {
        ChangeValueAndDirty(bTriggerHapticsEnabled, NewValue);
    }

    UFUNCTION()
    bool GetTriggerPullUsesHapticThreshold() const
    {
        return bTriggerPullUsesHapticThreshold;
    }

    UFUNCTION()
    void SetTriggerPullUsesHapticThreshold(const bool NewValue)
    {
        ChangeValueAndDirty(bTriggerPullUsesHapticThreshold, NewValue);
    }

    UFUNCTION()
    uint8 GetTriggerHapticStrength() const
    {
        return TriggerHapticStrength;
    }

    UFUNCTION()
    void SetTriggerHapticStrength(const uint8 NewValue)
    {
        ChangeValueAndDirty(TriggerHapticStrength, NewValue);
    }

    UFUNCTION()
    uint8 GetTriggerHapticStartPosition() const
    {
        return TriggerHapticStartPosition;
    }

    UFUNCTION()
    void SetTriggerHapticStartPosition(const uint8 NewValue)
    {
        ChangeValueAndDirty(TriggerHapticStartPosition, NewValue);
    }

private:
    /** Are trigger haptics enabled? */
    UPROPERTY()
    bool bTriggerHapticsEnabled = false;
    /** Does the game use the haptic feedback as its threshold for judging button presses? */
    UPROPERTY()
    bool bTriggerPullUsesHapticThreshold = true;
    /** The strength of the trigger haptic effects. */
    UPROPERTY()
    uint8 TriggerHapticStrength = 8;
    /** The start position of the trigger haptic effects */
    UPROPERTY()
    uint8 TriggerHapticStartPosition = 0;

    ////////////////////////////////////////////////////////
    // Subtitles
public:
    UFUNCTION()
    bool GetSubtitlesEnabled() const
    {
        return bEnableSubtitles;
    }

    UFUNCTION()
    void SetSubtitlesEnabled(bool Value)
    {
        ChangeValueAndDirty(bEnableSubtitles, Value);
    }

    UFUNCTION()
    ESubtitleDisplayTextSize GetSubtitlesTextSize() const
    {
        return SubtitleTextSize;
    }

    UFUNCTION()
    void SetSubtitlesTextSize(ESubtitleDisplayTextSize Value)
    {
        ChangeValueAndDirty(SubtitleTextSize, Value);
        ApplySubtitleOptions();
    }

    UFUNCTION()
    ESubtitleDisplayTextColor GetSubtitlesTextColor() const
    {
        return SubtitleTextColor;
    }

    UFUNCTION()
    void SetSubtitlesTextColor(ESubtitleDisplayTextColor Value)
    {
        ChangeValueAndDirty(SubtitleTextColor, Value);
        ApplySubtitleOptions();
    }

    UFUNCTION()
    ESubtitleDisplayTextBorder GetSubtitlesTextBorder() const
    {
        return SubtitleTextBorder;
    }

    UFUNCTION()
    void SetSubtitlesTextBorder(ESubtitleDisplayTextBorder Value)
    {
        ChangeValueAndDirty(SubtitleTextBorder, Value);
        ApplySubtitleOptions();
    }

    UFUNCTION()
    ESubtitleDisplayBackgroundOpacity GetSubtitlesBackgroundOpacity() const
    {
        return SubtitleBackgroundOpacity;
    }

    UFUNCTION()
    void SetSubtitlesBackgroundOpacity(ESubtitleDisplayBackgroundOpacity Value)
    {
        ChangeValueAndDirty(SubtitleBackgroundOpacity, Value);
        ApplySubtitleOptions();
    }

    void ApplySubtitleOptions();

private:
    UPROPERTY()
    bool bEnableSubtitles = true;

    UPROPERTY()
    ESubtitleDisplayTextSize SubtitleTextSize = ESubtitleDisplayTextSize::Medium;

    UPROPERTY()
    ESubtitleDisplayTextColor SubtitleTextColor = ESubtitleDisplayTextColor::White;

    UPROPERTY()
    ESubtitleDisplayTextBorder SubtitleTextBorder = ESubtitleDisplayTextBorder::None;

    UPROPERTY()
    ESubtitleDisplayBackgroundOpacity SubtitleBackgroundOpacity = ESubtitleDisplayBackgroundOpacity::Medium;

    ////////////////////////////////////////////////////////
    // Shared audio settings
public:
    UFUNCTION()
    EGameAllowBackgroundAudioSetting GetAllowAudioInBackgroundSetting() const
    {
        return AllowAudioInBackground;
    }

    UFUNCTION()
    void SetAllowAudioInBackgroundSetting(EGameAllowBackgroundAudioSetting NewValue);

    void ApplyBackgroundAudioSettings();

private:
    UPROPERTY()
    EGameAllowBackgroundAudioSetting AllowAudioInBackground = EGameAllowBackgroundAudioSetting::Off;

    ////////////////////////////////////////////////////////
    // Culture / language
public:
    /** Gets the pending culture */
    const FString& GetPendingCulture() const;

    /** Sets the pending culture to apply */
    void SetPendingCulture(const FString& NewCulture);

    // Called when the culture changes.
    void OnCultureChanged();

    /** Clears the pending culture to apply */
    void ClearPendingCulture();

    bool IsUsingDefaultCulture() const;

    void ResetToDefaultCulture();

    bool ShouldResetToDefaultCulture() const
    {
        return bResetToDefaultCulture;
    }

    void ApplyCultureSettings();

    void ResetCultureToCurrentSettings();

private:
    /** The pending culture to apply */
    UPROPERTY(Transient)
    FString PendingCulture;

    /* If true, resets the culture to default. */
    bool bResetToDefaultCulture = false;

    ////////////////////////////////////////////////////////
    // Gamepad Sensitivity
public:
    UFUNCTION()
    double GetMouseSensitivityX() const
    {
        return MouseSensitivityX;
    }

    UFUNCTION()
    void SetMouseSensitivityX(double NewValue)
    {
        ChangeValueAndDirty(MouseSensitivityX, NewValue);
        ApplyInputSensitivity();
    }

    UFUNCTION()
    double GetMouseSensitivityY() const
    {
        return MouseSensitivityY;
    }

    UFUNCTION()
    void SetMouseSensitivityY(double NewValue)
    {
        ChangeValueAndDirty(MouseSensitivityY, NewValue);
        ApplyInputSensitivity();
    }

    UFUNCTION()
    double GetTargetingMultiplier() const
    {
        return TargetingMultiplier;
    }

    UFUNCTION()
    void SetTargetingMultiplier(double NewValue)
    {
        ChangeValueAndDirty(TargetingMultiplier, NewValue);
        ApplyInputSensitivity();
    }

    UFUNCTION()
    bool GetInvertVerticalAxis() const
    {
        return bInvertVerticalAxis;
    }

    UFUNCTION()
    void SetInvertVerticalAxis(bool NewValue)
    {
        ChangeValueAndDirty(bInvertVerticalAxis, NewValue);
        ApplyInputSensitivity();
    }

    UFUNCTION()
    bool GetInvertHorizontalAxis() const
    {
        return bInvertHorizontalAxis;
    }

    UFUNCTION()
    void SetInvertHorizontalAxis(bool NewValue)
    {
        ChangeValueAndDirty(bInvertHorizontalAxis, NewValue);
        ApplyInputSensitivity();
    }

private:
    /** Holds the mouse horizontal sensitivity */
    UPROPERTY()
    double MouseSensitivityX = 1.0;

    /** Holds the mouse vertical sensitivity */
    UPROPERTY()
    double MouseSensitivityY = 1.0;

    /** Multiplier applied while Aiming down sights. */
    UPROPERTY()
    double TargetingMultiplier = 0.5;

    /** If true then the vertical look axis should be inverted */
    UPROPERTY()
    bool bInvertVerticalAxis = false;

    /** If true then the horizontal look axis should be inverted */
    UPROPERTY()
    bool bInvertHorizontalAxis = false;

    ////////////////////////////////////////////////////////
    // Gamepad Sensitivity
public:
    UFUNCTION()
    EGameGamepadSensitivity GetGamepadLookSensitivityPreset() const
    {
        return GamepadLookSensitivityPreset;
    }

    UFUNCTION()
    void SetLookSensitivityPreset(EGameGamepadSensitivity NewValue)
    {
        ChangeValueAndDirty(GamepadLookSensitivityPreset, NewValue);
        ApplyInputSensitivity();
    }

    UFUNCTION()
    EGameGamepadSensitivity GetGamepadTargetingSensitivityPreset() const
    {
        return GamepadTargetingSensitivityPreset;
    }

    UFUNCTION()
    void SetGamepadTargetingSensitivityPreset(EGameGamepadSensitivity NewValue)
    {
        ChangeValueAndDirty(GamepadTargetingSensitivityPreset, NewValue);
        ApplyInputSensitivity();
    }

    void ApplyInputSensitivity();

private:
    UPROPERTY()
    EGameGamepadSensitivity GamepadLookSensitivityPreset = EGameGamepadSensitivity::Normal;

    UPROPERTY()
    EGameGamepadSensitivity GamepadTargetingSensitivityPreset = EGameGamepadSensitivity::Normal;

    ////////////////////////////////////////////////////////
    /// Dirty and Change Reporting
private:
    template <typename T>
    bool ChangeValueAndDirty(T& CurrentValue, const T& NewValue)
    {
        if (CurrentValue != NewValue)
        {
            CurrentValue = NewValue;
            bIsDirty = true;
            OnSettingChanged.Broadcast(this);

            return true;
        }

        return false;
    }

    bool bIsDirty = false;
};
