// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Widget_ChatBox.h"

#include <Components/InputComponent.h>

#include "Components/VerticalBox.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_Widget_ChatBox::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    const APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
    if (PC && PC->InputComponent)
    {
        PC->InputComponent->BindAction("BeginSay", IE_Pressed, this, &UUR_Widget_ChatBox::OnBeginSay);
        PC->InputComponent->BindAction("BeginTeamSay", IE_Pressed, this, &UUR_Widget_ChatBox::OnBeginTeamSay);
    }
}
