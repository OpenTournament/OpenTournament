// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HealthBase.h"
#include "UR_Character.h"
#include "UR_AttributeSet.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_HealthBase::AUR_HealthBase()
{
    HealAmount = 25;
    bSuperHeal = false;
}

bool AUR_HealthBase::AllowPickupBy_Implementation(class AActor* Other)
{
    if (Super::AllowPickupBy_Implementation(Other))
    {
        // Can always pick up super-heal items
        if (bSuperHeal)
        {
            return true;
        }

        // Can only pick ups health packs if we are below 100
        AUR_Character* Char = Cast<AUR_Character>(Other);
        return (Char && Char->AttributeSet && Char->AttributeSet->GetHealth() < Char->AttributeSet->GetHealthMax());
    }
    return false;
}

void AUR_HealthBase::GiveTo_Implementation(class AActor* Other)
{
    AUR_Character* Char = Cast<AUR_Character>(Other);
    if (Char && Char->AttributeSet)
    {
        const float CurrentHealth = Char->AttributeSet->GetHealth();
        GAME_LOG(Game, Log, "Health Pickup: Current Health (%f)", CurrentHealth);

        // @! TODO : This is Temporary. Healing should be done via GameplayEffect.
        if (!bSuperHeal)
        {
            float FinalHealth = FMath::Clamp<int32>(static_cast<int32>(CurrentHealth) + HealAmount, 0.f, static_cast<int32>(Char->AttributeSet->GetHealthMax()));
            GAME_LOG(Game, Log, "Health Pickup: Restoring Health (%f)", (FinalHealth - CurrentHealth));
            Char->AttributeSet->SetHealth(FinalHealth);
        }
        else
        {
            GAME_LOG(Game, Log, "Health Pickup: Restoring Health (%d)", HealAmount);
            Char->AttributeSet->SetHealth(CurrentHealth + HealAmount);
        }
    }

    Super::GiveTo_Implementation(Other);
}

FText AUR_HealthBase::GetItemName_Implementation()
{
    return FText::FromString(FString::Printf(TEXT("%i Health"), HealAmount));
}
