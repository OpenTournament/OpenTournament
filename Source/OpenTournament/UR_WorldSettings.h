// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/WorldSettings.h"
#include "AssetRegistry/AssetData.h"

#include "UR_WorldSettings.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UTexture2D;
class USoundBase;
class UAudioComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Level info to describe a map for UI purposes (name, description, thumbnail...)
 * Stored in the header of map packages as Asset Registry Tags so we can extract them without loading whole packages.
 */
USTRUCT(BlueprintType)
struct FMapInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> Thumbnail;

    // Path to the actual map, for travelling
    UPROPERTY(BlueprintReadOnly, Transient)
    FSoftObjectPath MapPath;
};

/**
 * Custom WorldSettings class - this is a good place to put per-map options that the C++ gameplay code can easily access.
 * Typically this would include things like map description, thumbnail, music, initial camera location...
 */
UCLASS(BlueprintType)
class OPENTOURNAMENT_API AUR_WorldSettings : public AWorldSettings
{
    GENERATED_BODY()

public:

    AUR_WorldSettings();

    /**
    * UI properties stored in map header so they can be easily extracted
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable)
    FMapInfo MapInfo;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<USoundBase> Music;

    UPROPERTY()
    UAudioComponent* MusicComponent;

    UFUNCTION(BlueprintPure, Meta = (WorldContext = "WorldContext"))
    static AUR_WorldSettings* GetWorldSettings(UObject* WorldContext);

    //TODO: Filtering options, Sorting options
    UFUNCTION(BlueprintCallable)
    static void GetAllMaps(TArray<FMapInfo>& OutMaps);

    // Debug Utility
    UFUNCTION(BlueprintCallable)
    static void DebugDumpAssetTags(const FAssetData& AssetData);

    // Debug Utility
    UFUNCTION(BlueprintCallable)
    static void DebugDumpPackageTags(FString Path);

protected:

    virtual void BeginPlay() override;

#if WITH_EDITORONLY_DATA
    static void AddMapInfoTags(const UWorld* World, TArray<FAssetRegistryTag>& OutTags);
    virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
#endif

};

/////////////////////////////////////////////////////////////////////////////////////////////////
