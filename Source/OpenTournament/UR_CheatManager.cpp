// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CheatManager.h"

#include "UR_Character.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"
#include "UR_InventoryComponent.h"
#include "UR_Weapon.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_CheatManager::God()
{
    if (AUR_Character* URCharacter = Cast<AUR_Character>(GetOuterAPlayerController()->GetPawn()))
    {
        if (URCharacter->CanBeDamaged())
        {
            GetOuterAPlayerController()->ClientMessage(TEXT("God mode on"));
        }
        else
        {
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

void UUR_CheatManager::Cheat_AddScore(int32 InValue)
{
    if (auto PC = Cast<AUR_PlayerController>(GetOuter()))
    {
        if (auto PS = Cast<AUR_PlayerState>(PC->PlayerState))
        {
            PS->SetScore(PS->GetScore() + InValue);
        }
    }
}
