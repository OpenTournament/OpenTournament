// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CosmeticCheats.h"

#include "GameFramework/CheatManagerDefines.h"

#include "UR_ControllerComponent_CharacterParts.h"
#include "Cosmetics/UR_CharacterPartTypes.h"
#include "System/UR_DevelopmentStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_CosmeticCheats)

/////////////////////////////////////////////////////////////////////////////////////////////////
// UUR_CosmeticCheats

UUR_CosmeticCheats::UUR_CosmeticCheats()
{
#if UE_WITH_CHEAT_MANAGER
    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        UCheatManager::RegisterForOnCheatManagerCreated
        (FOnCheatManagerCreated::FDelegate::CreateLambda
            (
                [](UCheatManager* CheatManager)
                {
                    CheatManager->AddCheatManagerExtension(NewObject<ThisClass>(CheatManager));
                }));
    }
#endif
}

void UUR_CosmeticCheats::AddCharacterPart(const FString& AssetName, bool bSuppressNaturalParts)
{
#if UE_WITH_CHEAT_MANAGER
    if (UUR_ControllerComponent_CharacterParts* CosmeticComponent = GetCosmeticComponent())
    {
        TSubclassOf<AActor> PartClass = UUR_DevelopmentStatics::FindClassByShortName<AActor>(AssetName);
        if (PartClass != nullptr)
        {
            FUR_CharacterPart Part;
            Part.PartClass = PartClass;

            CosmeticComponent->AddCheatPart(Part, bSuppressNaturalParts);
        }
    }
#endif
}

void UUR_CosmeticCheats::ReplaceCharacterPart(const FString& AssetName, bool bSuppressNaturalParts)
{
    ClearCharacterPartOverrides();
    AddCharacterPart(AssetName, bSuppressNaturalParts);
}

void UUR_CosmeticCheats::ClearCharacterPartOverrides()
{
#if UE_WITH_CHEAT_MANAGER
    if (UUR_ControllerComponent_CharacterParts* CosmeticComponent = GetCosmeticComponent())
    {
        CosmeticComponent->ClearCheatParts();
    }
#endif
}

UUR_ControllerComponent_CharacterParts* UUR_CosmeticCheats::GetCosmeticComponent() const
{
    if (APlayerController* PC = GetPlayerController())
    {
        return PC->FindComponentByClass<UUR_ControllerComponent_CharacterParts>();
    }

    return nullptr;
}
