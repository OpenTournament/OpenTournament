// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "UR_LocalPlayer.h"
//#include "Settings/UR_SettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_InputComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

//class UUR_InputConfig;

UUR_InputComponent::UUR_InputComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UUR_InputComponent::AddInputMappings(const UUR_InputConfig* InputConfig, const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
    check(InputConfig);
    check(InputSubsystem);

    // Here you can handle any custom logic to add something from your input config if required
}

void UUR_InputComponent::RemoveInputMappings(const UUR_InputConfig* InputConfig, const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
    check(InputConfig);
    check(InputSubsystem);

    // Here you can handle any custom logic to remove input mappings that you may have added above
}

void UUR_InputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
    for (const auto& Handle : BindHandles)
    {
        RemoveBindingByHandle(Handle);
    }
    BindHandles.Reset();
}
