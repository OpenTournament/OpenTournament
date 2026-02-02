// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_SettingValueDiscreteDynamic_AudioOutputDevice.h"

#include "AudioDeviceNotificationSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_SettingValueDiscreteDynamic_AudioOutputDevice)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTSettings"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_SettingValueDiscreteDynamic_AudioOutputDevice::OnInitialized()
{
    Super::OnInitialized();

    DevicesObtainedCallback.BindUFunction(this, FName("OnAudioOutputDevicesObtained"));
    DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedDeviceSwap"));

    if (UAudioDeviceNotificationSubsystem* AudioDeviceNotifSubsystem = UAudioDeviceNotificationSubsystem::Get())
    {
        AudioDeviceNotifSubsystem->DeviceAddedNative.AddUObject(this, &ThisClass::DeviceAddedOrRemoved);
        AudioDeviceNotifSubsystem->DeviceRemovedNative.AddUObject(this, &ThisClass::DeviceAddedOrRemoved);
        //AudioDeviceNotifSubsystem->DeviceSwitchedNative.AddUObject(this, &ThisClass::DeviceSwitched);
        AudioDeviceNotifSubsystem->DefaultRenderDeviceChangedNative.AddUObject(this, &ThisClass::DefaultDeviceChanged);
    }

    UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesObtainedCallback);
}

void UUR_SettingValueDiscreteDynamic_AudioOutputDevice::OnAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices)
{
    int32 NewSize = AvailableDevices.Num();
    OutputDevices.Reset(NewSize++);
    OutputDevices.Append(AvailableDevices);

    OptionValues.Reset(NewSize);
    OptionDisplayTexts.Reset(NewSize);

    // Placeholder - needs to be first option so we can format the default device string later
    AddDynamicOption(TEXT(""), FText::GetEmpty());
    FString SystemDefaultDeviceName;

    for (const FAudioOutputDeviceInfo& DeviceInfo : OutputDevices)
    {
        if (!DeviceInfo.DeviceId.IsEmpty() && !DeviceInfo.Name.IsEmpty())
        {
            // System Default
            if (DeviceInfo.bIsSystemDefault)
            {
                SystemDefaultDeviceId = DeviceInfo.DeviceId;
                SystemDefaultDeviceName = DeviceInfo.Name;
            }

            // Current Device
            if (DeviceInfo.bIsCurrentDevice)
            {
                CurrentDeviceId = DeviceInfo.DeviceId;
            }

            // Add the menu option
            AddDynamicOption(DeviceInfo.DeviceId, FText::FromString(DeviceInfo.Name));
        }
    }

    OptionDisplayTexts[0] = FText::Format(LOCTEXT("DefaultAudioOutputDevice", "Default Output - {0}"), FText::FromString(SystemDefaultDeviceName));
    SetDefaultValueFromString(TEXT(""));
    RefreshEditableState();

    //LastKnownGoodIndex = GetDiscreteOptionDefaultIndex();
    //SetDiscreteOptionByIndex(GetDiscreteOptionIndex());
}

void UUR_SettingValueDiscreteDynamic_AudioOutputDevice::OnCompletedDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
    //if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
    //{
    //	UE_LOG(LogGame, VeryVerbose, TEXT("AudioOutputDevice failure! Resetting to: %s"), *(OptionDisplayTexts[LastKnownGoodIndex].ToString()));
    //	if (OptionValues.Num() < LastKnownGoodIndex && SwapResult.RequestedDeviceId != OptionValues[LastKnownGoodIndex])
    //	{
    //		SetDiscreteOptionByIndex(LastKnownGoodIndex);
    //	}

    //	// Remove the invalid device
    //	if (SwapResult.RequestedDeviceId != SystemDefaultDeviceId)
    //	{
    //		OutputDevices.RemoveAll([&SwapResult](FAudioOutputDeviceInfo& Device)
    //		{
    //			return Device.DeviceId == SwapResult.RequestedDeviceId;
    //		});

    //		RemoveDynamicOption(SwapResult.RequestedDeviceId);
    //		RefreshEditableState();
    //	}
    //}
}

void UUR_SettingValueDiscreteDynamic_AudioOutputDevice::DeviceAddedOrRemoved(FString DeviceId)
{
    UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesObtainedCallback);
}

void UUR_SettingValueDiscreteDynamic_AudioOutputDevice::DefaultDeviceChanged(EAudioDeviceChangedRole InRole, FString DeviceId)
{
    UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesObtainedCallback);
}

void UUR_SettingValueDiscreteDynamic_AudioOutputDevice::SetDiscreteOptionByIndex(int32 Index)
{
    Super::SetDiscreteOptionByIndex(Index);
    //UE_LOG(LogGame, VeryVerbose, TEXT("AudioOutputDevice set to %s - %s"), *(OptionDisplayTexts[Index].ToString()), *OptionValues[Index]);
    //bRequestDefault = false;

    //FString RequestedAudioDeviceId = GetValueAsString();

    //// Grab the correct deviceId if the user has selected default
    //const int32 DefaultOptionIndex = GetDiscreteOptionDefaultIndex();
    //if (Index == DefaultOptionIndex)
    //{
    //	RequestedAudioDeviceId = SystemDefaultDeviceId;
    //}

    //// Only swap if the requested deviceId is different than our current
    //if (RequestedAudioDeviceId == CurrentDeviceId)
    //{
    //	LastKnownGoodIndex = Index;
    //	UE_LOG(LogGame, VeryVerbose, TEXT("AudioOutputDevice (Not Swapping) - LKG set to index :%d"), LastKnownGoodIndex);
    //}
    //else
    //{
    //	bRequestDefault = (Index == DefaultOptionIndex);
    //	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(LocalPlayer, RequestedAudioDeviceId, DevicesSwappedCallback);
    //	UE_LOG(LogGame, VeryVerbose, TEXT("AudioOutputDevice requesting %s"), *RequestedAudioDeviceId);
    //}
}

#undef LOCTEXT_NAMESPACE
