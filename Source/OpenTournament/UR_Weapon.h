// Copyright 2019 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "UR_Character.h"
#include "UR_Projectile.h"
#include "Engine.h"
#include "Engine/Canvas.h" // for FCanvasIcon

#include "UR_Weapon.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Weapon;

namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Equipping,
	};
}


/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API AUR_Weapon : public AActor
{
	GENERATED_BODY()

	enum class EAmmoType
	{
		EBullet,
		ERocket,
		EMax,
	};

	virtual void PostInitializeComponents() override;

	virtual EAmmoType GetAmmoType() const
	{
		return EAmmoType::EBullet;
	}

	bool CanFire() const;




public:	
	AUR_Weapon(const FObjectInitializer & ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	AUR_Character* PlayerController;

	UPROPERTY(EditAnywhere)
	UShapeComponent* Tbox;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	int ammoCount;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	USkeletalMeshComponent* SM_TBox;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	FString WeaponName = FString(TEXT(""));

	UPROPERTY(EditAnywhere, Category = "Weapon")
	FString AmmoName = FString(TEXT(""));

	UPROPERTY(EditAnywhere, Category = "Weapon")
	UAudioComponent* Sound;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	UAudioComponent* SoundFire;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class AUR_Projectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	FVector Location;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	FRotator Rotation;

	bool bItemIsWithinRange = false;

	bool equipped = false;

	UFUNCTION()
	void Pickup();

	virtual void Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams) {};

	UFUNCTION()
	void setEquipped(bool eq);

	UFUNCTION()
	virtual void OnEquip(AUR_Weapon* LastWeapon);

	UFUNCTION()
	virtual void OnUnEquip();

	UFUNCTION()
	bool IsEquipped() const;

	UFUNCTION()
	bool IsAttachedToPawn() const;

	UFUNCTION()
	void AttachMeshToPawn();

	UFUNCTION()
	void AttachWeaponToPawn();

	UFUNCTION()
	void DetachMeshFromPawn();

	UFUNCTION()
	void GetPlayer(AActor* Player);

	UFUNCTION()
		void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnTriggerExit(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	/** get current weapon state */
	EWeaponState::Type GetCurrentState() const;

	/** get current ammo amount (total) */
	int32 GetCurrentAmmo() const;

	/** get max ammo amount */
	int32 GetMaxAmmo() const;

	/** get weapon mesh (needs pawn owner to determine variant) */
	USkeletalMeshComponent* GetWeaponMesh() const;

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class AUR_Character* GetPawnOwner() const; //TODO: Check this

	/** set the weapon's owning pawn */
	void SetOwningPawn(AUR_Character* AURCharacter);

protected:
	/** weapon mesh: 1st person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh3P;

	/** Returns Mesh1P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns Mesh3P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
