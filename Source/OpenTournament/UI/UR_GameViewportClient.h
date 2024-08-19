// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CommonGameViewportClient.h"

#include "UR_GameViewportClient.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UGameInstance;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(BlueprintType)
class UUR_GameViewportClient : public UCommonGameViewportClient
{
    GENERATED_BODY()

public:
    UUR_GameViewportClient();

    virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};
