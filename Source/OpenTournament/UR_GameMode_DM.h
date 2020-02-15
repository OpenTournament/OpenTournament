// Copyright (c) 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UR_GameMode.h"
#include "UR_GameMode_DM.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * DeathMatch GameMode
 */
UCLASS()
class OPENTOURNAMENT_API AUR_GameMode_DM : public AUR_GameMode
{
    GENERATED_BODY()

public:

    virtual void PlayerKilled_Implementation(AController* Victim, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser) override;

    UFUNCTION(BlueprintNativeEvent)
    void ScoreKill(AController* Victim, AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser);

    virtual AActor* IsThereAWinner_Implementation() override;
};
