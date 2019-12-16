// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "UR_Type_LiftState.h"
#include "UR_Lift.generated.h"

UCLASS()
class OPENTOURNAMENT_API AUR_Lift : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUR_Lift(const FObjectInitializer& ObjectInitializer);

	/*
	* Static Mesh Component - Lift Base
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lift", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComponent;

	/*
	* Box Component - Active Teleport Region
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lift", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxComponent;

	/*
	* Audio Component
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lift", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* AudioComponent;

	/*
	* Duration of the Lift travel between Start and End positions
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lift")
	float TravelDuration = 1;

	/*
	* Duration that Lift will wait at the End position before returning
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lift", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float StoppedAtEndPosition = 2;

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	FVector startLocation;

	bool bIsTriggered;

	TArray<AActor*> actorsOnTrigger;

	FTimerHandle returnTimerHandle;

	ELiftState liftState;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

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

};
