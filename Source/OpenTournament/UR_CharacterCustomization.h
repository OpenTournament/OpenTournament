// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <CoreMinimal.h>
#include <UObject/ObjectMacros.h>

#include "UR_CharacterCustomization.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class USkeletalMesh;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * User-configurable character customization.
 * Configured from game menu, sent to server, validated, and broadcasted to other clients.
 * Saved in UR_UserSettings.
 */
USTRUCT(BlueprintType)
struct FCharacterCustomization
{
    GENERATED_BODY()

    /**
    * Chosen character mesh (asset path).
    * In multiplayer, each client validates this against his own Backend list before being applied.
    * If a client does not allow this mesh to be used, or it is a mod mesh he doesn't have, then he will see default mesh instead.
    * A validation on server could be added as well but it is not necessary.
    */
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> Character;

    UPROPERTY(BlueprintReadOnly, Transient, NotReplicated)
    TObjectPtr<USkeletalMesh> LoadedCharacter;

    /**
    * Chosen hair mesh (asset path).
    */
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> Hair;

    UPROPERTY(BlueprintReadOnly, Transient, NotReplicated)
    TObjectPtr<USkeletalMesh> LoadedHair;

    //NOTE: Using FColor as it is much more efficient than FLinearColor (32 bits vs 128 bits)
    // Precision color probably doesn't matter, and negative/overglow colors are probably not desired.

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    FColor SkinTone;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    FColor HairColor;

    FCharacterCustomization()
    {
        SkinTone = FColor::White;
        HairColor = FColor::Black;
    }
};

//NOTE: We use this instead of storing a TMap, because ini TMap don't support line-by-line entries like TArray so fuck them.
USTRUCT(BlueprintType)
struct FNamedSkeletalMeshReference
{
    GENERATED_BODY()

    UPROPERTY(Config, BlueprintReadOnly)
    FName Name;

    UPROPERTY(Config, BlueprintReadOnly)
    TSoftObjectPtr<USkeletalMesh> Ref;

    bool operator==(const FName& OtherName) const
    {
        return OtherName == Name;
    }

    bool operator==(const TSoftObjectPtr<USkeletalMesh>& OtherRef) const
    {
        return OtherRef == Ref;
    }
};

/**
* Backend config and library CDO.
*/
UCLASS(Config = Game, Abstract)
class OPENTOURNAMENT_API UUR_CharacterCustomizationBackend : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, BlueprintReadOnly)
    TArray<FNamedSkeletalMeshReference> CharacterMeshes;

    UPROPERTY(Config, BlueprintReadOnly)
    TArray<FNamedSkeletalMeshReference> HairMeshes;

    UFUNCTION(BlueprintCallable)
    static void LoadCharacterCustomizationAssets(UPARAM(Ref) FCharacterCustomization& InCustomization);

    UFUNCTION(BlueprintCallable)
    static FCharacterCustomization MakeRandomCharacterCustomization();
};
