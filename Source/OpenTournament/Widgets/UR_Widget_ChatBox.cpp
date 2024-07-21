// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Widget_ChatBox.h"
#include "InputAction.h"
#include <EnhancedInputComponent.h>
#include <Components/InputComponent.h>

#include "Components/VerticalBox.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_Widget_ChatBox::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // @! TODO : Widgets shouldn't be adding bindings to Input in this way, we should have some other ChatComponent or something which handles that aspect
    if (const APlayerController* PC = Cast<APlayerController>(GetOwningPlayer()); IsValid(PC))
    {
        if (auto Input = PC->InputComponent)
        {
            if (auto EnhancedInputComponent = Cast<UEnhancedInputComponent>(Input))
            {
                // UInputAction
                EnhancedInputComponent->BindAction(InputActionBeginSay, ETriggerEvent::Triggered, this, &UUR_Widget_ChatBox::OnBeginSay);
                EnhancedInputComponent->BindAction(InputActionBeginTeamSay, ETriggerEvent::Triggered, this, &UUR_Widget_ChatBox::OnBeginTeamSay);
            }
        }
    }
}
