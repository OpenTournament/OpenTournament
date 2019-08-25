// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "UR_Lift.generated.h"

UCLASS()
class OPENTOURNAMENT_API AUR_Lift : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUR_Lift(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Lift")
	float TravelDuration = 1;

	UPROPERTY(EditAnywhere, Category = "Lift", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float StoppedAtEndPosition = 2;
	
	UPROPERTY(EditAnywhere, Category = "Lift")
	FVector EndRelativeLocation;

	UPROPERTY(EditAnywhere, Category = "Lift")
	bool EaseIn;

	UPROPERTY(EditAnywhere, Category = "Lift")
	bool EaseOut;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* BaseMeshComponent;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UBoxComponent* BaseTrigger;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	FVector startLocation;

	bool bIsTriggered;

	TArray<AActor*> actorsOnTrigger;

	FTimerHandle returnTimerHandle;

	enum ELiftState {
		Start,
		Moving,
		End
	};

	ELiftState liftState;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

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
