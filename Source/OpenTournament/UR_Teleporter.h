// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "UR_Teleporter.generated.h"

UCLASS()
class OPENTOURNAMENT_API AUR_Teleporter : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AUR_Teleporter(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(EditAnywhere, Category = "Teleporter")
	AUR_Teleporter* DestinationTeleporter;

	UPROPERTY(EditAnywhere, Category = "Teleporter", meta = (UIMin = "0.0", UIMax = "360.0"))
	float EnterRotation;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* BaseMeshComponent;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UBoxComponent* BaseTrigger;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
