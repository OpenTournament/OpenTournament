// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_CharacterPartTypes.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UChildActorComponent;
class UUR_PawnComponent_CharacterParts;
struct FUR_CharacterPartList;

//////////////////////////////////////////////////////////////////////

// How should collision be configured on the spawned part actor
UENUM()
enum class ECharacterCustomizationCollisionMode : uint8
{
    // Disable collision on spawned character parts
    NoCollision,

    // Leave the collision settings on character parts alone
    UseCollisionFromCharacterPart
};

//////////////////////////////////////////////////////////////////////

// A handle created by adding a character part entry, can be used to remove it later
USTRUCT(BlueprintType)
struct FUR_CharacterPartHandle
{
    GENERATED_BODY()

    void Reset()
    {
        PartHandle = INDEX_NONE;
    }

    bool IsValid() const
    {
        return PartHandle != INDEX_NONE;
    }

private:
    UPROPERTY()
    int32 PartHandle = INDEX_NONE;

    friend FUR_CharacterPartList;
};

//////////////////////////////////////////////////////////////////////
// A character part request

USTRUCT(BlueprintType)
struct FUR_CharacterPart
{
    GENERATED_BODY()

    // The part to spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> PartClass;

    // The socket to attach the part to (if any)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SocketName;

    // How to handle collision for the primitive components in the part
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterCustomizationCollisionMode CollisionMode = ECharacterCustomizationCollisionMode::NoCollision;

    // Compares against another part, ignoring the collision mode
    static bool AreEquivalentParts(const FUR_CharacterPart& A, const FUR_CharacterPart& B)
    {
        return (A.PartClass == B.PartClass) && (A.SocketName == B.SocketName);
    }
};
