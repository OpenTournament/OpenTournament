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

	virtual void Destroyed() override;


	/** add ammo */
	void GiveAmmo(int AddAmount);

	/** consume a bullet */
	void UseAmmo();

	/** query ammo type */
	virtual EAmmoType GetAmmoType() const
	{
		return EAmmoType::EBullet;
	}

	/** start weapon fire */
	virtual void StartFire();

	/** stop weapon fire */
	virtual void StopFire();

	/** check if weapon can fire */
	bool CanFire() const;




public:	
	// Sets default values for this actor's properties
	AUR_Weapon(const FObjectInitializer & ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	AUR_Character* PlayerController;

	UShapeComponent* Tbox;

	int ammoCount;

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* SM_TBox;

	UPROPERTY(EditAnywhere)
	FString WeaponName = FString(TEXT(""));

	UPROPERTY(EditAnywhere)
	FString AmmoName = FString(TEXT(""));

	UAudioComponent* Sound;
	UAudioComponent* SoundFire;


	TSubclassOf<class AUR_Projectile> ProjectileClass;

	FVector Location;
	FRotator Rotation;

	void Pickup();

	virtual void Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams) {};


	void setEquipped(bool eq);

	/** weapon is being equipped by owner pawn */
	virtual void OnEquip(AUR_Weapon* LastWeapon);

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	/** check if it's currently equipped */
	bool IsEquipped() const;

	/** check if mesh is already attached */
	bool IsAttachedToPawn() const;

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** attaches weapon object to pawn */
	void AttachWeaponToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

	void GetPlayer(AActor* Player);

	bool bItemIsWithinRange = false;

	bool equipped = false;

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
