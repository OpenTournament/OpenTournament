// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TaggedActor.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TaggedActor)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_TaggedActor::AUR_TaggedActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

void AUR_TaggedActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer.AppendTags(StaticGameplayTags);
}

#if WITH_EDITOR
bool AUR_TaggedActor::CanEditChange(const FProperty* InProperty) const
{
    // Prevent editing of the other tags property
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AActor, Tags))
    {
        return false;
    }

    return Super::CanEditChange(InProperty);
}
#endif
