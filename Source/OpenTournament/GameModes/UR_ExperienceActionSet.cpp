// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ExperienceActionSet.h"

#include <GameFeatureAction.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ExperienceActionSet)

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTSystem"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_ExperienceActionSet::UUR_ExperienceActionSet()
{}

#if WITH_EDITOR
EDataValidationResult UUR_ExperienceActionSet::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

    int32 EntryIndex = 0;
    for (const UGameFeatureAction* Action : Actions)
    {
        if (Action)
        {
            EDataValidationResult ChildResult = Action->IsDataValid(Context);
            Result = CombineDataValidationResults(Result, ChildResult);
        }
        else
        {
            Result = EDataValidationResult::Invalid;
            Context.AddError(FText::Format(LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in Actions"), FText::AsNumber(EntryIndex)));
        }

        ++EntryIndex;
    }

    return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void UUR_ExperienceActionSet::UpdateAssetBundleData()
{
    Super::UpdateAssetBundleData();

    for (UGameFeatureAction* Action : Actions)
    {
        if (Action)
        {
            Action->AddAdditionalAssetBundleData(AssetBundleData);
        }
    }
}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE
