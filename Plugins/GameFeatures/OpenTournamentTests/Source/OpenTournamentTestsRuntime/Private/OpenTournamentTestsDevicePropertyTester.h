// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/InputDevicePropertyHandle.h"
#include "OpenTournamentTestsDevicePropertyTester.generated.h"

class UInputDeviceProperty;
class UCapsuleComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
struct FHitResult;

/** This tester will apply device properties to a Player Controller on overlap, and remove them once overlap ends. */
UCLASS(Blueprintable, BlueprintType)
class AOpenTournamentTestsDevicePropertyTester : public AActor
{
	GENERATED_BODY()

public:

	AOpenTournamentTestsDevicePropertyTester();

	/** Device properties to apply on overlap with a player controller. */
	UPROPERTY(EditAnywhere, Category = "Device Property")
	TArray<TSubclassOf<UInputDeviceProperty>> DeviceProperties;

	/** The volume that will trigger device properties to be added and removed on overlap */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Device Property")
	TObjectPtr<UCapsuleComponent> CollisionVolume;

	/** A little mesh to make this collision volume visible */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Device Property")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	UFUNCTION(BlueprintCallable, Category = "Device Property")
	void ApplyDeviceProperties(const FPlatformUserId UserId);

	UFUNCTION(BlueprintCallable, Category = "Device Property")
	void RemoveDeviceProperties();

private:

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(Transient)
	TSet<FInputDevicePropertyHandle> ActivePropertyHandles;
};
