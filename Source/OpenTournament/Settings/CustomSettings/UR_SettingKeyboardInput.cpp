// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_SettingKeyboardInput.h"

#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include "Player/UR_LocalPlayer.h"
#include "Settings/UR_SettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_SettingKeyboardInput)

/////////////////////////////////////////////////////////////////////////////////////////////////

class ULocalPlayer;

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTSettings"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace OT::ErrorMessages
{
    static const FText UnknownMappingName = LOCTEXT("OTErrors_UnknownMappingName", "Unknown Mapping");
}


UUR_SettingKeyboardInput::UUR_SettingKeyboardInput()
{
    bReportAnalytics = false;
}

FText UUR_SettingKeyboardInput::GetSettingDisplayName() const
{
    if (const FKeyMappingRow* Row = FindKeyMappingRow())
    {
        if (Row->HasAnyMappings())
        {
            return Row->Mappings.begin()->GetDisplayName();
        }
    }

    return OT::ErrorMessages::UnknownMappingName;
}

FText UUR_SettingKeyboardInput::GetSettingDisplayCategory() const
{
    if (const FKeyMappingRow* Row = FindKeyMappingRow())
    {
        if (Row->HasAnyMappings())
        {
            return Row->Mappings.begin()->GetDisplayCategory();
        }
    }

    return OT::ErrorMessages::UnknownMappingName;
}

const FKeyMappingRow* UUR_SettingKeyboardInput::FindKeyMappingRow() const
{
    if (const UEnhancedPlayerMappableKeyProfile* Profile = FindMappableKeyProfile())
    {
        return Profile->FindKeyMappingRow(ActionMappingName);
    }

    ensure(false);
    return nullptr;
}

UEnhancedPlayerMappableKeyProfile* UUR_SettingKeyboardInput::FindMappableKeyProfile() const
{
    if (UEnhancedInputUserSettings* Settings = GetUserSettings())
    {
        return Settings->GetKeyProfileWithId(ProfileIdentifier);
    }

    ensure(false);
    return nullptr;
}

UEnhancedInputUserSettings* UUR_SettingKeyboardInput::GetUserSettings() const
{
    if (UUR_LocalPlayer* GameLocalPlayer = Cast<UUR_LocalPlayer>(LocalPlayer))
    {
        // Map the key to the player key profile
        if (UEnhancedInputLocalPlayerSubsystem* System = GameLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            return System->GetUserSettings();
        }
    }

    return nullptr;
}

void UUR_SettingKeyboardInput::OnInitialized()
{
    DynamicDetails = FGetGameSettingsDetails::CreateLambda
    ([this](ULocalPlayer&)
    {
        if (const FKeyMappingRow* Row = FindKeyMappingRow())
        {
            if (Row->HasAnyMappings())
            {
                return FText::Format(LOCTEXT("DynamicDetails_KeyboardInputAction", "Bindings for {0}"), Row->Mappings.begin()->GetDisplayName());
            }
        }
        return FText::GetEmpty();
    });

    Super::OnInitialized();
}

void UUR_SettingKeyboardInput::InitializeInputData(const UEnhancedPlayerMappableKeyProfile* KeyProfile, const FKeyMappingRow& MappingData, const FPlayerMappableKeyQueryOptions& InQueryOptions)
{
    check(KeyProfile);

    ProfileIdentifier = KeyProfile->GetProfileIdString();
    QueryOptions = InQueryOptions;

    for (const FPlayerKeyMapping& Mapping : MappingData.Mappings)
    {
        // Only add mappings that pass the query filters that have been provided upon creation
        if (!KeyProfile->DoesMappingPassQueryOptions(Mapping, QueryOptions))
        {
            continue;
        }

        ActionMappingName = Mapping.GetMappingName();
        InitialKeyMappings.Add(Mapping.GetSlot(), Mapping.GetCurrentKey());
        const FText& MappingDisplayName = Mapping.GetDisplayName();

        if (!MappingDisplayName.IsEmpty())
        {
            SetDisplayName(MappingDisplayName);
        }
    }

    const FString NameString = TEXT("KBM_Input_") + ActionMappingName.ToString();
    SetDevName(*NameString);
}

