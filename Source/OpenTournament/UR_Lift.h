// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <GameFramework/Actor.h>

#include "UR_Type_LiftState.h"
#include "UR_Lift.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAudioComponent;
class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class USoundBase;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
*
*/
UCLASS()
class OPENTOURNAMENT_API AUR_Lift : public AActor
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:

    AUR_Lift(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    * Static Mesh Component - Lift Base
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lift", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    /*
    * Box Component - Active Trigger Region
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lift", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* BoxComponent;

    /*
    * Audio Component
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lift", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void BeginPlay() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Lift")
    void PlayLiftEffects();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Lift")
    void StopLiftEffects();

    UFUNCTION()
    void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerExit(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable)
    void OnReachedStart();

    UFUNCTION(BlueprintCallable)
    void OnReachedEnd();

private:

    void MoveToStartPosition();
    void MoveToEndPosition();

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /*
    * Duration of the Lift travel between Start and End positions
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lift")
    float TravelDuration;

    /*
    * Duration that Lift will wait at the End position before returning
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lift", meta = (ClampMin = "0.01", UIMin = "0.01"))
    float StoppedAtEndPosition;

    /**
    * Relative Position at which the Lift stops
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lift")
    FVector EndRelativeLocation;

    /*
    * Ease the beginning of the movement of the Lift
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lift")
    bool EaseIn;

    /*
    * Ease the end of the movement of the Lift
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lift")
    bool EaseOut;

    /**
    * Lift starts moving
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift")
    USoundBase* LiftStartSound;

    /**
    * Lift is moving
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift")
    USoundBase* LiftMovingSound;

    /**
    * Lift reaches end
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift")
    USoundBase* LiftEndSound;

private:

    UPROPERTY()
    FVector StartLocation;

    UPROPERTY()
    bool bIsTriggered;

    UPROPERTY()
    TArray<AActor*> ActorsOnTrigger;

    UPROPERTY()
    ELiftState LiftState;

    UPROPERTY()
    FTimerHandle ReturnTimerHandle;
};
