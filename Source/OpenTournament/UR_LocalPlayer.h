// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <CommonLocalPlayer.h>

#include "UR_TeamAgentInterface.h"

#include "UR_LocalPlayer.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_MessageHistory;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(BlueprintType)
class OPENTOURNAMENT_API UUR_LocalPlayer : public UCommonLocalPlayer, public IUR_TeamAgentInterface
{
    GENERATED_BODY()

    UUR_LocalPlayer(const FObjectInitializer& ObjectInitializer);

public:
    /**
    * Reference to message history subobject.
    */
    UPROPERTY(BlueprintReadOnly)
    UUR_MessageHistory* MessageHistory;

    UPROPERTY(Config, BlueprintReadOnly)
    FString PlayerName;

    virtual FString GetNickname() const override;

    UFUNCTION(BlueprintCallable)
    void SavePlayerName(const FString& NewName)
    {
        PlayerName = NewName;
        SaveConfig();
    }
};
