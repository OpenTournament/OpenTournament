// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_NumberPopComponent.h"

#include "UR_NumberPopComponent_NiagaraText.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_DamagePopStyleNiagara;
class UNiagaraComponent;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Blueprintable)
class UUR_NumberPopComponent_NiagaraText : public UUR_NumberPopComponent
{
    GENERATED_BODY()

public:
    UUR_NumberPopComponent_NiagaraText(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~UUR_NumberPopComponent interface
    virtual void AddNumberPop(const FGameNumberPopRequest& NewRequest) override;
    //~End of UUR_NumberPopComponent interface

protected:
    TArray<int32> DamageNumberArray;

    /** Style patterns to attempt to apply to the incoming number pops */
    UPROPERTY(EditDefaultsOnly, Category = "NumberPop|Style")
    TObjectPtr<UUR_DamagePopStyleNiagara> Style;

    //Niagara Component used to display the damage
    UPROPERTY(EditDefaultsOnly, Category = "NumberPop|Style")
    TObjectPtr<UNiagaraComponent> NiagaraComp;
};
