// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Perception/AIPerceptionStimuliSourceComponent.h"

#include "AIPerceptionSourceNativeComp.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Native wrapper around AIPerceptionStimuliSourceComponent because its native API sucks.
*/
UCLASS(ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UAIPerceptionSourceNativeComp
    : public UAIPerceptionStimuliSourceComponent
{
    GENERATED_BODY()

public:
    void SetAutoRegisterAsSource(bool bAuto)
    {
        bAutoRegisterAsSource = bAuto;
    }

    void SetRegisterAsSourceForSenses(TArray<TSubclassOf<UAISense>> Senses)
    {
        if (bSuccessfullyRegistered)
        {
            for (auto& Sense : Senses)
            {
                if (Sense)
                    RegisterForSense(Sense);
            }
        }
        else
        {
            RegisterAsSourceForSenses = Senses;
        }
    }

protected:
    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override
    {
        UnregisterFromPerceptionSystem();
        Super::OnComponentDestroyed(bDestroyingHierarchy);
    }
};
