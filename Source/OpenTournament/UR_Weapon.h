// Copyright (c) Open Tournament Project, All Rights Reserved.

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

class UUR_CrosshairData;
class AUR_Character;
class AUR_Projectile;
class AUR_Ammo;
class UShapeComponent;
class UAudioComponent;
class USkeletalMeshComponent;
class USoundBase;
class UFXSystemAsset;
class UAnimMontage;
class UPaperSprite;

/////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT(BlueprintType)
struct FWeaponAmmoDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AUR_Ammo> AmmoClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AmmoAmount;

    FWeaponAmmoDefinition()
        : AmmoClass(NULL)
        , AmmoAmount(0)
    {
    }

    FWeaponAmmoDefinition(TSubclassOf<AUR_Ammo> InClass, int32 InAmount)
        : AmmoClass(InClass)
        , AmmoAmount(InAmount)
    {
    }
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
UCLASS(NotPlaceable)
class OPENTOURNAMENT_API AUR_Weapon
    : public AActor
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    FString WeaponName;

    /**
    * Ammo classes used by this weapon.
    * Picking up weapon will stack ammo into all the classes declared here.
    * By default all firemodes use the first ammo class.
    * To assign an ammo class to a specific firemode, override function GetAmmoIndex(ModeIndex).
    * If a weapon uses multiple ammo classes, be careful about event NotifyAmmoUpdated!
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    TArray<FWeaponAmmoDefinition> AmmoDefinitions;

    /**
    * Map AmmoDefinition indices to the real AUR_Ammo objects contained in InventoryComponent, for simpler usage.
    * Replicated (OwnerOnly) to workaround painful race conditions (eg. weapon replicated before ammo).
    */
    UPROPERTY(BlueprintReadOnly, Replicated)
    TArray<AUR_Ammo*> AmmoRefs;

    /**
    * Fallback to that weapon if no user configuration is found for this weapon.
    * Used to apply plug-and-play user settings on mod-weapons by falling back to more common (core) weapons.
    *
    * User settings should include weaponbar, grouping, keybindings, and crosshairs (to start with).
    * Might add per-weapon sensitivity, fov, and firemode remapping in the future as well.
    *
    * If unspecified, the game will fallback to parent weapon by default.
    * Specify this only if you want to target a different weapon for good reason.
    */
    UPROPERTY(EditAnywhere)
    TSubclassOf<AUR_Weapon> ModFallbackToWeaponConfig;

    /**
    * Atlas texture sprite to use in UI.
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UPaperSprite* WeaponSprite;

    /**
    * Default Crosshair Data
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UUR_CrosshairData* CrosshairData;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Some getters

public:
    UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Weapon")
    int32 GetAmmoIndex(int32 ModeIndex = 0) const;

    UFUNCTION(BlueprintPure)
    AUR_Ammo* GetAmmoObject(int32 ModeIndex = 0) const;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    int32 GetCurrentAmmo(int32 ModeIndex = 0) const;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    FORCEINLINE USkeletalMeshComponent* GetMesh1P() const
    {
        return Mesh1P;
    }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    FORCEINLINE USkeletalMeshComponent* GetMesh3P() const
    {
        return Mesh3P;
    }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    USkeletalMeshComponent* GetVisibleMesh() const;

    static UClass* GetNextFallbackConfigWeapon(TSubclassOf<AUR_Weapon> ForClass);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Weapon Attachment

public:
    UPROPERTY()
    bool bIsAttached;

protected:
    /**
    * Verify if weapon is attached according to its current state.
    * in state Inactive, ensure weapon is detached and fully hidden.
    * in other states, ensure weapon is attached and visible.
    *
    * In normal situations we only need to attach on BringUp and detach after PutDown.
    * However this can also be used to support more edgy cases.
    * For example, dropping weapon will go straight to Inactive without the putdown procedure.
    */
    UFUNCTION()
    void CheckWeaponAttachment();

    /** Attach meshes to owner and enable general visibility */
    UFUNCTION()
    void AttachMeshToPawn();

    /** Detach meshes and fully hide */
    UFUNCTION()
    void DetachMeshFromPawn();

public:
    /**
    * Update visibility of 1P and 3P meshes according to current view mode, keeping the right shadows.
    */
    UFUNCTION(BlueprintCosmetic)
    void UpdateMeshVisibility();

    /**
    * Toggle general visibility of the weapon, including hidden shadows.
    */
    UFUNCTION(BlueprintCosmetic)
    void ToggleGeneralVisibility(bool bVisible);

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

    UFUNCTION(BlueprintCallable)
    virtual void RequestStartFire(uint8 FireModeIndex);

    UFUNCTION(BlueprintCallable)
    virtual void RequestStopFire(uint8 FireModeIndex);

    UFUNCTION()
    virtual void NotifyAmmoUpdated(AUR_Ammo* Ammo);

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
    * Get the corrected muzzle flash transform, taking into account panini correction in first person view.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic)
    virtual FTransform GetFireEffectStartTransform(UUR_FireModeBase* FireMode);

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
    virtual bool HasEnoughAmmoFor(UUR_FireModeBase* FireMode);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void ConsumeAmmo(int32 Amount = 1);

    UFUNCTION(BlueprintCallable)
    virtual void CheckAutoSwap();

    UFUNCTION(BlueprintPure)
    virtual bool HasAnyAmmo();

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
