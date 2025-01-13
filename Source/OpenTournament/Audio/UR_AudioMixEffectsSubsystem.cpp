// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Audio/UR_AudioMixEffectsSubsystem.h"

#include "AudioMixerBlueprintLibrary.h"
#include "AudioModulationStatics.h"
#include "LoadingScreenManager.h"
#include "SoundControlBus.h"
#include "SoundControlBusMix.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Sound/SoundEffectSubmix.h"

#include "UR_AudioSettings.h"
#include "Settings/UR_SettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AudioMixEffectsSubsystem)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FSubsystemCollectionBase;

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AudioMixEffectsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UUR_AudioMixEffectsSubsystem::Deinitialize()
{
    if (ULoadingScreenManager* LoadingScreenManager = UGameInstance::GetSubsystem<ULoadingScreenManager>(GetWorld()->GetGameInstance()))
    {
        LoadingScreenManager->OnLoadingScreenVisibilityChangedDelegate().RemoveAll(this);
        ApplyOrRemoveLoadingScreenMix(false);
    }

    Super::Deinitialize();
}

bool UUR_AudioMixEffectsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    bool bShouldCreateSubsystem = Super::ShouldCreateSubsystem(Outer);

    if (Outer)
    {
        if (const UWorld* World = Outer->GetWorld())
        {
            bShouldCreateSubsystem = DoesSupportWorldType(World->WorldType) && bShouldCreateSubsystem;
        }
    }

    return bShouldCreateSubsystem;
}

