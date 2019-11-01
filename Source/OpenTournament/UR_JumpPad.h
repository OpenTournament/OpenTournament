// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "UR_Character.h"
#include "UR_JumpPad.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

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
	// Sets default values for this actor's properties
	AUR_JumpPad(const FObjectInitializer& ObjectInitializer);

	/*
	* Static Mesh Component - JumpPad Base
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jump Pad", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComponent;

	/*
	* Capsule Component - Active JumpPad Region
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jump Pad", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* CapsuleComponent;
	
	/*
	* Audio Component
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teleporter", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* AudioComponent;

	/*
	* ParticleSystem Component
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teleporter", meta = (AllowPrivateAccess = "true"))
	UParticleSystemComponent* ParticleSystemComponent;

	/*
	* Destination of the
	Jump - May be another JumpPad, TargetPoint, etc.
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Jump Pad")
	FVector Destination;

	/*
	* Duration of the Jump - in seconds
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Jump Pad")
	float JumpTime;

	/**
	* Actor is launched by JumpPad
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump Pad")
	USoundBase* JumpPadLaunchSound;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Jump Pad")
	void PlayJumpPadEffects();

	FVector CalculateJumpVelocity(AActor* character);

	UFUNCTION()
	void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
