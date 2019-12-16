// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include <UR_Type_DodgeDirection.h>

#include "UR_Character.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAnimationMontage;
class UUR_HealthComponent;
class UUR_ArmorComponent;
class UUR_InventoryComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FCharacterVoice
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Sounds)
    USoundBase* FootstepSound;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Sounds)
    USoundBase* JumpSound;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Sounds)
    USoundBase* DodgeSound;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Sounds)
    USoundBase* PainSound;
};

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API AUR_Character : public ACharacter
{
    GENERATED_BODY()

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////

    AUR_Character(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    bool bIsPickingUp = false;

    USkeletalMeshComponent* GetPawnMesh() const;

    /**
    * First person Camera
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* CharacterCameraComponent;

    /**
    * Character's first-person mesh (arms; seen only by self)
    */
    UPROPERTY(VisibleDefaultsOnly, Category = "Mesh")
    class USkeletalMeshComponent* MeshFirstPerson;

    /**
    * Audio content for Movement etc.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character")
    FCharacterVoice CharacterVoice;

    /**
    * Fire animation
    */
    UPROPERTY(EditDefaultsOnly, Category = "Character")
    UAnimMontage* FireAnimation;

    /**
    * Spring arm for third person camera
    */
    UPROPERTY(VisibleDefaultsOnly, Category = "Camera")
    class USpringArmComponent* ThirdPersonArm;

    /**
    * Third person camera.
    */
    UPROPERTY(VisibleDefaultsOnly, Category = "Camera")
    class UCameraComponent* ThirdPersonCamera;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // @section Input (Keypress to Weapon, Movement/Dodge)
    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Character")
    class UUR_CharacterMovementComponent* URMovementComponent;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Character")
    TSubclassOf<UUR_CharacterMovementComponent> MovementComponentClass;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Axis movement

    /**
    * Handles Input in Forward vector (Forward/Backward)
    * @param InValue normalized scalar of MovementInput to pass as InputVector to MovementComponent
    */
    virtual void MoveForward(const float InValue);

    /**
    * Handles Input in CrossOfForward horizontal vector (Right/Left)
    * @param InValue normalized scalar of MovementInput to pass as InputVector to MovementComponent
    */
    virtual void MoveRight(const float InValue);

    /**
    * Handles Input in CrossOfForward vertical vector (Up/Down)
    * @param InValue normalized scalar of MovementInput to pass as InputVector to MovementComponent
    */
    virtual void MoveUp(const float InValue);

    /**
    * Tick - For playing Footstep effects
    * @param DeltaTime tick time in seconds
    */
    void TickFootsteps(const float DeltaTime);

    /**
    * Play effects for footsteps
    * @param WalkingSpeedPercentage current movement speed 
    */
    void PlayFootstepEffects(float WalkingSpeedPercentage) const;

    /**
    * Last Foostep Timestamp.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Walk")
    float FootstepTimestamp;

    /**
    * Foostep Time Interval 
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Walk")
    float FootstepTimeIntervalBase;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Jump & Crouch

    /**
    * Speed beyond which player begins taking fall damage (note: absolute value)
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Jump")
    float FallDamageSpeedThreshold;

    /**
    * Override to handle playing Jump effects.
    */
    virtual void CheckJumpInput(float DeltaTime) override;

    /**
    * Override to clear dodging input as well
    */
    virtual void ClearJumpInput(float DeltaTime) override;

    /**
    * Override to take Falling Damage 
    */
    virtual void Landed(const FHitResult& Hit) override;

    /**
    * Take Falling Damage
    */
    UFUNCTION(BlueprintCallable, Category = Pawn)
    virtual void TakeFallingDamage(const FHitResult& Hit, float FallingSpeed);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Dodge

    /**
    * Flag used to indicate dodge directionality, indicates a pending dodge
    */
    UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Dodging")
    EDodgeDirection DodgeDirection;

    /**
    * Is this Character capable of dodging?
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Character|Dodge")
    bool IsDodgeCapable() const;

    /**
    * Is this Character permitted to dodge based on current situation
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Character|Dodge")
    bool IsDodgePermitted() const;

    /**
    * Can this character dodge based on capability and permission?
    */
    UFUNCTION(BlueprintCallable, Category = "Character|Dodge")
    bool CanDodge() const;

    /**
    * Dodge requested from PC Input. Dodge, if we CanDodge.
    */
    virtual void Dodge(FVector DodgeDir, FVector DodgeCross);

    /** 
    * Hook for sounds / effects OnDodge
    */
    UFUNCTION(BlueprintNativeEvent)
    void OnDodge(const FVector& DodgeLocation, const FVector& DodgeDir);

    /**
    * Hook for sounds / effects OnWallDodge
    */
    UFUNCTION(BlueprintNativeEvent)
    void OnWallDodge(const FVector& DodgeLocation, const FVector& DodgeDir);

    /**
    * Blueprint override for dodge handling. Return true if so
    */
    UFUNCTION(BlueprintImplementableEvent)
    bool DodgeOverride(const FVector& DodgeDir, const FVector& DodgeCross);

    /**
    * Temporary - Server update the DodgeDirection
    */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetDodgeDirection(const EDodgeDirection InDodgeDirection);


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // @section Health & Damage
    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Health Component
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Character|Health")
    UUR_HealthComponent* HealthComponent;

    /**
    * Armor Component
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Character|Armor")
    UUR_ArmorComponent* ArmorComponent;

    /**
    * Take Damage override.
    */
    virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    /**
    * Kill this player.
    * Authority only.
    */
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void Die(AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser);

    /**
    * Play dying effect (animation, ragdoll, sound, blood, gib).
    * Client only.
    */
    UFUNCTION(BlueprintCosmetic)
    virtual void PlayDeath();

    /**
    * Called on network client when replication channel is cut (ie. death).
    */
    virtual void TornOff() override
    {
        PlayDeath();
    }

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsAlive();


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // @section Inventory
    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Inventory Component
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Character|Inventory")
    UUR_InventoryComponent* InventoryComponent;

    bool isFiring = false;

    virtual void PawnStartFire(uint8 FireModeNum = 0) override;
    virtual void PawnStopFire(uint8 FireModeNum = 0);

    //Weapon select
    UFUNCTION()
    void WeaponSelect(int32 number);

    UFUNCTION(Exec, BlueprintCallable)
    void NextWeapon();

    UFUNCTION(Exec, BlueprintCallable)
    void PrevWeapon();

    UFUNCTION()
    void Fire();

    /** get weapon attach point */
    UFUNCTION()
    FName GetWeaponAttachPoint() const;

    //TODO: This should be part of weapon, not character.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    FVector MuzzleOffset;

protected:
    //pickup handlers
    void BeginPickup();
    void EndPickup();

    //these are to be improved later on
    void SelectWeapon0(); //pistol
    void SelectWeapon1(); //assault rifle
    void SelectWeapon2(); //shotgun
    void SelectWeapon3(); //rocket launcher
    void SelectWeapon4(); //grenade launcher
    void SelectWeapon5(); //sniper rifle

    void ShowInventory();

    /** socket or bone name for attaching weapon mesh */
    UPROPERTY(EditDefaultsOnly, Category = "Character|Inventory")
    FName WeaponAttachPoint;
};
