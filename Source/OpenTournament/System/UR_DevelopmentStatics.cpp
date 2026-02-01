// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_DevelopmentStatics.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"

#include "Development/UR_DeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_DevelopmentStatics)

/////////////////////////////////////////////////////////////////////////////////////////////////

bool UUR_DevelopmentStatics::ShouldSkipDirectlyToGameplay()
{
#if WITH_EDITOR
    if (GIsEditor)
    {
        return !GetDefault<UUR_DeveloperSettings>()->bTestFullGameFlowInPIE;
    }
#endif
    return false;
}

bool UUR_DevelopmentStatics::ShouldLoadCosmeticBackgrounds()
{
#if WITH_EDITOR
    if (GIsEditor)
    {
        return !GetDefault<UUR_DeveloperSettings>()->bSkipLoadingCosmeticBackgroundsInPIE;
    }
#endif
    return true;
}

bool UUR_DevelopmentStatics::CanPlayerBotsAttack()
{
#if WITH_EDITOR
    if (GIsEditor)
    {
        return GetDefault<UUR_DeveloperSettings>()->bAllowPlayerBotsToAttack;
    }
#endif
    return true;
}

//@TODO: Actually want to take a lambda and run on every authority world most of the time...
UWorld* UUR_DevelopmentStatics::FindPlayInEditorAuthorityWorld()
{
    check(GEngine);

    // Find the server world (any PIE world will do, in case they are running without a dedicated server, but the ded. server world is ideal)
    UWorld* ServerWorld = nullptr;
#if WITH_EDITOR
    for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE)
        {
            if (UWorld* TestWorld = WorldContext.World())
            {
                if (WorldContext.RunAsDedicated)
                {
                    // Ideal case
                    ServerWorld = TestWorld;
                    break;
                }
                else if (ServerWorld == nullptr)
                {
                    ServerWorld = TestWorld;
                }
                else
                {
                    // We already have a candidate, see if this one is 'better'
                    if (TestWorld->GetNetMode() < ServerWorld->GetNetMode())
                    {
                        return ServerWorld;
                    }
                }
            }
        }
    }
#endif

    return ServerWorld;
}

TArray<FAssetData> UUR_DevelopmentStatics::GetAllBlueprints()
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

    FName PluginAssetPath;

    TArray<FAssetData> BlueprintList;
    FARFilter Filter;
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = true;
    AssetRegistryModule.Get().GetAssets(Filter, BlueprintList);

    return BlueprintList;
}

UClass* UUR_DevelopmentStatics::FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass)
{
    FString TargetName = TargetNameRaw;
    TargetName.RemoveFromEnd(TEXT("_C"), ESearchCase::CaseSensitive);

    TArray<FAssetData> BlueprintList = UUR_DevelopmentStatics::GetAllBlueprints();
    for (const FAssetData& AssetData : BlueprintList)
    {
        if ((AssetData.AssetName.ToString() == TargetName) || (AssetData.GetObjectPathString() == TargetName))
        {
            if (UBlueprint* BP = Cast<UBlueprint>(AssetData.GetAsset()))
            {
                if (UClass* GeneratedClass = BP->GeneratedClass)
                {
                    if (GeneratedClass->IsChildOf(DesiredBaseClass))
                    {
                        return GeneratedClass;
                    }
                }
            }
        }
    }

    return nullptr;
}

UClass* UUR_DevelopmentStatics::FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures)
{
    check(DesiredBaseClass);

    FString TargetName = SearchToken;

    // Check native classes and loaded assets first before resorting to the asset registry
    bool bIsValidClassName = true;
    if (TargetName.IsEmpty() || TargetName.Contains(TEXT(" ")))
    {
        bIsValidClassName = false;
    }
    else if (!FPackageName::IsShortPackageName(TargetName))
    {
        if (TargetName.Contains(TEXT(".")))
        {
            // Convert type'path' to just path (will return the full string if it doesn't have ' in it)
            TargetName = FPackageName::ExportTextPathToObjectPath(TargetName);

            FString PackageName;
            FString ObjectName;
            TargetName.Split(TEXT("."), &PackageName, &ObjectName);

            const bool bIncludeReadOnlyRoots = true;
            FText Reason;
            if (!FPackageName::IsValidLongPackageName(PackageName, bIncludeReadOnlyRoots, &Reason))
            {
                bIsValidClassName = false;
            }
        }
        else
        {
            bIsValidClassName = false;
        }
    }

    UClass* ResultClass = nullptr;
    if (bIsValidClassName)
    {
        ResultClass = UClass::TryFindTypeSlow<UClass>(TargetName);
    }

    // If we still haven't found anything yet, try the asset registry for blueprints that match the requirements
    if (ResultClass == nullptr)
    {
        ResultClass = FindBlueprintClass(TargetName, DesiredBaseClass);
    }

    // Now validate the class (if we have one)
    if (ResultClass != nullptr)
    {
        if (!ResultClass->IsChildOf(DesiredBaseClass))
        {
            if (bLogFailures)
            {
                UE_LOG(LogConsoleResponse, Warning, TEXT("Found an asset %s but it wasn't of type %s"), *ResultClass->GetPathName(), *DesiredBaseClass->GetName());
            }
            ResultClass = nullptr;
        }
    }
    else
    {
        if (bLogFailures)
        {
            UE_LOG(LogConsoleResponse, Warning, TEXT("Failed to find class of type %s named %s"), *DesiredBaseClass->GetName(), *SearchToken);
        }
    }

    return ResultClass;
}
