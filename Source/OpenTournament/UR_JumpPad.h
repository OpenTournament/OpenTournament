// Copyright 2019 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_JumpPad.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class UAudioComponent;
class UCapsuleComponent;
class USoundBase;
class UStaticMeshComponent;
class UParticleSystemComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class OPENTOURNAMENT_API AUR_JumpPad : public AActor
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = JumpPad)
    USceneComponent* SceneRoot;

    /*
    * Static Mesh Component - JumpPad Base
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "JumpPad", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    /*
    * Capsule Component - Active JumpPad Region
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "JumpPad", meta = (AllowPrivateAccess = "true"))
    UCapsuleComponent* CapsuleComponent;
    
    /*
    * Audio Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "JumpPad", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    /*
    * ParticleSystem Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "JumpPad", meta = (AllowPrivateAccess = "true"))
    UParticleSystemComponent* ParticleSystemComponent;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    * Destination of the Jump - May be another JumpPad, TargetPoint, etc.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad", meta = (MakeEditWidget = ""))
    FTransform Destination;

    /** if set then lock the Destination when moving/rotating the JumpPad */
    UPROPERTY(EditAnywhere, Category = "JumpPadEditor")
    bool bLockDestination;

    /*
    * Duration of the Jump - in seconds
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "JumpPad")
    float JumpTime;

    /**
    * Actor is launched by JumpPad
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad")
    USoundBase* JumpPadLaunchSound;

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    // Sets default values for this actor's properties
    AUR_JumpPad(const FObjectInitializer& ObjectInitializer);

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    /**
    * Play Effects on Successful Jump
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "JumpPad")
    void PlayJumpPadEffects();

    /**
    * Is this actor permitted to jump? 
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure,, BlueprintCallable, Category = "JumpPad")
    bool IsPermittedToJump(const AActor* InCharacter) const;

    /**
    * Calculate our Jump Velocity
    */
    FVector CalculateJumpVelocity(const AActor* InCharacter);

    UFUNCTION()
    void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
    virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif
};
