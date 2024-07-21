// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Blueprint/UserWidget.h>
#include "UR_Widget_ChatBox.generated.h"


class UInputAction;
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Chat Box Widget
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Widget_ChatBox : public UUserWidget
{
    GENERATED_BODY()

    virtual void NativeOnInitialized() override;

public:
    /**
    * Function bound to "BeginSay" ActionMapping input.
    * Blueprint implementable.
    */
    UFUNCTION(BlueprintImplementableEvent)
    void OnBeginSay();

    /**
    * Function bound to "BeginTeamSay" ActionMapping input.
    * Blueprint implementable.
    */
    UFUNCTION(BlueprintImplementableEvent)
    void OnBeginTeamSay();

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionBeginSay;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionBeginTeamSay;
};
