// Copyright (c) Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameVerbMessage.h"
#include "UR_ActivatableWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UR_GameplayMessageDisplay.generated.h"


UCLASS(Blueprintable)
class OPENTOURNAMENT_API UUR_GameplayMessageDisplay : public UCommonUserWidget
{
    GENERATED_BODY()

protected:

    UPROPERTY(EditDefaultsOnly)
    FGameplayTagContainer GameplayMessagesToListenFor;

    UFUNCTION(BlueprintImplementableEvent)
    void MessageReceived(FGameplayTag Tag, const FGameVerbMessage& Message);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:

    TArray<FGameplayMessageListenerHandle> ListenerHandles;
};
