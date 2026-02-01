// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Pickup.h"
#include "UR_Armor.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UAudioComponent;
class UCapsuleComponent;
class USoundBase;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class OPENTOURNAMENT_API AUR_Armor
    : public AUR_Pickup
{
    GENERATED_BODY()

public:
    AUR_Armor(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UCapsuleComponent> CollisionComponentX;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UAudioComponent> AudioComponentX;

    /** Pickup mesh: 3rd person view */
    UPROPERTY(VisibleDefaultsOnly, Category = "Pickup")
    TObjectPtr<UStaticMeshComponent> StaticMeshComponentX;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /*UFUNCTION()
    void OnOverlap(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void Pickup(UUR_ArmorComponent* ArmorComponent);

    UFUNCTION(BlueprintImplementableEvent, Category = "Armor")
    void OnPickup();*/

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Armor")
    int32 ArmorValue;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Armor")
    bool IsBarrier;
};
