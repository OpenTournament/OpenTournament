// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "GAS/UR_AbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AbilityTagRelationshipMapping)

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const
{
    // Simple iteration for now
    for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
    {
        const FUR_AbilityTagRelationship& Tags = AbilityTagRelationships[i];
        if (AbilityTags.HasTag(Tags.AbilityTag))
        {
            if (OutTagsToBlock)
            {
                OutTagsToBlock->AppendTags(Tags.AbilityTagsToBlock);
            }
            if (OutTagsToCancel)
            {
                OutTagsToCancel->AppendTags(Tags.AbilityTagsToCancel);
            }
        }
    }
}

void UUR_AbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const
{
    // Simple iteration for now
    for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
    {
        const FUR_AbilityTagRelationship& Tags = AbilityTagRelationships[i];
        if (AbilityTags.HasTag(Tags.AbilityTag))
        {
            if (OutActivationRequired)
            {
                OutActivationRequired->AppendTags(Tags.ActivationRequiredTags);
            }
            if (OutActivationBlocked)
            {
                OutActivationBlocked->AppendTags(Tags.ActivationBlockedTags);
            }
        }
    }
}

bool UUR_AbilityTagRelationshipMapping::IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const
{
    // Simple iteration for now
    for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
    {
        const FUR_AbilityTagRelationship& Tags = AbilityTagRelationships[i];

        if (Tags.AbilityTag == ActionTag && Tags.AbilityTagsToCancel.HasAny(AbilityTags))
        {
            return true;
        }
    }

    return false;
}
