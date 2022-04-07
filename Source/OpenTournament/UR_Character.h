// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Interfaces/UR_TeamInterface.h"
#include "Components/InputComponent.h"  //struct FInputKeyBinding

#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "UR_Type_DodgeDirection.h"
#include "Enums/UR_MovementAction.h"

#include "UR_Character.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAnimationMontage;
class UGameplayTagsManager;
class UUR_AbilitySystemComponent;
class UUR_AttributeSet;
class UUR_GameplayAbility;
class UUR_InventoryComponent;
class APlayerController;
class IUR_ActivatableInterface;
class UUR_DamageType;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FCharacterVoice
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Sounds)
    USoundBase* FootstepSound;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Sounds)
    USoundBase* LandingSound;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Sounds)
    USoundBase* JumpSound;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Sounds)
    USoundBase* DodgeSound;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Sounds)
    USoundBase* PainSound;

    FCharacterVoice() : FootstepSound(NULL), LandingSound(NULL), JumpSound(NULL), DodgeSound(NULL), PainSound(NULL) {}
};

/**
* Replicatable damage event.
* Builtin damage events are not replicatable due to struct inheritance & missing reflection.
* This shall be used for replicating damage numbers, hitsounds, physics impulses, incoming damage on HUD...
*
* NOTE: For something like shotgun/flak, we'll need to group up events before replicating.
*/
USTRUCT(BlueprintType)
struct FReplicatedDamageEvent
{
    GENERATED_BODY()

