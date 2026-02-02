// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "UR_AudioMixEffectsSubsystem.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class FSubsystemCollectionBase;
class UObject;
class USoundControlBus;
class USoundControlBusMix;
class USoundEffectSubmixPreset;
class USoundSubmix;
class UWorld;

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FGameAudioSubmixEffectsChain
{
    GENERATED_BODY()

    // Submix on which to apply the Submix Effect Chain Override
    UPROPERTY(Transient)
    TObjectPtr<USoundSubmix> Submix = nullptr;

    // Submix Effect Chain Override (Effects processed in Array index order)
    UPROPERTY(Transient)
    TArray<TObjectPtr<USoundEffectSubmixPreset>> SubmixEffectChain;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This subsystem is meant to automatically engage default and user control bus mixes
 * to retrieve previously saved user settings and apply them to the activated user mix.
 * Additionally, this subsystem will automatically apply HDR/LDR Audio Submix Effect Chain Overrides
 * based on the user's preference for HDR Audio. Submix Effect Chain Overrides are defined in the
 * Game Audio Settings.
 */
UCLASS(MinimalAPI)
class UUR_AudioMixEffectsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem implementation Begin
    UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    UE_API virtual void Deinitialize() override;
    // USubsystem implementation End

    UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /** Called once all UWorldSubsystems have been initialized */
    UE_API virtual void PostInitialize() override;

    /** Called when world is ready to start gameplay before the game mode transitions to the correct state and call BeginPlay on all actors */
    UE_API virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Set whether the HDR Audio Submix Effect Chain Override settings are applied */
    UE_API void ApplyDynamicRangeEffectsChains(bool bHDRAudio);

protected:
    UE_API void OnLoadingScreenStatusChanged(bool bShowingLoadingScreen);

    UE_API void ApplyOrRemoveLoadingScreenMix(bool bWantsLoadingScreenMix);

    // Called when determining whether to create this Subsystem
    UE_API virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

    // Default Sound Control Bus Mix retrieved from the Game Audio Settings
    UPROPERTY(Transient)
    TObjectPtr<USoundControlBusMix> DefaultBaseMix = nullptr;

    // Loading Screen Sound Control Bus Mix retrieved from the Game Audio Settings
    UPROPERTY(Transient)
    TObjectPtr<USoundControlBusMix> LoadingScreenMix = nullptr;

    // User Sound Control Bus Mix retrieved from the Game Audio Settings
    UPROPERTY(Transient)
    TObjectPtr<USoundControlBusMix> UserMix = nullptr;

    // Overall Sound Control Bus retrieved from the Game Audio Settings and linked to the UI and game settings in GameSettingsLocal
    UPROPERTY(Transient)
    TObjectPtr<USoundControlBus> OverallControlBus = nullptr;

    // Music Sound Control Bus retrieved from the Game Audio Settings and linked to the UI and game settings in GameSettingsLocal
    UPROPERTY(Transient)
    TObjectPtr<USoundControlBus> MusicControlBus = nullptr;

    // SoundFX Sound Control Bus retrieved from the Game Audio Settings and linked to the UI and game settings in GameSettingsLocal
    UPROPERTY(Transient)
    TObjectPtr<USoundControlBus> SoundFXControlBus = nullptr;

    // Dialogue Sound Control Bus retrieved from the Game Audio Settings and linked to the UI and game settings in GameSettingsLocal
    UPROPERTY(Transient)
    TObjectPtr<USoundControlBus> DialogueControlBus = nullptr;

    // VoiceChat Sound Control Bus retrieved from the Game Audio Settings and linked to the UI and game settings in GameSettingsLocal
    UPROPERTY(Transient)
    TObjectPtr<USoundControlBus> VoiceChatControlBus = nullptr;

    // Submix Effect Chain Overrides to apply when HDR Audio is turned on
    UPROPERTY(Transient)
    TArray<FGameAudioSubmixEffectsChain> HDRSubmixEffectChain;

    // Submix Effect hain Overrides to apply when HDR Audio is turned off
    UPROPERTY(Transient)
    TArray<FGameAudioSubmixEffectsChain> LDRSubmixEffectChain;

    bool bAppliedLoadingScreenMix = false;
};

#undef UE_API
