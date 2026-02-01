// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ExperienceDefinition.h"

#include <GameFeatureAction.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ExperienceDefinition)

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTSystem"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_ExperienceDefinition::UUR_ExperienceDefinition()
{
}

#if WITH_EDITOR
EDataValidationResult UUR_ExperienceDefinition::IsDataValid(FDataValidationContext& Context) const
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

    // Make sure users didn't subclass from a BP of this (it's fine and expected to subclass once in BP, just not twice)
    if (!GetClass()->IsNative())
    {
        const UClass* ParentClass = GetClass()->GetSuperClass();

        // Find the native parent
        const UClass* FirstNativeParent = ParentClass;
        while ((FirstNativeParent != nullptr) && !FirstNativeParent->IsNative())
        {
            FirstNativeParent = FirstNativeParent->GetSuperClass();
        }

        if (FirstNativeParent != ParentClass)
        {
            Context.AddError(FText::Format(LOCTEXT("ExperienceInheritenceIsUnsupported",
                    "Blueprint subclasses of Blueprint experiences is not currently supported (use composition via ActionSets instead). Parent class was {0} but should be {1}."),
                FText::AsCultureInvariant(GetPathNameSafe(ParentClass)),
                FText::AsCultureInvariant(GetPathNameSafe(FirstNativeParent))
            ));
            Result = EDataValidationResult::Invalid;
        }
    }

    return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void UUR_ExperienceDefinition::UpdateAssetBundleData()
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
