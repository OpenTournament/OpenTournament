// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "CommonInputBaseTypes.h"
#include "EnhancedInputSubsystems.h"

#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "DataSource/GameSettingDataSource.h"
#include "EditCondition/WhenCondition.h"

#include "PlayerMappableInputConfig.h"
#include "UR_GameSettingRegistry.h"
#include "UR_SettingsLocal.h"
#include "UR_SettingsShared.h"
#include "CustomSettings/UR_SettingKeyboardInput.h"
#include "Player/UR_LocalPlayer.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class ULocalPlayer;

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OT"

/////////////////////////////////////////////////////////////////////////////////////////////////

UGameSettingCollection* UUR_GameSettingRegistry::InitializeMouseAndKeyboardSettings(UUR_LocalPlayer* InLocalPlayer)
{
    UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
    Screen->SetDevName(TEXT("MouseAndKeyboardCollection"));
    Screen->SetDisplayName(LOCTEXT("MouseAndKeyboardCollection_Name", "Mouse & Keyboard"));
    Screen->Initialize(InLocalPlayer);

    const TSharedRef<FWhenCondition> WhenPlatformSupportsMouseAndKeyboard = MakeShared<FWhenCondition>
    (
        [](const ULocalPlayer*, FGameSettingEditableState& InOutEditState)
        {
            const UCommonInputPlatformSettings* PlatformInput = UPlatformSettingsManager::Get().GetSettingsForPlatform<UCommonInputPlatformSettings>();
            if (!PlatformInput->SupportsInputType(ECommonInputType::MouseAndKeyboard))
            {
                InOutEditState.Kill(TEXT("Platform does not support mouse and keyboard"));
            }
        });

    // Mouse Sensitivity
    ////////////////////////////////////////////////////////////////////////////////////
    {
        UGameSettingCollection* Sensitivity = NewObject<UGameSettingCollection>();
        Sensitivity->SetDevName(TEXT("MouseSensitivityCollection"));
        Sensitivity->SetDisplayName(LOCTEXT("MouseSensitivityCollection_Name", "Sensitivity"));
        Screen->AddSetting(Sensitivity);

        //----------------------------------------------------------------------------------
        {
            UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
            Setting->SetDevName(TEXT("MouseSensitivityYaw"));
            Setting->SetDisplayName(LOCTEXT("MouseSensitivityYaw_Name", "X-Axis Sensitivity"));
            Setting->SetDescriptionRichText(LOCTEXT("MouseSensitivityYaw_Description", "Sets the sensitivity of the mouse's horizontal (x) axis. With higher settings the camera will move faster when looking left and right with the mouse."));

            Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityX));
            Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityX));
            Setting->SetDefaultValue(GetDefault<UUR_SettingsShared>()->GetMouseSensitivityX());
            Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
            Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.01);
            Setting->SetMinimumLimit(0.01);

            Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

            Sensitivity->AddSetting(Setting);
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
            Setting->SetDevName(TEXT("MouseSensitivityPitch"));
            Setting->SetDisplayName(LOCTEXT("MouseSensitivityPitch_Name", "Y-Axis Sensitivity"));
            Setting->SetDescriptionRichText(LOCTEXT("MouseSensitivityPitch_Description", "Sets the sensitivity of the mouse's vertical (y) axis. With higher settings the camera will move faster when looking up and down with the mouse."));

            Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityY));
            Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityY));
            Setting->SetDefaultValue(GetDefault<UUR_SettingsShared>()->GetMouseSensitivityY());
            Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
            Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.01);
            Setting->SetMinimumLimit(0.01);

            Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

            Sensitivity->AddSetting(Setting);
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
            Setting->SetDevName(TEXT("MouseTargetingMultiplier"));
            Setting->SetDisplayName(LOCTEXT("MouseTargetingMultiplier_Name", "Targeting Sensitivity"));
            Setting->SetDescriptionRichText(LOCTEXT("MouseTargetingMultiplier_Description", "Sets the modifier for reducing mouse sensitivity when targeting. 100% will have no slow down when targeting. Lower settings will have more slow down when targeting."));

            Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetTargetingMultiplier));
            Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetTargetingMultiplier));
            Setting->SetDefaultValue(GetDefault<UUR_SettingsShared>()->GetTargetingMultiplier());
            Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
            Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.01);
            Setting->SetMinimumLimit(0.01);

            Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

            Sensitivity->AddSetting(Setting);
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
            Setting->SetDevName(TEXT("InvertVerticalAxis"));
            Setting->SetDisplayName(LOCTEXT("InvertVerticalAxis_Name", "Invert Vertical Axis"));
            Setting->SetDescriptionRichText(LOCTEXT("InvertVerticalAxis_Description", "Enable the inversion of the vertical look axis."));

            Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertVerticalAxis));
            Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertVerticalAxis));
            Setting->SetDefaultValue(GetDefault<UUR_SettingsShared>()->GetInvertVerticalAxis());

            Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

            Sensitivity->AddSetting(Setting);
        }
        //----------------------------------------------------------------------------------
        {
            UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
            Setting->SetDevName(TEXT("InvertHorizontalAxis"));
            Setting->SetDisplayName(LOCTEXT("InvertHorizontalAxis_Name", "Invert Horizontal Axis"));
            Setting->SetDescriptionRichText(LOCTEXT("InvertHorizontalAxis_Description", "Enable the inversion of the Horizontal look axis."));

            Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertHorizontalAxis));
            Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertHorizontalAxis));
            Setting->SetDefaultValue(GetDefault<UUR_SettingsShared>()->GetInvertHorizontalAxis());

            Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

            Sensitivity->AddSetting(Setting);
        }
        //----------------------------------------------------------------------------------
    }

    // Bindings for Mouse & Keyboard - Automatically Generated
    ////////////////////////////////////////////////////////////////////////////////////
    {
        UGameSettingCollection* KeyBinding = NewObject<UGameSettingCollection>();
        KeyBinding->SetDevName(TEXT("KeyBindingCollection"));
        KeyBinding->SetDisplayName(LOCTEXT("KeyBindingCollection_Name", "Keyboard & Mouse"));
        Screen->AddSetting(KeyBinding);

        const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSystem = InLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
        const UEnhancedInputUserSettings* UserSettings = EnhancedInputSystem->GetUserSettings();

        // If you want to just get one profile pair, then you can do UserSettings->GetCurrentProfile

        // A map of key bindings mapped to their display category
        TMap<FString, UGameSettingCollection*> CategoryToSettingCollection;

        // Returns an existing setting collection for the display category if there is one.
        // If there isn't one, then it will create a new one and initialize it
        auto GetOrCreateSettingCollection = [&CategoryToSettingCollection, &Screen](FText DisplayCategory) -> UGameSettingCollection*
        {
            static const FString DefaultDevName = TEXT("Default_KBM");
            static const FText DefaultDevDisplayName = NSLOCTEXT("GameInputSettings", "GameInputDefaults", "Default Experiences");

            if (DisplayCategory.IsEmpty())
            {
                DisplayCategory = DefaultDevDisplayName;
            }

            FString DisplayCatString = DisplayCategory.ToString();

            if (UGameSettingCollection** ExistingCategory = CategoryToSettingCollection.Find(DisplayCatString))
            {
                return *ExistingCategory;
            }

            UGameSettingCollection* ConfigSettingCollection = NewObject<UGameSettingCollection>();
            ConfigSettingCollection->SetDevName(FName(DisplayCatString));
            ConfigSettingCollection->SetDisplayName(DisplayCategory);
            Screen->AddSetting(ConfigSettingCollection);
            CategoryToSettingCollection.Add(DisplayCatString, ConfigSettingCollection);

            return ConfigSettingCollection;
        };

        static TSet<FName> CreatedMappingNames;
        CreatedMappingNames.Reset();

        for (const TPair<FString, TObjectPtr<UEnhancedPlayerMappableKeyProfile>>& ProfilePair : UserSettings->GetAllAvailableKeyProfiles())
        {
            const FString& ProfileName = ProfilePair.Key;
            const TObjectPtr<UEnhancedPlayerMappableKeyProfile>& Profile = ProfilePair.Value;

            for (const TPair<FName, FKeyMappingRow>& RowPair : Profile->GetPlayerMappingRows())
            {
                // Create a setting row for anything with valid mappings and that we haven't created yet
                if (RowPair.Value.HasAnyMappings() /* && !CreatedMappingNames.Contains(RowPair.Key)*/)
                {
                    // We only want keyboard keys on this settings screen, so we will filter down by mappings
                    // that are set to keyboard keys
                    FPlayerMappableKeyQueryOptions Options = { };
                    Options.KeyToMatch = EKeys::W;
                    Options.bMatchBasicKeyTypes = true;

                    const FText& DesiredDisplayCategory = RowPair.Value.Mappings.begin()->GetDisplayCategory();

                    if (UGameSettingCollection* Collection = GetOrCreateSettingCollection(DesiredDisplayCategory))
                    {
                        // Create the settings widget and initialize it, adding it to this config's section
                        UUR_SettingKeyboardInput* InputBinding = NewObject<UUR_SettingKeyboardInput>();

                        InputBinding->InitializeInputData(Profile, RowPair.Value, Options);
                        InputBinding->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

                        Collection->AddSetting(InputBinding);
                        CreatedMappingNames.Add(RowPair.Key);
                    }
                    else
                    {
                        ensure(false);
                    }
                }
            }
        }
    }

    return Screen;
}

#undef LOCTEXT_NAMESPACE
