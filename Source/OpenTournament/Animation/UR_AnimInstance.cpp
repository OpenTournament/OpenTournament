// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AnimInstance.h"

#include "AbilitySystemGlobals.h"

#include "UR_Character.h"
#include "Character/UR_CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AnimInstance)

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AnimInstance::UUR_AnimInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

void UUR_AnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
    check(ASC);

    GameplayTagPropertyMap.Initialize(this, ASC);
}

#if WITH_EDITOR
EDataValidationResult UUR_AnimInstance::IsDataValid(FDataValidationContext& Context) const
{
    Super::IsDataValid(Context);

    GameplayTagPropertyMap.IsDataValid(this, Context);

    return ((Context.GetNumErrors() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}
#endif // WITH_EDITOR

void UUR_AnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    if (AActor* OwningActor = GetOwningActor())
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
        {
            InitializeWithAbilitySystem(ASC);
        }
    }
}

void UUR_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    const AUR_Character* Character = Cast<AUR_Character>(GetOwningActor());
    if (!Character)
    {
        return;
    }

    UUR_CharacterMovementComponent* CharMoveComp = CastChecked<UUR_CharacterMovementComponent>(Character->GetCharacterMovement());
    const FUR_CharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
    GroundDistance = GroundInfo.GroundDistance;
}
