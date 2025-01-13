// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_SettingsShared.h"

#include "EnhancedInputSubsystems.h"
#include "SubtitleDisplaySubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Internationalization/Culture.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Rendering/SlateRenderer.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include "Player/UR_LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_SettingsShared)

/////////////////////////////////////////////////////////////////////////////////////////////////

static FString SHARED_SETTINGS_SLOT_NAME = TEXT("SharedGameSettings");

namespace GameSettingsSharedCVars
{
    static float DefaultGamepadLeftStickInnerDeadZone = 0.25f;
    static FAutoConsoleVariableRef CVarGamepadLeftStickInnerDeadZone
    (
        TEXT("gpad.DefaultLeftStickInnerDeadZone"),
        DefaultGamepadLeftStickInnerDeadZone,
        TEXT("Gamepad left stick inner deadzone")
    );

    static float DefaultGamepadRightStickInnerDeadZone = 0.25f;
    static FAutoConsoleVariableRef CVarGamepadRightStickInnerDeadZone
    (
        TEXT("gpad.DefaultRightStickInnerDeadZone"),
        DefaultGamepadRightStickInnerDeadZone,
        TEXT("Gamepad right stick inner deadzone")
    );
}

UUR_SettingsShared::UUR_SettingsShared()
{
    FInternationalization::Get().OnCultureChanged().AddUObject(this, &ThisClass::OnCultureChanged);

    GamepadMoveStickDeadZone = GameSettingsSharedCVars::DefaultGamepadLeftStickInnerDeadZone;
    GamepadLookStickDeadZone = GameSettingsSharedCVars::DefaultGamepadRightStickInnerDeadZone;
}

int32 UUR_SettingsShared::GetLatestDataVersion() const
{
    // 0 = before subclassing ULocalPlayerSaveGame
    // 1 = first proper version
    return 1;
}

UUR_SettingsShared* UUR_SettingsShared::CreateTemporarySettings(const UUR_LocalPlayer* LocalPlayer)
{
    // This is not loaded from disk but should be set up to save
    UUR_SettingsShared* SharedSettings = Cast<UUR_SettingsShared>(CreateNewSaveGameForLocalPlayer(UUR_SettingsShared::StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME));

    SharedSettings->ApplySettings();

    return SharedSettings;
}

UUR_SettingsShared* UUR_SettingsShared::LoadOrCreateSettings(const UUR_LocalPlayer* LocalPlayer)
{
    // This will stall the main thread while it loads
    UUR_SettingsShared* SharedSettings = Cast<UUR_SettingsShared>(LoadOrCreateSaveGameForLocalPlayer(UUR_SettingsShared::StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME));

    SharedSettings->ApplySettings();

    return SharedSettings;
}

bool UUR_SettingsShared::AsyncLoadOrCreateSettings(const UUR_LocalPlayer* LocalPlayer, FOnSettingsLoadedEvent Delegate)
{
    FOnLocalPlayerSaveGameLoadedNative Lambda = FOnLocalPlayerSaveGameLoadedNative::CreateLambda
    ([Delegate]
    (ULocalPlayerSaveGame* LoadedSave)
        {
            UUR_SettingsShared* LoadedSettings = CastChecked<UUR_SettingsShared>(LoadedSave);

            LoadedSettings->ApplySettings();

            Delegate.ExecuteIfBound(LoadedSettings);
        });

    return ULocalPlayerSaveGame::AsyncLoadOrCreateSaveGameForLocalPlayer(UUR_SettingsShared::StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME, Lambda);
}

void UUR_SettingsShared::SaveSettings()
{
    // Schedule an async save because it's okay if it fails
    AsyncSaveGameToSlotForLocalPlayer();

    // TODO_BH: Move this to the serialize function instead with a bumped version number
    if (UEnhancedInputLocalPlayerSubsystem* System = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningPlayer))
    {
        if (UEnhancedInputUserSettings* InputSettings = System->GetUserSettings())
        {
            InputSettings->AsyncSaveSettings();
        }
    }
}

void UUR_SettingsShared::ApplySettings()
{
    ApplySubtitleOptions();
    ApplyBackgroundAudioSettings();
    ApplyCultureSettings();

    if (UEnhancedInputLocalPlayerSubsystem* System = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningPlayer))
    {
        if (UEnhancedInputUserSettings* InputSettings = System->GetUserSettings())
        {
            InputSettings->ApplySettings();
        }
    }
}

void UUR_SettingsShared::SetColorBlindStrength(int32 InColorBlindStrength)
{
    InColorBlindStrength = FMath::Clamp(InColorBlindStrength, 0, 10);
    if (ColorBlindStrength != InColorBlindStrength)
    {
        ColorBlindStrength = InColorBlindStrength;
        FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType
        (
            (EColorVisionDeficiency)(int32)ColorBlindMode,
            (int32)ColorBlindStrength,
            true,
            false);
    }
}