FText UUR_SettingKeyboardInput::GetKeyTextFromSlot(const EPlayerMappableKeySlot InSlot) const
{
    if (const UEnhancedPlayerMappableKeyProfile* Profile = FindMappableKeyProfile())
    {
        FPlayerMappableKeyQueryOptions QueryOptionsForSlot = QueryOptions;
        QueryOptionsForSlot.SlotToMatch = InSlot;

        if (const FKeyMappingRow* Row = FindKeyMappingRow())
        {
            for (const FPlayerKeyMapping& Mapping : Row->Mappings)
            {
                if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptionsForSlot))
                {
                    return Mapping.GetCurrentKey().GetDisplayName();
                }
            }
        }
    }

    return EKeys::Invalid.GetDisplayName();
}

void UUR_SettingKeyboardInput::ResetToDefault()
{
    if (UEnhancedInputUserSettings* Settings = GetUserSettings())
    {
        FMapPlayerKeyArgs Args = { };
        Args.MappingName = ActionMappingName;

        FGameplayTagContainer FailureReason;
        Settings->ResetAllPlayerKeysInRow(Args, FailureReason);

        NotifySettingChanged(EGameSettingChangeReason::Change);
    }
}

void UUR_SettingKeyboardInput::StoreInitial()
{
    if (const UEnhancedPlayerMappableKeyProfile* Profile = FindMappableKeyProfile())
    {
        if (const FKeyMappingRow* Row = FindKeyMappingRow())
        {
            for (const FPlayerKeyMapping& Mapping : Row->Mappings)
            {
                if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptions))
                {
                    ActionMappingName = Mapping.GetMappingName();
                    InitialKeyMappings.Add(Mapping.GetSlot(), Mapping.GetCurrentKey());
                }
            }
        }
    }
}

void UUR_SettingKeyboardInput::RestoreToInitial()
{
    for (TPair<EPlayerMappableKeySlot, FKey> Pair : InitialKeyMappings)
    {
        ChangeBinding(static_cast<int32>(Pair.Key), Pair.Value);
    }
}

bool UUR_SettingKeyboardInput::ChangeBinding(int32 InKeyBindSlot, FKey NewKey)
{
    if (!NewKey.IsGamepadKey())
    {
        FMapPlayerKeyArgs Args = { };
        Args.MappingName = ActionMappingName;
        Args.Slot = static_cast<EPlayerMappableKeySlot>(static_cast<uint8>(InKeyBindSlot));
        Args.NewKey = NewKey;
        // If you want to, you can additionally specify this mapping to only be applied to a certain hardware device or key profile
        //Args.ProfileId =
        //Args.HardwareDeviceId =

        if (UEnhancedInputUserSettings* Settings = GetUserSettings())
        {
            FGameplayTagContainer FailureReason;
            Settings->MapPlayerKey(Args, FailureReason);
            NotifySettingChanged(EGameSettingChangeReason::Change);
        }

        return true;
    }

    return false;
}

void UUR_SettingKeyboardInput::GetAllMappedActionsFromKey(int32 InKeyBindSlot, FKey Key, TArray<FName>& OutActionNames) const
{
    if (const UEnhancedPlayerMappableKeyProfile* Profile = FindMappableKeyProfile())
    {
        Profile->GetMappingNamesForKey(Key, OutActionNames);
    }
}

bool UUR_SettingKeyboardInput::IsMappingCustomized() const
{
    bool bResult = false;

    if (const UEnhancedPlayerMappableKeyProfile* Profile = FindMappableKeyProfile())
    {
        FPlayerMappableKeyQueryOptions QueryOptionsForSlot = QueryOptions;

        if (const FKeyMappingRow* Row = FindKeyMappingRow())
        {
            for (const FPlayerKeyMapping& Mapping : Row->Mappings)
            {
                if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptionsForSlot))
                {
                    bResult |= Mapping.IsCustomized();
                }
            }
        }
    }

    return bResult;
}

#undef LOCTEXT_NAMESPACE
