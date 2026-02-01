// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/CheatManager.h"

#include "UR_CosmeticCheats.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_ControllerComponent_CharacterParts;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

/** Cheats related to bots */
UCLASS(NotBlueprintable)
class UUR_CosmeticCheats final : public UCheatManagerExtension
{
    GENERATED_BODY()

public:
    UUR_CosmeticCheats();

    // Adds a character part
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    void AddCharacterPart(const FString& AssetName, bool bSuppressNaturalParts = true);

    // Replaces previous cheat parts with a new one
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    void ReplaceCharacterPart(const FString& AssetName, bool bSuppressNaturalParts = true);

    // Clears any existing cheats
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    void ClearCharacterPartOverrides();

private:
    UUR_ControllerComponent_CharacterParts* GetCosmeticComponent() const;
};
