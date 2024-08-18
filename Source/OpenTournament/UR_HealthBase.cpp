// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HealthBase.h"

#include "UR_AbilitySystemComponent.h"
#include "UR_AssetManager.h"
#include "UR_Character.h"
#include "UR_AttributeSet.h"
#include "UR_GameData.h"
#include "UR_GameplayTags.h"
#include "UR_LogChannels.h"
#include "Attributes/UR_HealthSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_HealthBase)

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
        return (Char && Char->HealthSet && Char->HealthSet->GetHealth() < Char->HealthSet->GetMaxHealth());
    }
    return false;
}

void AUR_HealthBase::GiveTo_Implementation(class AActor* Other)
{
    AUR_Character* Char = Cast<AUR_Character>(Other);
    if (Char)
    {
        if (auto ASC = Char->GetGameAbilitySystemComponent())
        {
            check(ASC);

            TSubclassOf<UGameplayEffect> HealGE = UUR_AssetManager::GetSubclass(UUR_GameData::Get().HealGameplayEffect_SetByCaller);
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealGE, 1.0f, ASC->MakeEffectContext());

            if (SpecHandle.IsValid())
            {
                SpecHandle.Data->SetSetByCallerMagnitude(URGameplayTags::SetByCaller_Heal, HealAmount);
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    }

    Super::GiveTo_Implementation(Other);
}

FText AUR_HealthBase::GetItemName_Implementation()
{
    return FText::FromString(FString::Printf(TEXT("%i Health"), HealAmount));
}
