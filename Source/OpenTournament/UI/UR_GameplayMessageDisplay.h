// Copyright (c) Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameVerbMessage.h"
#include "UR_ActivatableWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UR_GameplayMessageDisplay.generated.h"


UCLASS(Blueprintable)
class OPENTOURNAMENT_API UUR_GameplayMessageDisplay : public UUR_ActivatableWidget
{
    GENERATED_BODY()

protected:

    UPROPERTY(EditDefaultsOnly)
    FGameplayTagContainer GameplayMessagesToListenFor;

    UFUNCTION(BlueprintImplementableEvent)
    void MessageReceived(FGameplayTag Tag, const FGameVerbMessage& Message);

    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;

private:

    TArray<FGameplayMessageListenerHandle> ListenerHandles;
};
