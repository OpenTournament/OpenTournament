// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <GameFramework/WorldSettings.h>

#include <AssetRegistry/AssetData.h>

#include "UR_WorldSettings.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UTexture2D;

class UUR_ExperienceDefinition;

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

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The default world settings object, used primarily to set the default gameplay experience to use when playing on this map
 * Custom WorldSettings class - this is a good place to put per-map options that the C++ gameplay code can easily access.
 * Typically, this would include things like map description, thumbnail, music, initial camera location...
 *
 */
UCLASS()
class OPENTOURNAMENT_API AUR_WorldSettings : public AWorldSettings
{
    GENERATED_BODY()

public:
    AUR_WorldSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
    virtual void CheckForErrors() override;
#endif

public:
    virtual void BeginPlay() override;

    // Returns the default experience to use when a server opens this map if it is not overridden by the user-facing experience
    FPrimaryAssetId GetDefaultGameplayExperience() const;

    /**
    * UI properties stored in map header so they can be easily extracted
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable)
    FMapInfo MapInfo;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<USoundBase> Music;

    UFUNCTION()
    void OnMusicLoaded();

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
    // The default experience to use when a server opens this map if it is not overridden by the user-facing experience
    UPROPERTY(EditDefaultsOnly, Category=GameMode)
    TSoftClassPtr<UUR_ExperienceDefinition> DefaultGameplayExperience;

public:
#if WITH_EDITORONLY_DATA

    static void AddMapInfoTags(const UWorld* World, FAssetRegistryTagsContext Context);

    virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;

    // Is this level part of a front-end or other standalone experience?
    // When set, the net mode will be forced to Standalone when you hit Play in the editor
    UPROPERTY(EditDefaultsOnly, Category=PIE)
    bool ForceStandaloneNetMode = false;

#endif
};