void UUR_AudioMixEffectsSubsystem::PostInitialize()
{
    if (const UUR_AudioSettings* GameAudioSettings = GetDefault<UUR_AudioSettings>())
    {
        if (UObject* ObjPath = GameAudioSettings->DefaultControlBusMix.TryLoad())
        {
            if (USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath))
            {
                DefaultBaseMix = SoundControlBusMix;
            }
            else
            {
                ensureMsgf(SoundControlBusMix, TEXT("Default Control Bus Mix reference missing from Game Audio Settings."));
            }
        }

        if (UObject* ObjPath = GameAudioSettings->LoadingScreenControlBusMix.TryLoad())
        {
            if (USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath))
            {
                LoadingScreenMix = SoundControlBusMix;
            }
            else
            {
                ensureMsgf(SoundControlBusMix, TEXT("Loading Screen Control Bus Mix reference missing from Game Audio Settings."));
            }
        }

        if (UObject* ObjPath = GameAudioSettings->UserSettingsControlBusMix.TryLoad())
        {
            if (USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath))
            {
                UserMix = SoundControlBusMix;
            }
            else
            {
                ensureMsgf(SoundControlBusMix, TEXT("User Control Bus Mix reference missing from Game Audio Settings."));
            }
        }

        if (UObject* ObjPath = GameAudioSettings->OverallVolumeControlBus.TryLoad())
        {
            if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
            {
                OverallControlBus = SoundControlBus;
            }
            else
            {
                ensureMsgf(SoundControlBus, TEXT("Overall Control Bus reference missing from Game Audio Settings."));
            }
        }

        if (UObject* ObjPath = GameAudioSettings->MusicVolumeControlBus.TryLoad())
        {
            if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
            {
                MusicControlBus = SoundControlBus;
            }
            else
            {
                ensureMsgf(SoundControlBus, TEXT("Music Control Bus reference missing from Game Audio Settings."));
            }
        }

        if (UObject* ObjPath = GameAudioSettings->SoundFXVolumeControlBus.TryLoad())
        {
            if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
            {
                SoundFXControlBus = SoundControlBus;
            }
            else
            {
                ensureMsgf(SoundControlBus, TEXT("SoundFX Control Bus reference missing from Game Audio Settings."));
            }
        }

        if (UObject* ObjPath = GameAudioSettings->DialogueVolumeControlBus.TryLoad())
        {
            if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
            {
                DialogueControlBus = SoundControlBus;
            }
            else
            {
                ensureMsgf(SoundControlBus, TEXT("Dialogue Control Bus reference missing from Game Audio Settings."));
            }
        }

        if (UObject* ObjPath = GameAudioSettings->VoiceChatVolumeControlBus.TryLoad())
        {
            if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
            {
                VoiceChatControlBus = SoundControlBus;
            }
            else
            {
                ensureMsgf(SoundControlBus, TEXT("VoiceChat Control Bus reference missing from Game Audio Settings."));
            }
        }

        // Load HDR Submix Effect Chain
        for (const FGameSubmixEffectChainMap& SoftSubmixEffectChain : GameAudioSettings->HDRAudioSubmixEffectChain)
        {
            FGameAudioSubmixEffectsChain NewEffectChain;

            if (UObject* SubmixObjPath = SoftSubmixEffectChain.Submix.LoadSynchronous())
            {
                if (USoundSubmix* Submix = Cast<USoundSubmix>(SubmixObjPath))
                {
                    NewEffectChain.Submix = Submix;
                    TArray<USoundEffectSubmixPreset*> NewPresetChain;

                    for (const TSoftObjectPtr<USoundEffectSubmixPreset>& SoftEffect : SoftSubmixEffectChain.SubmixEffectChain)
                    {
                        if (UObject* EffectObjPath = SoftEffect.LoadSynchronous())
                        {
                            if (USoundEffectSubmixPreset* SubmixPreset = Cast<USoundEffectSubmixPreset>(EffectObjPath))
                            {
                                NewPresetChain.Add(SubmixPreset);
                            }
                        }
                    }

                    NewEffectChain.SubmixEffectChain.Append(NewPresetChain);
                }
            }

            HDRSubmixEffectChain.Add(NewEffectChain);
        }

        // Load LDR Submix Effect Chain
        for (const FGameSubmixEffectChainMap& SoftSubmixEffectChain : GameAudioSettings->LDRAudioSubmixEffectChain)
        {
            FGameAudioSubmixEffectsChain NewEffectChain;

            if (UObject* SubmixObjPath = SoftSubmixEffectChain.Submix.LoadSynchronous())
            {
                if (USoundSubmix* Submix = Cast<USoundSubmix>(SubmixObjPath))
                {
                    NewEffectChain.Submix = Submix;
                    TArray<USoundEffectSubmixPreset*> NewPresetChain;

                    for (const TSoftObjectPtr<USoundEffectSubmixPreset>& SoftEffect : SoftSubmixEffectChain.SubmixEffectChain)
                    {
                        if (UObject* EffectObjPath = SoftEffect.LoadSynchronous())
                        {
                            if (USoundEffectSubmixPreset* SubmixPreset = Cast<USoundEffectSubmixPreset>(EffectObjPath))
                            {
                                NewPresetChain.Add(SubmixPreset);
                            }
                        }
                    }

                    NewEffectChain.SubmixEffectChain.Append(NewPresetChain);
                }
            }

            LDRSubmixEffectChain.Add(NewEffectChain);
        }
    }

    // Register with the loading screen manager
    if (ULoadingScreenManager* LoadingScreenManager = UGameInstance::GetSubsystem<ULoadingScreenManager>(GetWorld()->GetGameInstance()))
    {
        LoadingScreenManager->OnLoadingScreenVisibilityChangedDelegate().AddUObject(this, &ThisClass::OnLoadingScreenStatusChanged);
        ApplyOrRemoveLoadingScreenMix(LoadingScreenManager->GetLoadingScreenDisplayStatus());
    }
}

void UUR_AudioMixEffectsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    if (const UWorld* World = InWorld.GetWorld())
    {
        // Activate the default base mix
        if (DefaultBaseMix)
        {
            UAudioModulationStatics::ActivateBusMix(World, DefaultBaseMix);
        }

        // Retrieve the user settings
        if (const UUR_SettingsLocal* GameSettingsLocal = GetDefault<UUR_SettingsLocal>())
        {
            // Activate the User Mix
            if (UserMix)
            {
                UAudioModulationStatics::ActivateBusMix(World, UserMix);

                if (OverallControlBus && MusicControlBus && SoundFXControlBus && DialogueControlBus && VoiceChatControlBus)
                {
                    const FSoundControlBusMixStage OverallControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, OverallControlBus, GameSettingsLocal->GetOverallVolume());
                    const FSoundControlBusMixStage MusicControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, MusicControlBus, GameSettingsLocal->GetMusicVolume());
                    const FSoundControlBusMixStage SoundFXControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, SoundFXControlBus, GameSettingsLocal->GetSoundFXVolume());
                    const FSoundControlBusMixStage DialogueControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, DialogueControlBus, GameSettingsLocal->GetDialogueVolume());
                    const FSoundControlBusMixStage VoiceChatControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, VoiceChatControlBus, GameSettingsLocal->GetVoiceChatVolume());

                    TArray<FSoundControlBusMixStage> ControlBusMixStageArray;
                    ControlBusMixStageArray.Add(OverallControlBusMixStage);
                    ControlBusMixStageArray.Add(MusicControlBusMixStage);
                    ControlBusMixStageArray.Add(SoundFXControlBusMixStage);
                    ControlBusMixStageArray.Add(DialogueControlBusMixStage);
                    ControlBusMixStageArray.Add(VoiceChatControlBusMixStage);

                    UAudioModulationStatics::UpdateMix(World, UserMix, ControlBusMixStageArray);
                }
            }

            ApplyDynamicRangeEffectsChains(GameSettingsLocal->IsHDRAudioModeEnabled());
        }
    }
}

