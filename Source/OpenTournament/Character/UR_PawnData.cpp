// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PawnData)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_PawnData::UUR_PawnData(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , PawnClass(nullptr)
{
    InputConfig = nullptr;
    DefaultCameraMode = nullptr;
}