int32 UUR_SettingsShared::GetColorBlindStrength() const
{
    return ColorBlindStrength;
}

void UUR_SettingsShared::SetColorBlindMode(EColorBlindMode InMode)
{
    if (ColorBlindMode != InMode)
    {
        ColorBlindMode = InMode;
        FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType
        (
            (EColorVisionDeficiency)(int32)ColorBlindMode,
            (int32)ColorBlindStrength,
            true,
            false);
    }
}

EColorBlindMode UUR_SettingsShared::GetColorBlindMode() const
{
    return ColorBlindMode;
}

void UUR_SettingsShared::ApplySubtitleOptions()
{
    if (USubtitleDisplaySubsystem* SubtitleSystem = USubtitleDisplaySubsystem::Get(OwningPlayer))
    {
        FSubtitleFormat SubtitleFormat;
        SubtitleFormat.SubtitleTextSize = SubtitleTextSize;
        SubtitleFormat.SubtitleTextColor = SubtitleTextColor;
        SubtitleFormat.SubtitleTextBorder = SubtitleTextBorder;
        SubtitleFormat.SubtitleBackgroundOpacity = SubtitleBackgroundOpacity;

        SubtitleSystem->SetSubtitleDisplayOptions(SubtitleFormat);
    }
}

//////////////////////////////////////////////////////////////////////

void UUR_SettingsShared::SetAllowAudioInBackgroundSetting(EGameAllowBackgroundAudioSetting NewValue)
{
    if (ChangeValueAndDirty(AllowAudioInBackground, NewValue))
    {
        ApplyBackgroundAudioSettings();
    }
}

void UUR_SettingsShared::ApplyBackgroundAudioSettings()
{
    if (OwningPlayer && OwningPlayer->IsPrimaryPlayer())
    {
        FApp::SetUnfocusedVolumeMultiplier((AllowAudioInBackground != EGameAllowBackgroundAudioSetting::Off) ? 1.0f : 0.0f);
    }
}

//////////////////////////////////////////////////////////////////////

void UUR_SettingsShared::ApplyCultureSettings()
{
    if (bResetToDefaultCulture)
    {
        const FCulturePtr SystemDefaultCulture = FInternationalization::Get().GetDefaultCulture();
        check(SystemDefaultCulture.IsValid());

        const FString CultureToApply = SystemDefaultCulture->GetName();
        if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
        {
            // Clear this string
            GConfig->RemoveKey(TEXT("Internationalization"), TEXT("Culture"), GGameUserSettingsIni);
            GConfig->Flush(false, GGameUserSettingsIni);
        }
        bResetToDefaultCulture = false;
    }
    else if (!PendingCulture.IsEmpty())
    {
        // SetCurrentCulture may trigger PendingCulture to be cleared (if a culture change is broadcast) so we take a copy of it to work with
        const FString CultureToApply = PendingCulture;
        if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
        {
            // Note: This is intentionally saved to the users config
            // We need to localize text before the player logs in and very early in the loading screen
            GConfig->SetString(TEXT("Internationalization"), TEXT("Culture"), *CultureToApply, GGameUserSettingsIni);
            GConfig->Flush(false, GGameUserSettingsIni);
        }
        ClearPendingCulture();
    }
}

void UUR_SettingsShared::ResetCultureToCurrentSettings()
{
    ClearPendingCulture();
    bResetToDefaultCulture = false;
}

const FString& UUR_SettingsShared::GetPendingCulture() const
{
    return PendingCulture;
}

void UUR_SettingsShared::SetPendingCulture(const FString& NewCulture)
{
    PendingCulture = NewCulture;
    bResetToDefaultCulture = false;
    bIsDirty = true;
}

void UUR_SettingsShared::OnCultureChanged()
{
    ClearPendingCulture();
    bResetToDefaultCulture = false;
}

void UUR_SettingsShared::ClearPendingCulture()
{
    PendingCulture.Reset();
}

bool UUR_SettingsShared::IsUsingDefaultCulture() const
{
    FString Culture;
    GConfig->GetString(TEXT("Internationalization"), TEXT("Culture"), Culture, GGameUserSettingsIni);

    return Culture.IsEmpty();
}

void UUR_SettingsShared::ResetToDefaultCulture()
{
    ClearPendingCulture();
    bResetToDefaultCulture = true;
    bIsDirty = true;
}

//////////////////////////////////////////////////////////////////////

void UUR_SettingsShared::ApplyInputSensitivity()
{
}
