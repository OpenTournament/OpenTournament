// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_Weapon.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class AUR_Character;
class AUR_Projectile;
class UShapeComponent;
class UAudioComponent;
class USkeletalMeshComponent;
class USoundBase;
class UFXSystemAsset;

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace EWeaponState
{
    enum Type
    {
        Idle,
        Firing,
        Equipping,
    };
}

USTRUCT()
struct FReplicatedHitscanInfo
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Start;

    UPROPERTY()
    FVector End;

    UPROPERTY()
    FVector ImpactNormal;

    FReplicatedHitscanInfo() {}
    FReplicatedHitscanInfo(const FVector& Start, const FVector& End, const FVector& ImpactNormal)
        : Start(Start), End(End), ImpactNormal(ImpactNormal) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Weapon Base Class
 */
UCLASS()
class OPENTOURNAMENT_API AUR_Weapon : public AActor
{
    GENERATED_BODY()

public:	
    AUR_Weapon(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    enum class EAmmoType
    {
        EBullet,
        ERocket,
        EMax,
    };

    virtual EAmmoType GetAmmoType() const
    {
        return EAmmoType::EBullet;
    }

    bool CanFire() const;


protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Owner() override;

    UFUNCTION()
    virtual void OnRep_Equipped();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:

    UPROPERTY(EditAnywhere)
    UShapeComponent* Tbox;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    UAudioComponent* Sound;

    UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly, Category = "Weapon")
    int32 AmmoCount;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    FString WeaponName;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    FString AmmoName;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    USoundBase* PickupSound;

    //TODO: this will be per-firemode
    UPROPERTY(EditAnywhere, Category = "Weapon")
    USoundBase* FireSound;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    FName MuzzleSocketName;

    //TODO: this will be per-firemode
    UPROPERTY(EditAnywhere, Category = "Weapon")
    UParticleSystem* MuzzleFlashFX;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AUR_Projectile> ProjectileClass;

    bool bItemIsWithinRange = false;

    UPROPERTY(ReplicatedUsing = OnRep_Equipped)
    bool bIsEquipped = false;

    UFUNCTION()
    void Pickup();

    UFUNCTION(BlueprintAuthorityOnly)
    void GiveTo(AUR_Character* NewOwner);

    UFUNCTION()
    virtual void Fire();

    UFUNCTION()
    void SetEquipped(bool bEquipped);

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

    UPROPERTY(EditAnywhere)
    AUR_Character* URCharOwner;


    /** get current weapon state */
    EWeaponState::Type GetCurrentState() const;

    /** get current ammo amount (total) */
    int32 GetCurrentAmmo() const;

    /** get max ammo amount */
    int32 GetMaxAmmo() const;

    /** get weapon mesh (needs pawn owner to determine variant) */
    USkeletalMeshComponent* GetWeaponMesh() const;

    /** get pawn owner */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game|Weapon")
    AUR_Character* GetPawnOwner() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game|Weapon")
    bool IsLocallyControlled() const;

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

    /////////////////////////////////////////////////////////////////////////////////////////////////

    //============================================================
    // Basic firing loop with network support.
    // - Not fit for fast fire-rate like mini/beams.
    // - No support for custom systems like charging.
    // - No blueprint support yet.
    //============================================================
public:

    UPROPERTY()
    bool bFiring;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    float FireInterval;

    /**
    * Used to calculate server response time to our ServerFire() call, and adjust fire loop.
    * Owner client only.
    */
    UPROPERTY()
    float LocalFireTime;

    /**
    * Used to make sure server/authority never fires faster than FireInterval.
    * Authority only.
    */
    UPROPERTY()
    float LastFireTime;

    FTimerHandle FireLoopTimerHandle;
    FTimerHandle DelayedFireTimerHandle;

    /**
    * Start the fire loop on owning client.
    */
    UFUNCTION()
    virtual void LocalStartFire();

    /**
    * Stop the fire loop on owning client.
    */
    UFUNCTION()
    virtual void LocalStopFire();

    /**
    * Fire loop.
    * Fully simulated on owning client.
    */
    UFUNCTION()
    virtual void LocalFireLoop();

    /**
    * Simulate fire on owning client.
    */
    UFUNCTION()
    virtual void LocalFire();

    /**
    * Fire on server.
    */
    UFUNCTION(Server, Reliable)
    void ServerFire();

    /**
    * Consume ammo for shot.
    */
    UFUNCTION(BlueprintAuthorityOnly)
    virtual void ConsumeAmmo();

    /**
    * Server just fired.
    * Remote clients should spawn muzzle flash effect.
    * Owner client should update his fire loop timer to avoid lingering desync.
    */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastFired_Projectile();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFired_Hitscan(const FReplicatedHitscanInfo& HitscanInfo);

    /**
    * Factor code for both above methods :
    * Owner client should update his fire loop timer to avoid lingering desync.
    */
    UFUNCTION()
    void LocalConfirmFired();

    /**
    * Play fire sound, muzzle flash.
    * Client only.
    */
    UFUNCTION(BlueprintCosmetic)
    virtual void PlayFireEffects();

    /**
    * Play hitscan effects (beam, impact).
    */
    UFUNCTION(BlueprintCosmetic)
    virtual void PlayHitscanEffects(const FReplicatedHitscanInfo& HitscanInfo);

    //============================================================
    // Helper methods
    //============================================================

    UFUNCTION()
    virtual void GetFireVector(FVector& FireLoc, FRotator& FireRot);

    /**
    * Spawn projectile.
    * Authority only.
    */
    UFUNCTION(BlueprintAuthorityOnly)
    virtual void SpawnShot_Projectile();

    /**
    * Perform hitscan trace and apply damage.
    * Authority only.
    */
    UFUNCTION(BlueprintAuthorityOnly)
    virtual void SpawnShot_Hitscan(FReplicatedHitscanInfo& OutHitscanInfo);

    /**
    * 
    */
    UFUNCTION()
    void HitscanTrace(FHitResult& OutHit);

    /**
    * On hitscan trace overlap,
    * Return whether hitscan should hit target or fire through.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
    bool HitscanShouldHitActor(AActor* Other);

    /**
    * For hitscan test implem.
    */
    UPROPERTY(EditAnywhere, Category = "Weapon")
    UFXSystemAsset* BeamTemplate;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    UParticleSystem* BeamImpactTemplate;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    USoundBase* BeamImpactSound;
};