    /**
    * Matches builtin DamageEvent.ClassID
    * 1 = PointDamage
    * 2 = RadialDamage
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Type;

    /**
    * Full damage value (includes over-damage).
    * In case of RadialDamage, this is already scaled by distance.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Damage;

    /**
    * Damage applied to health.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HealthDamage;

    /**
    * Damage applied to armor.
    * NOTE: some compression could be applied here, but reflection/BP doesn't support int16 so it's annoying
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ArmorDamage;

    /**
    * Damage location.
    * In case of PointDamage, matches HitLocation.
    * In case of RadialDamage, matches Origin.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    /**
    * Knockback.
    * In case of PointDamage, equals to KnockbackPower * ShotDirection.
    * In case of RadialDamage, equals to (unscaled KnockbackPower, Radius, 0).
    *
    * NOTE: For PointDamage we should make sure to always send a non-zero vector even if we have zero knockback,
    * so clients/widgets can use it to get ShotDirection.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Knockback;

    /**
    * Damage type class.
    * Already casted as UR_DamageType because the core class is useless.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    const UUR_DamageType* DamType;

    /**
    * Damage instigator
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APawn* DamageInstigator;

    FReplicatedDamageEvent() : Type(0), Damage(0), HealthDamage(0), ArmorDamage(0), Location(0,0,0), Knockback(0,0,0), DamType(NULL), DamageInstigator(NULL) {}

    virtual bool IsOfType(int32 InID) const { return Type == InID; };
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Damage event dispatcher
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterDamageEventSignature, AUR_Character*, Character, const FReplicatedDamageEvent, RepDamageEvent);

/**
* Death event dispatcher.
* NOTE: Controllers not available on client so this is not ideal for Killer.
* Pawn and PlayerState are possible alternatives with both pros and cons. Need to discuss this.
* Similar debate about the Damage event.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterDeathSignature, AUR_Character*, Character, AController*, Killer);

/**
* Pickup event dispatcher.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPickupEventSignature, AUR_Pickup*, Pickup);


/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API AUR_Character : public ACharacter
    , public IAbilitySystemInterface
    , public IGameplayTagAssetInterface
    , public IUR_TeamInterface
{
    GENERATED_BODY()

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////

    AUR_Character(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    //deprecated
    bool bIsPickingUp = false;

    /**
    * First person Camera
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* CharacterCameraComponent;

    /**
    * Character's first-person mesh (arms; seen only by self)
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
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

    UFUNCTION(BlueprintCosmetic, BlueprintNativeEvent)
    void SetupMaterials();

    UFUNCTION(BlueprintCosmetic, BlueprintNativeEvent, BlueprintCallable)
    void UpdateTeamColor();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /** DEPRECATED. Use GetMesh1P() or GetMesh3P() instead */
    USkeletalMeshComponent* GetPawnMesh() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
    FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return MeshFirstPerson; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
    FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return GetMesh(); }

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual UInputComponent* CreatePlayerInputComponent() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

    virtual void BecomeViewTarget(APlayerController* PC) override;
    virtual void EndViewTarget(APlayerController* PC) override;

    virtual void OnRep_PlayerState() override;

    // Override to update Physics Movement GameplayTags
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

    virtual void UnPossessed() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Camera Management

    /**
    * Updated by CalcCamera. Controlled by PickCamera/IsThirdPersonCamera.
    * Client only.
    */
    UPROPERTY(BlueprintReadOnly)
    bool bViewingThirdPerson;

    /**
    * Return the camera component to use when viewing this pawn.
    * Called by CalcCamera which is tick-based.
    * Override this to implement new cameras.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintCosmetic, BlueprintPure)
    UCameraComponent* PickCamera();

    /**
    * Return true if this camera component is a third-person view.
    * Called by CalcCamera which is tick-based.
    * This controls triggering of CameraViewChanged event.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure)
    bool IsThirdPersonCamera(UCameraComponent* Camera);

    /**
    * Update 1p/3p meshes visibility according to bViewingThirdPerson.
    * Client only.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic)
    void CameraViewChanged();

    /**
    * Register a new zoom interface, that will be automatically activated / deactivated,
    * according to this character current view mode (1P, 3P, spectated, etc.)
    */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic)
    virtual void RegisterZoomInterface(TScriptInterface<IUR_ActivatableInterface> NewZoomInterface);

    UPROPERTY()
    TScriptInterface<IUR_ActivatableInterface> CurrentZoomInterface;

    /** Temporary - probably needs to sit in PlayerController */
    UFUNCTION(Exec)
    virtual void BehindView(int32 Switch = -1);

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
    * Last Footstep Timestamp.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Walk")
    float FootstepTimestamp;

    /**
    * Footstep Time Interval 
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Walk")
    float FootstepTimeIntervalBase;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Weapons bindings

    TArray<FInputKeyBinding> WeaponBindings;

    // Called on possess, and when controlling player changes binds.
    UFUNCTION(BlueprintCallable)
    virtual void SetupWeaponBindings();

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // General - EyeHeight Adjustment

    virtual void RecalculateBaseEyeHeight() override;

    void TickEyePosition(const float DeltaTime);

    /**
    * Called on Landing. Calculate the View Offset.
    */
    void LandedViewOffset();

    UPROPERTY()
    FVector DefaultCameraPosition;

    UPROPERTY()
    float EyeOffsetLandingBobMinimum;

    UPROPERTY()
    float EyeOffsetLandingBobMaximum;

    UPROPERTY()
    FVector EyeOffset;

    UPROPERTY()
    FVector TargetEyeOffset;

    UPROPERTY()
    FVector EyeOffsetToTargetInterpolationRate;

    UPROPERTY()
    FVector TargetEyeOffsetToNeutralInterpolationRate;

    UPROPERTY()
    FVector CrouchEyeOffset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Eyeheight")
    float CrouchEyeOffsetZ;

    UPROPERTY()
    float OldLocationZ;


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Jump

    /**
    * Points of Damage inflicted by Speed/Velocity exceeding FallDamageSpeedThreshold (note: absolute value)
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Jump")
    float FallDamageScalar;

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
    * Override to take Falling Damage, calculate LandedViewOffset, etc.
    */
    virtual void Landed(const FHitResult& Hit) override;

    /**
    * Cosmetic effects related to Landing
    */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Character|Jump")
    void PlayLandedEffects(const FHitResult& Hit);

    /**
    * Take Falling Damage
    */
    UFUNCTION(BlueprintCallable, Category = Pawn)
    virtual void TakeFallingDamage(const FHitResult& Hit, float FallingSpeed);

    UPROPERTY()
    UParticleSystem* LandedParticleSystemClass;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Crouch

    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    /**
    * Effects on Starting a Crouch
    */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Character|Crouch")
    virtual void OnStartCrouchEffects();

    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    /**
    * Effects on Ending a Crouch
    */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Character|Crouch")
    virtual void OnEndCrouchEffects();

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Character|Crouch")
    float PriorCrouchTime;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character|Crouch")
    USoundBase* CrouchTransitionSound;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Character|Crouch")
    float CrouchTransitionSpeed;

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
    * Perform a Dodge. Testing purposes only.
    */
    UFUNCTION(BlueprintCallable, Category = "Character|Dodge")
    void Dodge(const EDodgeDirection InDodgeDirection);

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
    * Temporary - Set & replicate DodgeDirection.
    * TODO: implement properly in movement core.
    * See https://docs.unrealengine.com/en-US/Gameplay/Networking/CharacterMovementComponent/index.html
    */
    UFUNCTION()
    virtual void SetDodgeDirection(const EDodgeDirection InDodgeDirection)
    {
        DodgeDirection = InDodgeDirection;
        ServerSetDodgeDirection(DodgeDirection);
    }
    UFUNCTION(Server, Reliable)
    void ServerSetDodgeDirection(const EDodgeDirection InDodgeDirection);
    virtual void ServerSetDodgeDirection_Implementation(const EDodgeDirection InDodgeDirection)
    {
        DodgeDirection = InDodgeDirection;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // @section Gameplay Ability System
    /////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Gameplay Tags

    /**
    * GameplayTagsManager
    */
    UPROPERTY(BlueprintReadOnly, Category = "GameplayTags")
    UGameplayTagsManager* GameplayTagsManager;

    /**
    * Initialize the GameplayTagsManager reference
    */
    void InitializeGameplayTagsManager();
    
    /**
    * Character's GameplayTags
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer GameplayTags;

    /**
    * Get Character's GameplayTags
    */
    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; }

    /**
    * Update Movement GameplayTags pertaining to Physics
    */
    void UpdateMovementPhysicsGameplayTags(const EMovementMode PreviousMovementMode);

    /**
    * Update Character GameplayTags
    */
    UFUNCTION(BlueprintCallable, Category = "GameplayTags")
    void UpdateGameplayTags(const FGameplayTagContainer& TagsToRemove, const FGameplayTagContainer& TagsToAdd);

    /**
    * MovementAction enums mapped to GameplayTags
    */
    UPROPERTY(BlueprintReadOnly, Category = "GameplayTags")
    TMap<EMovementAction, FGameplayTag> MovementActionGameplayTags;

    /**
    * MovementMode enums mapped to GameplayTags
    */
    UPROPERTY(BlueprintReadOnly, Category = "GameplayTags")
    TMap<TEnumAsByte<EMovementMode>, FGameplayTag> MovementModeGameplayTags;

    /**
    * Initialize our GameplayTag data structures
    */
    void InitializeGameplayTags();

    /**
    * Initialize the MovementAction GameplayTags data structure
    */
    void InitializeMovementActionGameplayTags();

    /**
    * Initialize the MovementMode GameplayTags data structure
    */
    void InitializeMovementModeGameplayTags();

    /**
    * Get the GameplayTag associated with given MovementAction
    */
    FGameplayTag GetMovementActionGameplayTag(const EMovementAction InMovementAction);
    
    /**
    * Get the GameplayTag associated with given EMovementMode
    */
    FGameplayTag GetMovementModeGameplayTag(const EMovementMode InMovementMode);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // GAS

    // Implement IAbilitySystemInterface
    UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    /** Grant a GameplayAbility */
    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Character")
    void Server_GiveAbility(TSubclassOf<UUR_GameplayAbility> InAbility, const int32 InAbilityLevel = 1);

    /** Remove a GameplayAbility */
    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Character")
    void Server_RemoveAbility(TSubclassOf<UUR_GameplayAbility> InAbilityClass) const;

    /** Get the Level of a GameplayAbility */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
    int32 GetAbilityLevel(TSubclassOf<UUR_GameplayAbility> InAbilityClass) const;

    /** Get the Level of a GameplayAbility */
    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Character")
    void Server_SetAbilityLevel(TSubclassOf<UUR_GameplayAbility> InAbilityClass, const int32 InAbilityLevel = 1);

    /*
    * Ability System Component
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Character|Abilities")
    UUR_AbilitySystemComponent* AbilitySystemComponent;

    /**
    * Attribute Set
    */
    UPROPERTY()
    UUR_AttributeSet* AttributeSet;

    /** Abilities to grant to this character on creation. These will be activated by tag or event and are not bound to specific inputs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Abilities")
    TArray<TSubclassOf<UUR_GameplayAbility>> GameplayAbilities;

    /** Passive gameplay effects applied on creation */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Abilities")
    TArray<TSubclassOf<UGameplayEffect>> PassiveGameplayEffects;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // @section Health & Damage
    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Take Damage override.
    */
    virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastDamageEvent(const FReplicatedDamageEvent RepDamageEvent);

    UPROPERTY(BlueprintAssignable, Category = "Character")
    FCharacterDamageEventSignature OnDamageReceived;

    UPROPERTY(BlueprintAssignable, Category = "Character")
    FCharacterDamageEventSignature OnDamageDealt;

    /**
    * Kill this player.
    * Authority only.
    */
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void Die(AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser, const FReplicatedDamageEvent& RepDamageEvent);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastDied(AController* Killer, const FReplicatedDamageEvent RepDamageEvent);
    virtual void MulticastDied_Implementation(AController* Killer, const FReplicatedDamageEvent RepDamageEvent)
    {
        PlayDeath(Killer, RepDamageEvent);
    }

    /**
    * Play dying state on client (animation, ragdoll, sound, blood, gib, camera).
    */
    UFUNCTION(BlueprintCosmetic, BlueprintNativeEvent)
    void PlayDeath(AController* Killer, const FReplicatedDamageEvent& RepDamageEvent);

    /**
    * Set to true after PlayDeath() is received on clients.
    * Used in TornOff() to adjust life span accordingly.
    */
    UPROPERTY(BlueprintReadWrite)
    bool bPlayingDeath;

    /**
    * Event Called on Character Death.
    * Server & Client.
    */
    UPROPERTY(BlueprintAssignable, Category = "Character")
    FCharacterDeathSignature OnDeath;

    /**
    * Called on network clients when replication channel is cut (ie. death).
    */
    virtual void TornOff() override;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsAlive() const;

    UFUNCTION(Exec)
    virtual void Suicide()
    {
        ServerSuicide();
    }

    UFUNCTION(Server, Reliable)
    void ServerSuicide();

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // @section Inventory
    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Inventory Component
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Character|Inventory")
    UUR_InventoryComponent* InventoryComponent;

    //deprecated
    bool bIsFiring = false;

    /**
    * Track firing inputs in a stack to know if/which one should be active now.
    */
    UPROPERTY()
    TArray<uint8> DesiredFireModeNum;

    virtual void PawnStartFire(uint8 FireModeNum = 0) override;
    virtual void PawnStopFire(uint8 FireModeNum = 0);

    /** get weapon attach point */
    UFUNCTION()
    FName GetWeaponAttachPoint() const;

    //TODO: This should be part of weapon, not character.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    FVector MuzzleOffset;

    /** socket or bone name for attaching weapon mesh */
    UPROPERTY(EditDefaultsOnly, Category = "Character|Inventory")
    FName WeaponAttachPoint;

    UFUNCTION(Exec, BlueprintCallable)
    virtual void SelectWeapon(int32 Index);

    UFUNCTION(Exec, BlueprintCallable)
    virtual void NextWeapon();

    UFUNCTION(Exec, BlueprintCallable)
    virtual void PrevWeapon();

    /**
    * Pickup event.
    */
    UPROPERTY(BlueprintAssignable)
    FPickupEventSignature PickupEvent;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    //~ Begin TeamInterface
    virtual int32 GetTeamIndex_Implementation() override;
    virtual void SetTeamIndex_Implementation(int32 NewTeamIndex) override;
    //~ End TeamInterface
   
};