void UUR_AudioMixEffectsSubsystem::ApplyDynamicRangeEffectsChains(bool bHDRAudio)
{
    TArray<FGameAudioSubmixEffectsChain> AudioSubmixEffectsChainToApply;
    TArray<FGameAudioSubmixEffectsChain> AudioSubmixEffectsChainToClear;

    // If HDR Audio is selected, then we clear out any existing LDR Submix Effect Chain Overrides
    // otherwise the reverse is the case.
    if (bHDRAudio)
    {
        AudioSubmixEffectsChainToApply.Append(HDRSubmixEffectChain);
        AudioSubmixEffectsChainToClear.Append(LDRSubmixEffectChain);
    }
    else
    {
        AudioSubmixEffectsChainToApply.Append(LDRSubmixEffectChain);
        AudioSubmixEffectsChainToClear.Append(HDRSubmixEffectChain);
    }

    // We want to collect just the submixes we need to actually clear, otherwise they'll be overridden by the new settings
    TArray<USoundSubmix*> SubmixesLeftToClear;

    // We want to get the submixes that are not being overridden by the new effect chains, so we can clear those out separately
    for (const FGameAudioSubmixEffectsChain& EffectChainToClear : AudioSubmixEffectsChainToClear)
    {
        bool bAddToList = true;

        for (const FGameAudioSubmixEffectsChain& SubmixEffectChain : AudioSubmixEffectsChainToApply)
        {
            if (SubmixEffectChain.Submix == EffectChainToClear.Submix)
            {
                bAddToList = false;
                break;
            }
        }

        if (bAddToList)
        {
            SubmixesLeftToClear.Add(EffectChainToClear.Submix);
        }
    }

    // Override submixes
    for (const FGameAudioSubmixEffectsChain& SubmixEffectChain : AudioSubmixEffectsChainToApply)
    {
        if (SubmixEffectChain.Submix)
        {
            UAudioMixerBlueprintLibrary::SetSubmixEffectChainOverride(GetWorld(), SubmixEffectChain.Submix, SubmixEffectChain.SubmixEffectChain, 0.1f);
        }
    }

    // Clear remaining submixes
    for (USoundSubmix* Submix : SubmixesLeftToClear)
    {
        UAudioMixerBlueprintLibrary::ClearSubmixEffectChainOverride(GetWorld(), Submix, 0.1f);
    }
}

void UUR_AudioMixEffectsSubsystem::OnLoadingScreenStatusChanged(bool bShowingLoadingScreen)
{
    ApplyOrRemoveLoadingScreenMix(bShowingLoadingScreen);
}

void UUR_AudioMixEffectsSubsystem::ApplyOrRemoveLoadingScreenMix(bool bWantsLoadingScreenMix)
{
    const UWorld* World = GetWorld();

    if (bAppliedLoadingScreenMix != bWantsLoadingScreenMix && LoadingScreenMix && World)
    {
        if (bWantsLoadingScreenMix)
        {
            // Apply the mix
            UAudioModulationStatics::ActivateBusMix(World, LoadingScreenMix);
        }
        else
        {
            // Remove the mix
            UAudioModulationStatics::DeactivateBusMix(World, LoadingScreenMix);
        }
        bAppliedLoadingScreenMix = bWantsLoadingScreenMix;
    }
}

bool UUR_AudioMixEffectsSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
    // We only need this subsystem on Game worlds (PIE included)
    return (WorldType == EWorldType::Game || WorldType == EWorldType::PIE);
}
