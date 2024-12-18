// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_WorldSettings.h"

#include <EngineUtils.h>
#include <AssetRegistry/IAssetRegistry.h>
#include <Components/AudioComponent.h>
#include <Engine/AssetManager.h>
#include <Engine/StreamableManager.h>
#include <GameFramework/PlayerStart.h>
#include <Logging/MessageLog.h>
#include <Misc/Paths.h>
#include <Misc/UObjectToken.h>
#include <Sound/SoundBase.h>

#include "UR_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_WorldSettings)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_WorldSettings::AUR_WorldSettings(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    MapInfo.DisplayName = "MyLevel";

#if WITH_EDITORONLY_DATA
    FWorldDelegates::GetAssetTagsWithContext.AddStatic(&AUR_WorldSettings::AddMapInfoTags);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////

FPrimaryAssetId AUR_WorldSettings::GetDefaultGameplayExperience() const
{
    FPrimaryAssetId Result;
    if (!DefaultGameplayExperience.IsNull())
    {
        Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath());

        if (!Result.IsValid())
        {
            UE_LOG(LogGame, Error, TEXT("%s.DefaultGameplayExperience is %s but that failed to resolve into an asset ID (you might need to add a path to the Asset Rules in your game feature plugin or project settings"), *GetPathNameSafe(this), *DefaultGameplayExperience.ToString());
        }
    }
    return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void AUR_WorldSettings::CheckForErrors()
{
    Super::CheckForErrors();

    FMessageLog MapCheck("MapCheck");

    for (TActorIterator<APlayerStart> PlayerStartIt(GetWorld()); PlayerStartIt; ++PlayerStartIt)
    {
        APlayerStart* PlayerStart = *PlayerStartIt;
        if (IsValid(PlayerStart) && PlayerStart->GetClass() == APlayerStart::StaticClass())
        {
            MapCheck.Warning()
                    ->AddToken(FUObjectToken::Create(PlayerStart))
                    ->AddToken(FTextToken::Create(FText::FromString("is a normal APlayerStart, replace with AUR_PlayerStart.")));
        }
    }

    //@TODO: Make sure the soft object path is something that can actually be turned into a primary asset ID (e.g., is not pointing to an experience in an unscanned directory)
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_WorldSettings::BeginPlay()
{
    Super::BeginPlay();

    if (!IsNetMode(NM_DedicatedServer))
    {
        if (const UAssetManager* Manager = UAssetManager::GetIfInitialized())
        {
            auto& StreamableManager = Manager->GetStreamableManager();
            const FStreamableDelegate CallbackDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnMusicLoaded);

            StreamableManager.RequestAsyncLoad(Music.ToSoftObjectPath(), CallbackDelegate);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_WorldSettings::OnMusicLoaded()
{
    // @! TODO : Play music from some kind of Music system
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_WorldSettings* AUR_WorldSettings::GetWorldSettings(UObject* WorldContext)
{
    if (WorldContext && WorldContext->GetWorld())
    {
        return Cast<AUR_WorldSettings>(WorldContext->GetWorld()->K2_GetWorldSettings());
    }

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// Called under the hood by GetTagValue<FMapInfo>
static void LexFromString(FMapInfo& MapInfo, const TCHAR* String)
{
    UScriptStruct* ScriptStruct = FMapInfo::StaticStruct();
    ScriptStruct->ImportText(String,
        &MapInfo,
        nullptr,
        PPF_UseDeprecatedProperties,
        (FOutputDevice*)GWarn,
        []()
        {
            return FString("FMapInfo");
        });
}

void AUR_WorldSettings::GetAllMaps(TArray<FMapInfo>& OutMaps)
{
    if (auto AssetRegistry = IAssetRegistry::Get())
    {
        TArray<FAssetData> List;
        AssetRegistry->GetAssetsByClass(FTopLevelAssetPath(UWorld::StaticClass()->GetPathName()), List);
        for (auto& Data : List)
        {
            FMapInfo& MapInfo = OutMaps.Emplace_GetRef();
            if (!Data.GetTagValue<FMapInfo>("MapInfo", MapInfo))
            {
                // Fallback values
                MapInfo.DisplayName = Data.AssetName.ToString();
            }
            // Dynamic values
            MapInfo.MapPath = Data.GetSoftObjectPath();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_WorldSettings::DebugDumpAssetTags(const FAssetData& AssetData)
{
    if (AssetData.IsValid())
    {
        AssetData.EnumerateTags([](const TPair<FName, FAssetTagValueRef>& Pair)
        {
            if (Pair.Key != "FiBData")  //fuck this tag in particular
                UE_LOG(LogTemp, Log, TEXT("[DumpTags] %s = %s"), *Pair.Key.ToString(), *Pair.Value.AsString());
        });
    }
}

void AUR_WorldSettings::DebugDumpPackageTags(FString Path)
{
    if (auto AssetRegistry = IAssetRegistry::Get())
    {
        // GetAssetByObjectPath with FSoftObjectPath requires full object path not just package path
        if (!Path.Contains("."))
            Path = Path + "." + FPaths::GetBaseFilename(Path);

        FAssetData AssetData = AssetRegistry->GetAssetByObjectPath(FSoftObjectPath(Path), true);
        if (AssetData.IsValid())
            DebugDumpAssetTags(AssetData);
        else
            UE_LOG(LogTemp, Warning, TEXT("[DumpTags] Error: Asset file not found at: %s"), *Path);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITORONLY_DATA

#include "UObject/AssetRegistryTagsContext.h"

// We use this trick to inject WorldSettings' tags into the World (.umap) asset file
// UObject::GetAssetRegistryTags iterates all UPROPERTY marked with AssetRegistrySearchable
// As such, our MapInfo data will end up in the .umap asset and can be retrieved without loading the entire levels in memory.

//NOTE: investigate OnGetExtraObjectTagsWithContext

void AUR_WorldSettings::AddMapInfoTags(const UWorld* World, FAssetRegistryTagsContext Context)
{
    if (World && World->GetWorldSettings())
    {
        World->GetWorldSettings()->GetAssetRegistryTags(Context);
    }
}

void AUR_WorldSettings::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
    // skip AActor
    UObject::GetAssetRegistryTags(Context);
}

#endif
