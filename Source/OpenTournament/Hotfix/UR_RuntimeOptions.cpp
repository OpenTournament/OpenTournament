// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_RuntimeOptions.h"

#include "UObject/Class.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_RuntimeOptions)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_RuntimeOptions::UUR_RuntimeOptions()
{
    static constexpr const TCHAR* DefaultOptionCommandPrefix = TEXT("ro");

    OptionCommandPrefix = DefaultOptionCommandPrefix;
}

UUR_RuntimeOptions* UUR_RuntimeOptions::GetRuntimeOptions()
{
    return GetMutableDefault<UUR_RuntimeOptions>();
}

const UUR_RuntimeOptions& UUR_RuntimeOptions::Get()
{
    const UUR_RuntimeOptions& RuntimeOptions = *GetDefault<UUR_RuntimeOptions>();
    return RuntimeOptions;
}
