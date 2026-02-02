// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"

#include "UR_AudioSettings.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
class USoundEffectSubmixPreset;
class USoundSubmix;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct OPENTOURNAMENT_API FGameSubmixEffectChainMap
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, meta = (AllowedClasses = "/Script/Engine.SoundSubmix"))
    TSoftObjectPtr<USoundSubmix> Submix = nullptr;

    UPROPERTY(EditAnywhere, meta = (AllowedClasses = "/Script/Engine.SoundEffectSubmixPreset"))
    TArray<TSoftObjectPtr<USoundEffectSubmixPreset>> SubmixEffectChain;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Game AudioSettings"))
class OPENTOURNAMENT_API UUR_AudioSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** The Default Base Control Bus Mix */
    UPROPERTY(config, EditAnywhere, Category = MixSettings, meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBusMix"))
    FSoftObjectPath DefaultControlBusMix;

    /** The Loading Screen Control Bus Mix - Called during loading screens to cover background audio events */
    UPROPERTY(config, EditAnywhere, Category = MixSettings, meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBusMix"))
    FSoftObjectPath LoadingScreenControlBusMix;

    /** The Default Base Control Bus Mix */
    UPROPERTY(config, EditAnywhere, Category = UserMixSettings, meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBusMix"))
    FSoftObjectPath UserSettingsControlBusMix;

    /** Control Bus assigned to the Overall sound volume setting */
    UPROPERTY(config, EditAnywhere, Category = UserMixSettings, meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
    FSoftObjectPath OverallVolumeControlBus;

    /** Control Bus assigned to the Music sound volume setting */
    UPROPERTY(config, EditAnywhere, Category = UserMixSettings, meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
    FSoftObjectPath MusicVolumeControlBus;

    /** Control Bus assigned to the SoundFX sound volume setting */
    UPROPERTY(config, EditAnywhere, Category = UserMixSettings, meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
    FSoftObjectPath SoundFXVolumeControlBus;

    /** Control Bus assigned to the Dialogue sound volume setting */
    UPROPERTY(config, EditAnywhere, Category = UserMixSettings, meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
    FSoftObjectPath DialogueVolumeControlBus;

    /** Control Bus assigned to the VoiceChat sound volume setting */
    UPROPERTY(config, EditAnywhere, Category = UserMixSettings, meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
    FSoftObjectPath VoiceChatVolumeControlBus;

    /** Submix Processing Chains to achieve high dynamic range audio output */
    UPROPERTY(config, EditAnywhere, Category = EffectSettings)
    TArray<FGameSubmixEffectChainMap> HDRAudioSubmixEffectChain;

    /** Submix Processing Chains to achieve low dynamic range audio output */
    UPROPERTY(config, EditAnywhere, Category = EffectSettings)
    TArray<FGameSubmixEffectChainMap> LDRAudioSubmixEffectChain;
};
