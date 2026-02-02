// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameFeatures/UR_GameFeaturePolicy.h"

#include "GameFeatureData.h"
#include "GameplayCueSet.h"
#include "GameFeatures/Public/GameFeaturesSubsystem.h"

#include "GAS/UR_GameplayCueManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameFeaturePolicy::UUR_GameFeaturePolicy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameFeaturePolicy& UUR_GameFeaturePolicy::Get()
{
    return UGameFeaturesSubsystem::Get().GetPolicy<UUR_GameFeaturePolicy>();
}

void UUR_GameFeaturePolicy::InitGameFeatureManager()
{
    Observers.Add(NewObject<UUR_GameFeature_HotfixManager>());
    Observers.Add(NewObject<UUR_GameFeature_AddGameplayCuePaths>());

    UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
    for (UObject* Observer : Observers)
    {
        Subsystem.AddObserver(Observer, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::CurrentAndFuture);
    }

    Super::InitGameFeatureManager();
}

void UUR_GameFeaturePolicy::ShutdownGameFeatureManager()
{
    Super::ShutdownGameFeatureManager();

    UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
    for (UObject* Observer : Observers)
    {
        Subsystem.RemoveObserver(Observer);
    }
    Observers.Empty();
}

TArray<FPrimaryAssetId> UUR_GameFeaturePolicy::GetPreloadAssetListForGameFeature(const UGameFeatureData* GameFeatureToLoad, bool bIncludeLoadedAssets) const
{
    return Super::GetPreloadAssetListForGameFeature(GameFeatureToLoad, bIncludeLoadedAssets);
}

const TArray<FName> UUR_GameFeaturePolicy::GetPreloadBundleStateForGameFeature() const
{
    return Super::GetPreloadBundleStateForGameFeature();
}

void UUR_GameFeaturePolicy::GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const
{
    // Editor will load both, this can cause hitching as the bundles are set to not preload in editor
    bLoadClientData = !IsRunningDedicatedServer();
    bLoadServerData = !IsRunningClientOnly();
}

bool UUR_GameFeaturePolicy::IsPluginAllowed(const FString& PluginURL, FString* OutReason) const
{
    return Super::IsPluginAllowed(PluginURL, OutReason);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Hotfix/UR_HotfixManager.h"

void UUR_GameFeature_HotfixManager::OnGameFeatureLoading(const UGameFeatureData* GameFeatureData, const FString& PluginURL)
{
    if (UUR_HotfixManager* HotfixManager = Cast<UUR_HotfixManager>(UOnlineHotfixManager::Get(nullptr)))
    {
        HotfixManager->RequestPatchAssetsFromIniFiles();
    }
}

//////////////////////////////////////////////////////////////////////
//

#include "AbilitySystemGlobals.h"
#include "GameFeatureAction_AddGameplayCuePath.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameFeaturePolicy)

class FName;
struct FPrimaryAssetId;

void UUR_GameFeature_AddGameplayCuePaths::OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UUR_GameFeature_AddGameplayCuePaths::OnGameFeatureRegistering);

    const FString PluginRootPath = TEXT("/") + PluginName;
    for (const UGameFeatureAction* Action : GameFeatureData->GetActions())
    {
        if (const UGameFeatureAction_AddGameplayCuePath* AddGameplayCueGFA = Cast<UGameFeatureAction_AddGameplayCuePath>(Action))
        {
            const TArray<FDirectoryPath>& DirsToAdd = AddGameplayCueGFA->GetDirectoryPathsToAdd();

            if (UUR_GameplayCueManager* GCM = UUR_GameplayCueManager::Get())
            {
                UGameplayCueSet* RuntimeGameplayCueSet = GCM->GetRuntimeCueSet();
                const int32 PreInitializeNumCues = RuntimeGameplayCueSet ? RuntimeGameplayCueSet->GameplayCueData.Num() : 0;

                for (const FDirectoryPath& Directory : DirsToAdd)
                {
                    FString MutablePath = Directory.Path;
                    UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
                    GCM->AddGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);
                }

                // Rebuild the runtime library with these new paths
                if (!DirsToAdd.IsEmpty())
                {
                    GCM->InitializeRuntimeObjectLibrary();
                }

                const int32 PostInitializeNumCues = RuntimeGameplayCueSet ? RuntimeGameplayCueSet->GameplayCueData.Num() : 0;
                if (PreInitializeNumCues != PostInitializeNumCues)
                {
                    GCM->RefreshGameplayCuePrimaryAsset();
                }
            }
        }
    }
}

void UUR_GameFeature_AddGameplayCuePaths::OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
    const FString PluginRootPath = TEXT("/") + PluginName;
    for (const UGameFeatureAction* Action : GameFeatureData->GetActions())
    {
        if (const UGameFeatureAction_AddGameplayCuePath* AddGameplayCueGFA = Cast<UGameFeatureAction_AddGameplayCuePath>(Action))
        {
            const TArray<FDirectoryPath>& DirsToAdd = AddGameplayCueGFA->GetDirectoryPathsToAdd();

            if (UGameplayCueManager* GCM = UAbilitySystemGlobals::Get().GetGameplayCueManager())
            {
                int32 NumRemoved = 0;
                for (const FDirectoryPath& Directory : DirsToAdd)
                {
                    FString MutablePath = Directory.Path;
                    UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
                    NumRemoved += GCM->RemoveGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);
                }

                ensure(NumRemoved == DirsToAdd.Num());

                // Rebuild the runtime library only if there is a need to
                if (NumRemoved > 0)
                {
                    GCM->InitializeRuntimeObjectLibrary();
                }
            }
        }
    }
}
