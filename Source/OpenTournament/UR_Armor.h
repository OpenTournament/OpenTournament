// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "UR_Character.h"
#include "Engine.h"
#include "Engine/Canvas.h" // for FCanvasIcon
#include "UR_Armor.generated.h"

UCLASS()
class OPENTOURNAMENT_API AUR_Armor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUR_Armor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere)
	UShapeComponent* Tbox;

	UPROPERTY(EditAnywhere)
	UAudioComponent* Sound;

	UPROPERTY(EditAnywhere)
	AUR_Character* PlayerController;

	int32 armorVal = 0;

	UPROPERTY(EditAnywhere)
	bool armorBarrier = false;

	bool bItemIsWithinRange = false;

	void Pickup();

	void GetPlayer(AActor* Player);

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* SM_TBox;

	UFUNCTION()
		void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnTriggerExit(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		UStaticMeshComponent* ArmorMesh;

	/** Returns Mesh3P subobject **/
	FORCEINLINE UStaticMeshComponent* GetMesh() const { return ArmorMesh; }

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
