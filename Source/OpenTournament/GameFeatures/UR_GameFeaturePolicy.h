// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFeatureStateChangeObserver.h"
#include "GameFeaturesProjectPolicies.h"

#include "UR_GameFeaturePolicy.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class FName;
class UGameFeatureData;
struct FPrimaryAssetId;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Manager to keep track of the state machines that bring a game feature plugin into memory and active
 * This class discovers plugins either that are built-in and distributed with the game or are reported externally (i.e. by a web service or other endpoint)
 */
UCLASS(MinimalAPI, Config = Game)
class UUR_GameFeaturePolicy : public UDefaultGameFeaturesProjectPolicies
{
    GENERATED_BODY()

public:
    OPENTOURNAMENT_API static UUR_GameFeaturePolicy& Get();

    UUR_GameFeaturePolicy(const FObjectInitializer& ObjectInitializer);

    //~UGameFeaturesProjectPolicies interface
    virtual void InitGameFeatureManager() override;
    virtual void ShutdownGameFeatureManager() override;
    virtual TArray<FPrimaryAssetId> GetPreloadAssetListForGameFeature(const UGameFeatureData* GameFeatureToLoad, bool bIncludeLoadedAssets = false) const override;
    virtual bool IsPluginAllowed(const FString& PluginURL, FString* OutReason) const override;
    virtual const TArray<FName> GetPreloadBundleStateForGameFeature() const override;
    virtual void GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const override;
    //~End of UGameFeaturesProjectPolicies interface

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<UObject>> Observers;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

// checked
UCLASS()
class UUR_GameFeature_HotfixManager : public UObject, public IGameFeatureStateChangeObserver
{
    GENERATED_BODY()

public:
    virtual void OnGameFeatureLoading(const UGameFeatureData* GameFeatureData, const FString& PluginURL) override;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

// checked
UCLASS()
class UUR_GameFeature_AddGameplayCuePaths : public UObject, public IGameFeatureStateChangeObserver
{
    GENERATED_BODY()

public:
    virtual void OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;
    virtual void OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;
};
