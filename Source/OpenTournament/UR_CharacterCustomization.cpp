// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CharacterCustomization.h"
#include <Engine/SkeletalMesh.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

// Helper
template <typename T, typename StructType>
T* TryLoadUserAsset(const TSoftObjectPtr<T>& InUserPath, const TArray<StructType>& AssetMap)
{
    if (auto EntryPtr = AssetMap.FindByKey(InUserPath))
    {
        return EntryPtr->Ref.LoadSynchronous();
    }
    return nullptr;
}

void UUR_CharacterCustomizationBackend::LoadCharacterCustomizationAssets(FCharacterCustomization& InCustomization)
{
    auto Backend = GetDefault<UUR_CharacterCustomizationBackend>();

    InCustomization.LoadedCharacter = TryLoadUserAsset(InCustomization.Character, Backend->CharacterMeshes);
    InCustomization.LoadedHair = TryLoadUserAsset(InCustomization.Hair, Backend->HairMeshes);
}

FCharacterCustomization UUR_CharacterCustomizationBackend::MakeRandomCharacterCustomization()
{
    FCharacterCustomization Result;
    auto Backend = GetDefault<UUR_CharacterCustomizationBackend>();

    if (Backend->CharacterMeshes.Num() > 0)
    {
        const int32 Index = FMath::RandRange(0, Backend->CharacterMeshes.Num() - 1);
        Result.Character = Backend->CharacterMeshes[Index].Ref;
    }

    if (Backend->HairMeshes.Num() > 0)
    {
        const int32 Index = FMath::RandRange(0, Backend->HairMeshes.Num() - 1);
        Result.Hair = Backend->HairMeshes[Index].Ref;
    }

    {
        const uint8 Hue = FMath::RandRange(14, 38);
        const uint8 Brightness = FMath::RandRange(255 - (38 - Hue) * (255 - 10) / 14, 255);
        const uint8 Saturation = 128 + 127 * (255 - Brightness) / (255 - 10);
        Result.SkinTone = FLinearColor::MakeFromHSV8(Hue, Saturation, Brightness).ToFColor(true);
    }

    Result.HairColor = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255));

    return Result;
}
