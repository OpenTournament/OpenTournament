// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <LoadingProcessInterface.h>
#include <Components/GameStateComponent.h>

#include "UR_ExperienceManagerComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_ExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameExperienceLoaded, const UUR_ExperienceDefinition* /*Experience*/);

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace UE::GameFeatures
{
    struct FResult;
}

enum class EGameExperienceLoadState
{
    Unloaded,
    Loading,
    LoadingGameFeatures,
    LoadingChaosTestingDelay,
    ExecutingActions,
    Loaded,
    Deactivating
};

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_ExperienceManagerComponent final
    : public UGameStateComponent
    , public ILoadingProcessInterface
{
    GENERATED_BODY()

public:
    UUR_ExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~UActorComponent interface
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~End of UActorComponent interface

    //~ILoadingProcessInterface interface
    virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
    //~End of ILoadingProcessInterface

    // Tries to set the current experience, either a UI or gameplay one
    void SetCurrentExperience(FPrimaryAssetId ExperienceId);

    // Ensures the delegate is called once the experience has been loaded,
    // before others are called.
    // However, if the experience has already loaded, calls the delegate immediately.
    void CallOrRegister_OnExperienceLoaded_HighPriority(FOnGameExperienceLoaded::FDelegate&& Delegate);

    // Ensures the delegate is called once the experience has been loaded
    // If the experience has already loaded, calls the delegate immediately
    void CallOrRegister_OnExperienceLoaded(FOnGameExperienceLoaded::FDelegate&& Delegate);

    // Ensures the delegate is called once the experience has been loaded
    // If the experience has already loaded, calls the delegate immediately
    void CallOrRegister_OnExperienceLoaded_LowPriority(FOnGameExperienceLoaded::FDelegate&& Delegate);

    // This returns the current experience if it is fully loaded, asserting otherwise
    // (i.e., if you called it too soon)
    const UUR_ExperienceDefinition* GetCurrentExperienceChecked() const;

    // Returns true if the experience is fully loaded
    bool IsExperienceLoaded() const;

private:
    UFUNCTION()
    void OnRep_CurrentExperience();

    void StartExperienceLoad();
    void OnExperienceLoadComplete();
    void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
    void OnExperienceFullLoadCompleted();

    void OnActionDeactivationCompleted();
    void OnAllActionsDeactivated();

private:
    UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
    TObjectPtr<const UUR_ExperienceDefinition> CurrentExperience;

    EGameExperienceLoadState LoadState = EGameExperienceLoadState::Unloaded;

    int32 NumGameFeaturePluginsLoading = 0;
    TArray<FString> GameFeaturePluginURLs;

    int32 NumObservedPausers = 0;
    int32 NumExpectedPausers = 0;

    /**
     * Delegate called when the experience has finished loading just before others
     * (e.g., subsystems that set up for regular gameplay)
     */
    FOnGameExperienceLoaded OnExperienceLoaded_HighPriority;

    /** Delegate called when the experience has finished loading */
    FOnGameExperienceLoaded OnExperienceLoaded;

    /** Delegate called when the experience has finished loading */
    FOnGameExperienceLoaded OnExperienceLoaded_LowPriority;
};
