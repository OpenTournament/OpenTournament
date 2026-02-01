// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include <Engine/DataAsset.h>

#include "UR_DamagePopStyleNiagara.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UNiagaraSystem;

/////////////////////////////////////////////////////////////////////////////////////////////////

/* PopStyle is used to define what Niagara asset should be used for the Damage System representation */
UCLASS()
class UUR_DamagePopStyleNiagara : public UDataAsset
{
    GENERATED_BODY()

public:
    // Name of the Niagara Array to set the Damage information
    UPROPERTY(EditDefaultsOnly, Category="DamagePop")
    FName NiagaraArrayName;

    // Niagara System used to display the damages
    UPROPERTY(EditDefaultsOnly, Category="DamagePop")
    TObjectPtr<UNiagaraSystem> TextNiagara;
};
