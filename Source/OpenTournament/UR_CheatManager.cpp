// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CheatManager.h"

#include "UR_Character.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"
#include "UR_InventoryComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_CheatManager::God()
{
    if (AUR_Character* URCharacter = Cast<AUR_Character>(GetOuterAPlayerController()->GetPawn()))
    {
        if (URCharacter->CanBeDamaged())
        {
            //URCharacter->SetCanBeDamaged(false);
            GetOuterAPlayerController()->ClientMessage(TEXT("God mode on"));
        }
        else
        {
            //URCharacter->SetCanBeDamaged(true);
            GetOuterAPlayerController()->ClientMessage(TEXT("God Mode off"));
        }
    }

    // This handles the heavy lifting hooking into CanBeDamaged
    Super::God();
}

void UUR_CheatManager::Cheat_Loaded()
{
    if (AUR_Character* URCharacter = Cast<AUR_Character>(GetOuterAPlayerController()->GetPawn()))
    {
        if (UUR_InventoryComponent* Inventory = URCharacter->FindComponentByClass<UUR_InventoryComponent>())
        {
            for (const auto WeaponClass : LoadedWeaponClasses)
            {
                const auto World = URCharacter->GetWorld();
                auto SpawnedWeapon = World->SpawnActor<AUR_Weapon>(WeaponClass);
                Inventory->Add(SpawnedWeapon);
            }            
        }
    }
}

void UUR_CheatManager::Cheat_AddScore(int32 n)
{
    if (auto PC = Cast<AUR_PlayerController>(GetOuter()))
    {
        if (auto PS = Cast<AUR_PlayerState>(PC->PlayerState))
        {
            PS->Score += n;
        }
    }
}
