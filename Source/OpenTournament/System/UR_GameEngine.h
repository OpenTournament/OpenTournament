// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/GameEngine.h"

#include "UR_GameEngine.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class IEngineLoop;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_GameEngine : public UGameEngine
{
    GENERATED_BODY()

public:
    UUR_GameEngine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    virtual void Init(IEngineLoop* InEngineLoop) override;
};
