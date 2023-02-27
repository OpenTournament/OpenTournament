// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_WorldSettings.h"

#include "Components/AudioComponent.h"
#include "AssetRegistry/IAssetRegistry.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_WorldSettings::AUR_WorldSettings()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->SetupAttachment(RootComponent); //is attachment mandatory?
    //MusicComponent->SoundClassOverride = MusicSoundClass; //TODO
    MusicComponent->bAllowSpatialization = false;
    MusicComponent->bShouldRemainActiveIfDropped = true;    //never reset if cut out by priority
    MusicComponent->bIsUISound = true;  //continue when game paused
    MusicComponent->bSuppressSubtitles = true;  //???
    //MusicComponent->bIgnoreForFlushing = true;    //not sure about this
    MusicComponent->bIsMusic = true;    //what does this do???

    MapInfo.DisplayName = "MyLevel";

#if WITH_EDITORONLY_DATA
    FWorldDelegates::GetAssetTags.AddStatic(&AUR_WorldSettings::AddMapInfoTags);
#endif
}

void AUR_WorldSettings::BeginPlay()
{
    Super::BeginPlay();

    if (MusicComponent && !IsNetMode(NM_DedicatedServer))
    {
        //TODO: this is definitely an appropriate case to ASYNC load
        if (auto LoadedMusic = Music.LoadSynchronous())
        {
            MusicComponent->SetSound(LoadedMusic);
            MusicComponent->Play();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_WorldSettings* AUR_WorldSettings::GetWorldSettings(UObject* WorldContext)
{
    if (WorldContext && WorldContext->GetWorld())
        return Cast<AUR_WorldSettings>(WorldContext->GetWorld()->K2_GetWorldSettings());

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// Called under the hood by GetTagValue<FMapInfo>
static void LexFromString(FMapInfo& MapInfo, const TCHAR* String)
{
    UScriptStruct* ScriptStruct = FMapInfo::StaticStruct();
    ScriptStruct->ImportText(String, &MapInfo, nullptr, PPF_UseDeprecatedProperties, (FOutputDevice*)GWarn, []() { return FString("FMapInfo"); });
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
        AssetData.EnumerateTags([](const TPair<FName, FAssetTagValueRef>& Pair) {
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

void AUR_WorldSettings::AddMapInfoTags(const UWorld* World, TArray<FAssetRegistryTag>& OutTags)
{
    if (World && World->GetWorldSettings())
    {
        World->GetWorldSettings()->GetAssetRegistryTags(OutTags);
    }
}

void AUR_WorldSettings::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
    // skip AActor
    UObject::GetAssetRegistryTags(OutTags);
}

/*
 * Previous approach = automatically create and maintain reference to a MapInfo object in an external package with _MapInfo suffix
 * It works, but the AssetRegistry approach is much simpler
 * 

#include "UObject/ObjectSaveContext.h"
#include "UnrealEd/Public/FileHelpers.h"

void AUR_WorldSettings::PreSave(FObjectPreSaveContext SaveContext)
{
    Super::PreSave(SaveContext);

    if (GetPackage()->HasAnyPackageFlags(PKG_PlayInEditor))
        return;

    if (SaveContext.IsCooking())
        return;

    const FString MapInfoPackagePath = GetPackage()->GetPathName() + "_MapInfo";
    const FString MapInfoName = FPaths::GetCleanFilename(MapInfoPackagePath);

    // Find or create external package for MapInfo
    UPackage* MapInfoPackage = CreatePackage(*MapInfoPackagePath);
    if (!MapInfoPackage)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load/create package: %s"), *MapInfoPackagePath);
        return;
    }

    // Find possible existing object
    UUR_MapInfo* ExternalMapInfo = FindObject<UUR_MapInfo>(MapInfoPackage, *MapInfoName);

    // Create if not exists
    if (!ExternalMapInfo)
    {
        ExternalMapInfo = NewObject<UUR_MapInfo>(MapInfoPackage, *MapInfoName, RF_Public | RF_Standalone | RF_Transactional, MapInfo);
        if (!ExternalMapInfo)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create new MapInfo object: %s.%s"), *MapInfoPackagePath, *MapInfoName);
            return;
        }
        MapInfoPackage->SetIsExternallyReferenceable(true);
        MapInfoPackage->MarkPackageDirty();
    }

    if (ExternalMapInfo && MapInfo != ExternalMapInfo)
    {
        // If old mapinfo was inside map package, garbage it
        if (MapInfo && MapInfo->GetPackage() == GetPackage())
            MapInfo->MarkAsGarbage();

        // Point to external asset
        MapInfo = ExternalMapInfo;
    }

    // Save MapInfo package along with the map (kinda like _BuiltData)
    UEditorLoadingAndSavingUtils::SavePackages({ MapInfoPackage }, true);
}
*/

#endif
