// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <EnhancedInputComponent.h>

#include <UR_InputConfig.h>

#include "UR_InputComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UEnhancedInputLocalPlayerSubsystem;
class UUR_InputConfig;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UInputComponent provides parameterized delegates and removeability only for Actions.
 * UR_InputComponent implements them for direct key bindings as well.
 *
 * Used for weapon group bindings.
 */
UCLASS(Config = Input)
class OPENTOURNAMENT_API UUR_InputComponent
    : public UEnhancedInputComponent
{
    GENERATED_BODY()

public:

    UUR_InputComponent(const FObjectInitializer& ObjectInitializer);

    void AddInputMappings(const UUR_InputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
    void RemoveInputMappings(const UUR_InputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

    template<class UserClass, typename FuncType>
    void BindNativeAction(const UUR_InputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

    template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
    void BindAbilityActions(const UUR_InputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

    void RemoveBinds(TArray<uint32>& BindHandles);

    ////

    template <class DelegateType, class UserClass, typename... VarTypes>
    FInputKeyBinding& BindKeyParameterized(const FInputChord Chord, const EInputEvent KeyEvent, UserClass* Object, typename DelegateType::template TUObjectMethodDelegate<UserClass>::FMethodPtr Func, VarTypes... Vars)
    {
        FInputKeyBinding KB(Chord, KeyEvent);
        KB.KeyDelegate.BindDelegate<DelegateType>(Object, Func, Vars...);
        KeyBindings.Emplace(MoveTemp(KB));
        return KeyBindings.Last();
    }

    /**
    * NOTE: Returns a reference to the added element in the Array. That reference is volatile.
    * It can be used to alter the binding right after adding it, but don't store it. Store a copy instead.
    */
    template <class DelegateType, class UserClass, typename... VarTypes>
    FInputKeyBinding& BindKeyParameterized(const FKey Key, const EInputEvent KeyEvent, UserClass* Object, typename DelegateType::template TUObjectMethodDelegate<UserClass>::FMethodPtr Func, VarTypes... Vars)
    {
        return BindKeyParameterized<DelegateType, UserClass, VarTypes...>(FInputChord(Key, false, false, false, false), KeyEvent, Object, Func, Vars...);
    }

    bool RemoveKeyBinding(const FInputKeyBinding& BindingToRemove)
    {
        for (int32 i = 0; i < KeyBindings.Num(); i++)
        {
            // FInputKeyBinding doesn't implement == but FInputChord does
            if (KeyBindings[i].Chord == BindingToRemove.Chord && KeyBindings[i].KeyEvent == BindingToRemove.KeyEvent)
            {
                KeyBindings.RemoveAt(i);
                return true;
            }
        }
        return false;
    }
};


template<class UserClass, typename FuncType>
void UUR_InputComponent::BindNativeAction(const UUR_InputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
    check(InputConfig);
    if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
    {
        BindAction(IA, TriggerEvent, Object, Func);
    }
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
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
