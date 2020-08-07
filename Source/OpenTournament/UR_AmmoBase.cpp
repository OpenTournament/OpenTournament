// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AmmoBase.h"

#include "UR_Character.h"
#include "UR_InventoryComponent.h"
#include "UR_Ammo.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_AmmoBase::AUR_AmmoBase()
{
}

#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
void AUR_AmmoBase::CheckForErrors()
{
    Super::CheckForErrors();

    if (!HasAnyFlags(RF_ClassDefaultObject) && !IsValid(AmmoClass))
    {
        FMessageLog("MapCheck").Warning()
            ->AddToken(FUObjectToken::Create(this))
            ->AddToken(FTextToken::Create(FText::FromString(TEXT("has no valid ammo class"))));
    }
}
#endif

void AUR_AmmoBase::BeginPlay()
{
    // Avoid doing bullshit if we don't even have a valid weapon class
    if (!IsValid(AmmoClass))
    {
        bPickupAvailable = false;
        bRepInitialPickupAvailable = false;
        ShowPickupAvailable(false);
    }
    else
        Super::BeginPlay();
}

void AUR_AmmoBase::GiveTo_Implementation(class AActor* Other)
{
    if (AUR_Character* Char = Cast<AUR_Character>(Other))
    {
        if (Char->InventoryComponent)
        {
            Char->InventoryComponent->AddAmmo(AmmoClass);
        }
    }
    Super::GiveTo_Implementation(Other);
}

FText AUR_AmmoBase::GetItemName_Implementation()
{
    if (AmmoClass)
    {
        return FText::FromString(AmmoClass->GetDefaultObject<AUR_Ammo>()->AmmoName);
    }

    return FText::FromString(TEXT("void"));
}
