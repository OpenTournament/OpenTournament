// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OpenTournament.h"


#include "UR_ArmorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OPENTOURNAMENT_API UUR_ArmorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUR_ArmorComponent();

protected:
	// Called when the game starts
	//virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	* Armor value.
	*/
	UPROPERTY(Replicated, BlueprintReadOnly, EditDefaultsOnly, Category = "ArmorComponent")
	int32 Armor;
	UPROPERTY(Replicated, BlueprintReadOnly, EditDefaultsOnly, Category = "ArmorComponent")
	bool hasBarrier;

	/**
	* Armor Maximum value.
	*/
	UPROPERTY(Replicated, BlueprintReadOnly, EditDefaultsOnly, Category = "ArmorComponent")
	int32 ArmorMax;

	/**
	* Set armor to a numerical value
	*/
	UFUNCTION(BlueprintCallable, Category = "ArmorComponent")
	void SetArmor(const int32 InValue);

	/**
	* Change armor by a value
	*/
	UFUNCTION(BlueprintCallable, Category = "ArmorComponent")
	void ChangeArmor(const int32 InChangeValue);

	/**
	* Change armor by a value
	*/
	UFUNCTION(BlueprintCallable, Category = "ArmorComponent")
		void SetBarrier(bool barrier);

		
};
