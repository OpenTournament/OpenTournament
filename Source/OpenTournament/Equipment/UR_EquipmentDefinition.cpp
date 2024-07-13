// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_EquipmentDefinition.h"

#include "UR_EquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_EquipmentDefinition)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_EquipmentDefinition::UUR_EquipmentDefinition(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InstanceType = UUR_EquipmentInstance::StaticClass();
}
