// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <EnhancedInputComponent.h>

#include "CoreMinimal.h"
#include "Components/InputComponent.h"

#include "UR_InputComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UInputComponent provides parameterized delegates and removeability only for Actions.
 * UR_InputComponent implements them for direct key bindings as well.
 *
 * Used for weapon group bindings.
 */
UCLASS()
class OPENTOURNAMENT_API UUR_InputComponent
    : public UEnhancedInputComponent
    //: public UInputComponent
{
    GENERATED_BODY()

public:
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
