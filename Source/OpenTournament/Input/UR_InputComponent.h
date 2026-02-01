// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <EnhancedInputComponent.h>

#include "UR_InputConfig.h"

#include "UR_InputComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UEnhancedInputLocalPlayerSubsystem;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UInputComponent provides parameterized delegates and removeability only for Actions.
 * UR_InputComponent implements them for direct key bindings as well.
 *
 * Used for weapon group bindings.
 */
UCLASS(Config = Input)
class UUR_InputComponent
    : public UEnhancedInputComponent
{
    GENERATED_BODY()

public:
    UUR_InputComponent(const FObjectInitializer& ObjectInitializer);

    void AddInputMappings(const UUR_InputConfig* InputConfig, const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
    void RemoveInputMappings(const UUR_InputConfig* InputConfig, const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

    template <class UserClass, typename FuncType>
    void BindNativeAction(const UUR_InputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

    template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
    void BindAbilityActions(const UUR_InputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

    void RemoveBinds(TArray<uint32>& BindHandles);
};

template <class UserClass, typename FuncType>
void UUR_InputComponent::BindNativeAction(const UUR_InputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
    check(InputConfig);
    if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
    {
        BindAction(IA, TriggerEvent, Object, Func);
    }
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UUR_InputComponent::BindAbilityActions(const UUR_InputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
    check(InputConfig);

    for (const FGameInputAction& Action : InputConfig->AbilityInputActions)
    {
        if (Action.InputAction && Action.InputTag.IsValid())
        {
            if (PressedFunc)
            {
                BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
            }

            if (ReleasedFunc)
            {
                BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
            }
        }
    }
}
