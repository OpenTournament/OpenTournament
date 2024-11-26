// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CrosshairData.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_CrosshairData)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OT_CrosshairData"

/////////////////////////////////////////////////////////////////////////////////////////////////


UUR_CrosshairData::UUR_CrosshairData()
    : Tag()
    , Texture(nullptr)
    , Color(FColor::White)
    , Scale(FVector2D(1.f, 1.f))
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
EDataValidationResult UUR_CrosshairData::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

    if (!Texture)
    {
        Result = EDataValidationResult::Invalid;
        Context.AddError(FText::Format(LOCTEXT("CrosshairDataTextureIsNull", "Null entry for Texture in CrosshairData {Name}"), FText::FromString(GetName())));
    }

    // TODO : Error for Color & Scale values?

    return Super::IsDataValid(Context);
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
