// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_FireModeBase.h"
#include "UR_FireModeBasic.h"
#include "UR_FireModeCharged.h"
#include "UR_FireModeContinuous.h"

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
class UAnimMontage;

/////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    /** Weapon is not in the hands */
    Inactive,
    /** Weapon is currently being equipped, will be able to fire soon */
    BringUp,
    /** Weapon can start firing anytime */
    Idle,
    /** Weapon is currently firing/charging/on cooldown - a FireMode is active */
    Firing,
    /** Generic state to prevent firing, can be used to implement eg. reloading */
    Busy,
    /** Weapon is currently being unequipped */
    PutDown,

    MAX             UMETA(Hidden)
};

/**
* Event dispatcher.
* Notify the weapon has changed state.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponStateChangedSignature, AUR_Weapon*, Weapon, EWeaponState, NewState);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Weapon Base Class
 */
UCLASS()
class OPENTOURNAMENT_API AUR_Weapon : public AActor
    //, public IUR_FireModeBaseInterface
    //, public IUR_FireModeBasicInterface
    , public IUR_FireModeChargedInterface
    , public IUR_FireModeContinuousInterface
{
    GENERATED_BODY()

protected:	
    AUR_Weapon(const FObjectInitializer& ObjectInitializer);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PostInitializeComponents() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // ???

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

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Very, very basic support for picking up weapons on the ground.

protected:

    UPROPERTY(VisibleAnywhere)
    UShapeComponent* TriggerBox;

    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Weapon possession

public:

    UPROPERTY(BlueprintReadOnly)
    AUR_Character* URCharOwner;

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    void GiveTo(AUR_Character* NewOwner);

protected:

    virtual void OnRep_Owner() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // General properties

protected:

    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    USkeletalMeshComponent* Mesh1P;

    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    USkeletalMeshComponent* Mesh3P;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    USoundBase* OutOfAmmoSound;

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    USoundBase* PickupSound;

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_AmmoCount, Category = "Weapon")
    int32 AmmoCount;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    FString WeaponName;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    FString AmmoName;

protected:

    UFUNCTION()
    virtual void OnRep_AmmoCount();

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Some getters

