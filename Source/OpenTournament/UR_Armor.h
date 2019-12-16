// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_Armor.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UAudioComponent;
class UCapsuleComponent;
class USoundBase;
class UUR_ArmorComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class OPENTOURNAMENT_API AUR_Armor : public AActor
{
    GENERATED_BODY()
    
public:

    AUR_Armor(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(EditDefaultsOnly)
    UCapsuleComponent* CollisionComponent;

    UPROPERTY(EditDefaultsOnly)
    UAudioComponent* AudioComponent;

    /** Pickup mesh: 3rd person view */
    UPROPERTY(VisibleDefaultsOnly, Category = "Pickup")
    UStaticMeshComponent* StaticMeshComponent;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION()
    void OnOverlap(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void Pickup(UUR_ArmorComponent* ArmorComponent);

    UFUNCTION(BlueprintImplementableEvent, Category = "Armor")
    void OnPickup();

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Armor")
    USoundBase* PickupSound;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Armor")
    int32 ArmorValue;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Armor")
    bool IsBarrier;
};
