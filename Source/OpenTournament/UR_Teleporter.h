// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UR_Type_ExitRotation.h"

#include "UR_Teleporter.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UArrowComponent;
class UAudioComponent;
class UCapsuleComponent;
class USoundBase;
class UStaticMeshComponent;
class UParticleSystemComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class OPENTOURNAMENT_API AUR_Teleporter : public AActor
{
    GENERATED_BODY()

public:

    /*
    * Static Mesh Component - Teleporter Base
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    /*
    * Capsule Component - Active Teleport Region
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UCapsuleComponent* CapsuleComponent;

    /*
    * Audio Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    /*
    * Arrow Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UArrowComponent* ArrowComponent;

    /*
    * ParticleSystem Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UParticleSystemComponent* ParticleSystemComponent;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    * Destination of Teleport - May be another Teleporter, TargetPoint, etc.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Teleporter")
    AActor* DestinationActor;

    /*
    * Does our teleport force us into a new rotation, or is it relative to our DestinationActor's rotation?
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Teleporter")
    EExitRotation ExitRotationType;

    /*
    * Do Actors teleported retain their velocity?
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Teleporter")
    bool bKeepMomentum = true;

    /**
    * Actor teleports out
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter")
    USoundBase* TeleportOutSound;

    /**
    * Actor teleports in
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter")
    USoundBase* TeleportInSound;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Sets default values for this actor's properties
    AUR_Teleporter(const FObjectInitializer& ObjectInitializer);

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    bool PerformTeleport(AActor* TargetActor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Teleporter")
    void PlayTeleportEffects();

    void GetDesiredRotation(FRotator& DesiredRotation, const FRotator& TargetActorRotation, const FRotator& DestinationRotation);

    UFUNCTION()
    void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