public:

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
    FORCEINLINE int32 GetCurrentAmmo() const { return AmmoCount; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
    FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
    FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
    USkeletalMeshComponent* GetVisibleMesh() const;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Weapon Attachment

public:

    UPROPERTY()
    bool bIsAttached;

protected:

    /**
    * Verify if weapon is attached according to its current state.
    * in state Inactive, ensure weapon is detached.
    * in any other states, ensure weapon is attached.
    *
    * In normal situations we only need to attach on BringUp and detach after PutDown.
    * However this can also be used to support more edgy cases.
    * For example, dropping weapon will go straight to Inactive without the putdown procedure.
    */
    UFUNCTION()
    void CheckWeaponAttachment();

    UFUNCTION()
    void AttachMeshToPawn();

    UFUNCTION()
    void DetachMeshFromPawn();

public:

    /**
    * Update 1P and 3P mesh visibility according to current view mode.
    * If local player is viewing weapon owner in 1P mode, show 1P mesh and hide 3P mesh.
    * Or the contrary.
    */
    UFUNCTION(BlueprintCosmetic)
    void UpdateMeshVisibility();

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    //============================================================
    // Weapon animations & timings
    //============================================================

    UPROPERTY(EditAnywhere, Category = "Weapon")
    UAnimMontage* BringUpMontage;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    float BringUpTime;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    UAnimMontage* PutDownMontage;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    float PutDownTime;

    /**
    * When requesting putdown during cooldown, delay by a percentage of that cooldown.
    * If cooldown is 1 second and this is at 75%, you can putdown 0.75s after firing.
    */
    UPROPERTY(EditAnywhere, Category = "Weapon", Meta = (ClampMin = "0", ClampMax = "1"))
    float CooldownDelaysPutDownByPercent;

    /**
    * Whether to automatically reduce the above delay by the PutDownTime itself.
    * In the above scenario and with 0.3 put down time, you can putdown 0.45s after firing.
    */
    UPROPERTY(EditAnywhere, Category = "Weapon", Meta = (EditCondition = "CooldownDelaysPutDownByPercent>0"))
    bool bReducePutDownDelayByPutDownTime;

    //============================================================
    // WeaponStates Core
    //============================================================

    UPROPERTY(BlueprintReadOnly)
    EWeaponState WeaponState;

    UFUNCTION()
    virtual void SetWeaponState(EWeaponState NewState);

    UPROPERTY(BlueprintAssignable)
    FWeaponStateChangedSignature OnWeaponStateChanged;

    UFUNCTION()
    void BringUp(float FromPosition);

    UFUNCTION()
    void BringUpCallback();

    UFUNCTION()
    void PutDown(float FromPosition);

    UFUNCTION()
    void PutDownCallback();

    /** BringUp/PutDown animation timer */
    FTimerHandle SwapAnimTimerHandle;

    /** Delay before putdown due to cooldown */
    FTimerHandle PutDownDelayTimerHandle;

    /**
    * Activate all fire modes. Use on bring up.
    */
    UFUNCTION()
    virtual void Activate();

    /**
    * Deactivate all fire modes.
    * Use when changing or dropping weapon, or when dying, to ensure no firing loop remains.
    */
    UFUNCTION()
    virtual void Deactivate();

    //============================================================
    // External API
    //============================================================

    /**
    * Request to bring the weapon up.
    * If weapon is Inactive (not equipped), bring up happens immediately.
    * If weapon is on PutDown, bring up also happens immediately, and should recover from its current position.
    * In all other cases, this just cancels any previous call to RequestPutDown, and nothing happens.
    */
    UFUNCTION()
    virtual void RequestBringUp();

    /**
    * Request to put the weapon down whenever possible.
    * After waiting on firemodes cooldowns, weapon will go to PutDown state.
    * Once finished, weapon will go to Inactive state.
    * Use the OnWeaponStateChanged dispatcher to track progress.
    */
    UFUNCTION()
    virtual void RequestPutDown();

    UFUNCTION()
    virtual void RequestStartFire(uint8 FireModeIndex);

    UFUNCTION()
    virtual void RequestStopFire(uint8 FireModeIndex);


    //============================================================
    // Helpers
    //============================================================

    /**
    * Factor method for ammo checking before starting fire,
    * and looping while out of ammo.
    */
    UFUNCTION()
    void TryStartFire(UUR_FireModeBase* FireMode);

    FTimerHandle RetryStartFireTimerHandle;

    UFUNCTION(BlueprintCallable)
    virtual void GetFireVector(FVector& FireLoc, FRotator& FireRot);

    /**
    * Offset projectiles spawn location towards muzzle,
    * with a line trace check to ensure we don't spawn behind geometry.
    */
    UFUNCTION(BlueprintCallable)
    virtual void OffsetFireLoc(UPARAM(ref) FVector& FireLoc, const FRotator& FireRot, FName OffsetSocketName = NAME_None);

    /**
    * Safely read SimulatedInfo passed from client into validated FireLoc and FireRot.
    * Offsetted by socket if specified.
    */
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void GetValidatedFireVector(const FSimulatedShotInfo& SimulatedInfo, FVector& FireLoc, FRotator& FireRot, FName OffsetSocketName = NAME_None);

    UFUNCTION(BlueprintCallable)
    static FVector SeededRandCone(const FVector& Dir, float ConeHalfAngleDeg, int32 Seed);

    UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, BlueprintCallable)
    AUR_Projectile* SpawnProjectile(TSubclassOf<AUR_Projectile> InProjectileClass, const FVector& StartLoc, const FRotator& StartRot);

    UFUNCTION(BlueprintCallable)
    void HitscanTrace(const FVector& TraceStart, const FVector& TraceEnd, FHitResult& OutHit);

    /**
    * On hitscan trace overlap,
    * Return whether hitscan should hit target or fire through.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
    bool HitscanShouldHitActor(AActor* Other);

    UFUNCTION(BlueprintCallable)
    bool HasEnoughAmmoFor(UUR_FireModeBase* FireMode);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void ConsumeAmmo(int32 Amount = 1);

    //============================================================
    // Firemodes
    //============================================================

    /**
    * Array of fire modes components, where indices are reflecting FireMode->Index.
    * Built automatically from the components list.
    * Might contain null elements, if indices are skipped.
    * Eg. if only one firemode with index 1 is present, 0 will be null.
    */
    UPROPERTY(BlueprintReadOnly)
    TArray<UUR_FireModeBase*> FireModes;

    /**
    * Current active firemode. There should only be one at a time.
    * Only relevant in weapon state Firing. Should be null otherwise.
    */
    UPROPERTY(BlueprintReadOnly)
    UUR_FireModeBase* CurrentFireMode;

    /**
    * Stack of desired fire modes (controlled by RequestStartFire/StopFire).
    * First is newest. No duplicates.
    */
    UPROPERTY(BlueprintReadOnly)
    TArray<UUR_FireModeBase*> DesiredFireModes;

    //============================================================
    // FireModeBase interface
    //============================================================

    virtual void FireModeChangedStatus_Implementation(UUR_FireModeBase* FireMode) override;

    /**
    * In BringUp state, return the bring up time left.
    * In Idle state, return 0.
    * In Firing state, return CurrentFireMode's cooldown time left.
    * In other states, return some high value to prevent any shot.
    */
    virtual float TimeUntilReadyToFire_Implementation(UUR_FireModeBase* FireMode) override;

    virtual void BeginSpinUp_Implementation(UUR_FireModeBase* FireMode, float CurrentSpinValue) override;

    virtual void BeginSpinDown_Implementation(UUR_FireModeBase* FireMode, float CurrentSpinValue) override;

    virtual void SpinDone_Implementation(UUR_FireModeBase* FireMode, bool bFullySpinnedUp) override;

    //============================================================
    // FireModeBasic interface
    //============================================================

    virtual void SimulateShot_Implementation(UUR_FireModeBasic* FireMode, FSimulatedShotInfo& OutSimulatedInfo) override;

    virtual void SimulateHitscanShot_Implementation(UUR_FireModeBasic* FireMode, FSimulatedShotInfo& OutSimulatedInfo, FHitscanVisualInfo& OutHitscanInfo) override;

    virtual void AuthorityShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo) override;

    virtual void AuthorityHitscanShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo, FHitscanVisualInfo& OutHitscanInfo) override;

    /** Play firing effects (muzzle flash, fire sound, animations...) */
    virtual void PlayFireEffects_Implementation(UUR_FireModeBasic* FireMode) override;

    /** Play hitscan effects (beam, impact...) */
    virtual void PlayHitscanEffects_Implementation(UUR_FireModeBasic* FireMode, const FHitscanVisualInfo& HitscanInfo) override;

    //============================================================
    // FireModeCharged interface
    //============================================================

    virtual void ChargeLevel_Implementation(UUR_FireModeCharged* FireMode, int32 ChargeLevel, bool bWasPaused) override;

    //============================================================
    // FireModeContinuous interface
    //============================================================

    virtual void SimulateContinuousHitCheck_Implementation(UUR_FireModeContinuous* FireMode) override;

    virtual void AuthorityStartContinuousFire_Implementation(UUR_FireModeContinuous* FireMode) override;

    virtual void AuthorityContinuousHitCheck_Implementation(UUR_FireModeContinuous* FireMode) override;

    virtual void AuthorityStopContinuousFire_Implementation(UUR_FireModeContinuous* FireMode) override;

    virtual void StartContinuousEffects_Implementation(UUR_FireModeContinuous* FireMode) override;

    virtual void UpdateContinuousEffects_Implementation(UUR_FireModeContinuous* FireMode, float DeltaTime) override;

    virtual void StopContinuousEffects_Implementation(UUR_FireModeContinuous* FireMode) override;

};
