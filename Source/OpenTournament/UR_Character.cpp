// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Character.h"

#include "Net/UnrealNetwork.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/GameState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

#include "OpenTournament.h"
#include "UR_InventoryComponent.h"
#include "UR_CharacterMovementComponent.h"
#include "UR_AttributeSet.h"
#include "UR_AbilitySystemComponent.h"
#include "UR_GameplayAbility.h"
#include "UR_PlayerController.h"
#include "UR_GameMode.h"
#include "UR_Weapon.h"
#include "UR_Projectile.h"
#include "Interfaces/UR_ActivatableInterface.h"
#include "UR_PlayerState.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Character::AUR_Character(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer.SetDefaultSubobjectClass<UUR_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName)),
    FootstepTimestamp(0.f),
    FootstepTimeIntervalBase(0.300f),
    FallDamageScalar(0.15f),
    FallDamageSpeedThreshold(2675.f),
    CrouchTransitionSpeed(12.f)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    // Unreal & UT99 Values (Scaling Factor 2.5)
    GetCapsuleComponent()->InitCapsuleSize(42.5f, 97.5f);

    URMovementComponent = Cast<UUR_CharacterMovementComponent>(GetCharacterMovement());
    URMovementComponent->bUseFlatBaseForFloorChecks = true;

    InventoryComponent = Cast<UUR_InventoryComponent>(CreateDefaultSubobject<UUR_InventoryComponent>(TEXT("InventoryComponent")));

    // Create a CameraComponent	
    CharacterCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    CharacterCameraComponent->SetupAttachment(GetCapsuleComponent());
    CharacterCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, BaseEyeHeight)); // Position the camera
    CharacterCameraComponent->bUsePawnControlRotation = true;

    // FVector(-39.56f, 1.75f, BaseEyeHeight)
    DefaultCameraPosition = FVector(-0.f, 0.f, BaseEyeHeight);
    BaseEyeHeight = 64.f;
    CrouchedEyeHeight = 64.f;
    EyeOffset = FVector(0.f, 0.f, 0.f);
    TargetEyeOffset = EyeOffset;
    EyeOffsetLandingBobMaximum = BaseEyeHeight + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    EyeOffsetLandingBobMinimum = EyeOffsetLandingBobMaximum / 10.f;
    EyeOffsetToTargetInterpolationRate = FVector(18.f, 10.f, 10.f);
    TargetEyeOffsetToNeutralInterpolationRate = FVector(5.f, 5.f, 5.f);

    // Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
    MeshFirstPerson = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFirstPerson"));
    MeshFirstPerson->SetOnlyOwnerSee(true);
    MeshFirstPerson->SetupAttachment(CharacterCameraComponent);
    MeshFirstPerson->bCastDynamicShadow = false;
    MeshFirstPerson->CastShadow = false;
    MeshFirstPerson->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
    MeshFirstPerson->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

    WeaponAttachPoint = FName(TEXT("GripPoint"));

    // Mesh third person
    GetMesh()->bOwnerNoSee = true;

    // Third person camera
    ThirdPersonArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonArm"));
    ThirdPersonArm->SetupAttachment(GetCapsuleComponent());
    ThirdPersonArm->TargetArmLength = 400.f;
    ThirdPersonArm->TargetOffset.Set(0.f, 0.f, 100.f);
    ThirdPersonArm->bUsePawnControlRotation = true;

    ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
    ThirdPersonCamera->SetupAttachment(ThirdPersonArm);
    
    // Create the attribute set, this replicates by default
    AttributeSet = CreateDefaultSubobject<UUR_AttributeSet>(TEXT("AttributeSet"));

    // Create the ASC
    AbilitySystemComponent = CreateDefaultSubobject<UUR_AbilitySystemComponent>("AbilitySystemComponent");
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AUR_Character, DodgeDirection);
    DOREPLIFETIME(AUR_Character, InventoryComponent);
    DOREPLIFETIME(AUR_Character, AbilitySystemComponent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::BeginPlay()
{
    Super::BeginPlay();

    AttributeSet->SetHealth(100.f);
    AttributeSet->SetHealthMax(100.f);
    AttributeSet->SetArmor(100.f);
    AttributeSet->SetArmorMax(100.f);
    AttributeSet->SetShieldMax(100.f);

    SetupMaterials();
}

void AUR_Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickFootsteps(DeltaTime);
    TickEyePosition(DeltaTime);
}

UAbilitySystemComponent* AUR_Character::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AUR_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &AUR_Character::BeginPickup);
    PlayerInputComponent->BindAction("Pickup", IE_Released, this, &AUR_Character::EndPickup);

    PlayerInputComponent->BindAction("ShowInventory", IE_Pressed, this, &AUR_Character::ShowInventory);

    PlayerInputComponent->BindAction("ARifle", IE_Pressed, this, &AUR_Character::SelectWeapon1);
    PlayerInputComponent->BindAction("Shotgun", IE_Pressed, this, &AUR_Character::SelectWeapon2);
    PlayerInputComponent->BindAction("RLauncher", IE_Pressed, this, &AUR_Character::SelectWeapon3);
    PlayerInputComponent->BindAction("GLauncher", IE_Pressed, this, &AUR_Character::SelectWeapon4);
    PlayerInputComponent->BindAction("SRifle", IE_Pressed, this, &AUR_Character::SelectWeapon5);
    PlayerInputComponent->BindAction("Pistol", IE_Pressed, this, &AUR_Character::SelectWeapon0);

    // Select Weapon Bind
    // Throw Weapon

    // Voice
    // Ping
    // Emote

    PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AUR_Character::NextWeapon);
    PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AUR_Character::PrevWeapon);
}

void AUR_Character::SetupMaterials_Implementation()
{
    /*
    for (UPrimitiveComponent* Component : { GetMesh3P(), GetMesh1P() })
    {
        for (int32 i = 0; i < Component->GetNumMaterials(); i++)
        {
            Component->CreateDynamicMaterialInstance(i);
        }
    }
    */
    //NOTE: this is not required because MeshComponent::SetParameterValueOnMaterials already generates MIDs as needed.
    // Might want to remove this function altogether

    UpdateTeamColor();
}

void AUR_Character::UpdateTeamColor_Implementation()
{
    if (GetMesh3P())
    {
        GetMesh3P()->SetScalarParameterValueOnMaterials(FName(TEXT("TeamIndex")), IUR_TeamInterface::Execute_GetTeamIndex(this));
        GetMesh3P()->SetScalarParameterValueOnMaterials(FName(TEXT("IsSelf")), Cast<APlayerController>(GetController()) ? 1.f : 0.f);
    }
    if (GetMesh1P())
    {
        GetMesh1P()->SetScalarParameterValueOnMaterials(FName(TEXT("TeamIndex")), IUR_TeamInterface::Execute_GetTeamIndex(this));
        // FirstPerson mesh is always considered "self" and should use AllyColor
        GetMesh1P()->SetScalarParameterValueOnMaterials(FName(TEXT("IsSelf")), 1.f);
    }
}

void AUR_Character::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (GetNetMode() != NM_DedicatedServer)
    {
        UpdateTeamColor();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Camera

void AUR_Character::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
    UCameraComponent* Camera = PickCamera();
    if (Camera)
    {
        const bool bThirdPerson = IsThirdPersonCamera(Camera);
        if (bThirdPerson != bViewingThirdPerson)
        {
            bViewingThirdPerson = bThirdPerson;
            CameraViewChanged();
        }
        Camera->GetCameraView(DeltaTime, OutResult);
    }
    else
    {
        // Fallback to Super
        Super::CalcCamera(DeltaTime, OutResult);
    }
}

UCameraComponent* AUR_Character::PickCamera_Implementation()
{
    // End game = always 3p
    AGameState* GS = GetWorld()->GetGameState<AGameState>();
    if (GS && GS->HasMatchEnded() && ThirdPersonCamera)
    {
        return ThirdPersonCamera;
    }

    // Alive = 1p
    //TODO: specs should be able to switch at will.
    // That would be a variable in PlayerController.
    // Hopefully this is client-only, so we can get local PC to check this.
    if (IsAlive() && CharacterCameraComponent && CharacterCameraComponent->IsActive())
    {
        return CharacterCameraComponent;
    }

    // Fallback to 3p
    if (ThirdPersonCamera && ThirdPersonCamera->IsActive())
    {
        return ThirdPersonCamera;
    }

    return nullptr;
}

bool AUR_Character::IsThirdPersonCamera_Implementation(UCameraComponent* Camera)
{
    return Camera && Camera == ThirdPersonCamera;
}

void AUR_Character::CameraViewChanged_Implementation()
{
    GetMesh()->SetOwnerNoSee(!bViewingThirdPerson);
    MeshFirstPerson->SetVisibility(!bViewingThirdPerson, true);

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->UpdateMeshVisibility();
    }

    // If a zoom is active, toggle it according to 1P/3P
    if (CurrentZoomInterface)
    {
        IUR_ActivatableInterface::Execute_AIF_SetActive(CurrentZoomInterface.GetObject(), !bViewingThirdPerson, false);
    }
}

void AUR_Character::BecomeViewTarget(APlayerController* PC)
{
    Super::BecomeViewTarget(PC);

    if (PC && PC->IsLocalController())
    {
        // Update all the things (character 1p/3p mesh, weapon 1p/3p mesh, zooming state)
        bViewingThirdPerson = IsThirdPersonCamera(PickCamera());
        CameraViewChanged();
    }
}

void AUR_Character::EndViewTarget(APlayerController* PC)
{
    // If a zoom is active, deactivate it
    if (CurrentZoomInterface)
    {
        IUR_ActivatableInterface::Execute_AIF_Deactivate(CurrentZoomInterface.GetObject(), false);
    }

    Super::EndViewTarget(PC);
}

void AUR_Character::RegisterZoomInterface(TScriptInterface<IUR_ActivatableInterface> NewZoomInterface)
{
    if (CurrentZoomInterface == NewZoomInterface)
    {
        return;
    }

    // Deactivate previous
    if (CurrentZoomInterface)
    {
        IUR_ActivatableInterface::Execute_AIF_Deactivate(CurrentZoomInterface.GetObject(), false);
    }

    // Set new
    CurrentZoomInterface = NewZoomInterface;

    // Activate if currently viewed in 1P
    if (CurrentZoomInterface && UUR_FunctionLibrary::IsViewingFirstPerson(this))
    {
        IUR_ActivatableInterface::Execute_AIF_Activate(CurrentZoomInterface.GetObject(), false);
    }
}

void AUR_Character::BehindView(int32 Switch)
{
    CharacterCameraComponent->SetActive((Switch == -1) ? (!CharacterCameraComponent->IsActive()) : !Switch);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Movement

void AUR_Character::MoveForward(const float InValue)
{
    if (InValue != 0.0f)
    {
        const FRotator ControlRotation = GetControlRotation();
        FRotator ForwardRotation = ControlRotation;

        // If we aren't flying / swimming, our ForwardRotation should trim out pitch / roll values
        if (URMovementComponent != nullptr && !URMovementComponent->Is3DMovementMode())
        {
            ForwardRotation = FRotator(0, ControlRotation.Yaw, 0);
        }

        // Move in the direction calculated by our ForwardRotation
        AddMovementInput(FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::X), InValue);
    }
}

void AUR_Character::MoveRight(const float InValue)
{
    if (InValue != 0.0f)
    {
        // Strafing needs no Pitch, so just take our Control Rotation's Yaw
        const FRotator ControlRotation = GetControlRotation();
        const FRotator ForwardRotation(0, ControlRotation.Yaw, 0);

        // Add movement in the perpendicular Y axis
        AddMovementInput(FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::Y), InValue);
    }
}

void AUR_Character::MoveUp(const float InValue)
{
    if (InValue != 0.0f)
    {
        // add movement in up direction
        AddMovementInput(FVector(0.f, 0.f, 1.f), InValue);
    }
}

void AUR_Character::TickFootsteps(const float DeltaTime)
{
    const float VelocityMagnitude = GetCharacterMovement()->Velocity.Size();
    const float WalkingSpeedPercentage = GetCharacterMovement()->MaxWalkSpeed / VelocityMagnitude;
    const float TimeSeconds = GetWorld()->TimeSeconds;
    const float TimeSinceLastFootstep = TimeSeconds - FootstepTimestamp;

    if (GetCharacterMovement()->MovementMode == MOVE_Walking)
    {
        if (VelocityMagnitude > 0.0f && TimeSinceLastFootstep > FootstepTimeIntervalBase * WalkingSpeedPercentage)
        {
            PlayFootstepEffects(WalkingSpeedPercentage);
            FootstepTimestamp = TimeSeconds;
        }
    }
}

void AUR_Character::PlayFootstepEffects(const float WalkingSpeedPercentage) const
{
    const float FootstepVolume = FMath::Clamp<float>(0.2f, 1.f, WalkingSpeedPercentage);
    UGameplayStatics::PlaySound2D(GetWorld(), CharacterVoice.FootstepSound, FootstepVolume, 1.f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::RecalculateBaseEyeHeight()
{
    //NOTE: Careful, this is called every Tick on non-owning clients, not just during crouch events
    if (GetLocalRole() == ROLE_SimulatedProxy)
    {
        Super::RecalculateBaseEyeHeight();
        return;
    }

    const float DefaultHalfHeight{ GetDefaultHalfHeight() };
    const float AbsoluteDifference = DefaultHalfHeight - ((GetCharacterMovement()->CrouchedHalfHeight / DefaultHalfHeight) * DefaultHalfHeight);

    if (GetMovementComponent()->IsMovingOnGround())
    {
        CrouchEyeOffsetZ += bIsCrouched ? AbsoluteDifference : -1.f * AbsoluteDifference;
    }
}

void AUR_Character::TickEyePosition(const float DeltaTime)
{
    // Check if Player JustTeleported. If so, ensure the EyeOffset updates immediately
    if (GetCharacterMovement()->bJustTeleported && (FMath::Abs(OldLocationZ - GetActorLocation().Z) > GetCharacterMovement()->MaxStepHeight))
    {
        EyeOffset.Z = 0.f;
    }
    else
    {
        EyeOffset.Z += (OldLocationZ - GetActorLocation().Z);
    }

    // Crouch Stuff
    const float StandingBonus{ bIsCrouched ? CrouchTransitionSpeed : 0.f };
    CrouchEyeOffsetZ =  FMath::FInterpTo(CrouchEyeOffsetZ, BaseEyeHeight, DeltaTime, CrouchTransitionSpeed + StandingBonus);

    EyeOffset.X = FMath::FInterpTo(EyeOffset.X, TargetEyeOffset.X, DeltaTime, EyeOffsetToTargetInterpolationRate.X);
    EyeOffset.Y = FMath::FInterpTo(EyeOffset.Y, TargetEyeOffset.Y, DeltaTime, EyeOffsetToTargetInterpolationRate.Y);
    EyeOffset.Z = FMath::FInterpTo(EyeOffset.Z, TargetEyeOffset.Z, DeltaTime, EyeOffsetToTargetInterpolationRate.Z);
    TargetEyeOffset.X = FMath::FInterpTo(TargetEyeOffset.X, 0.f, DeltaTime, TargetEyeOffsetToNeutralInterpolationRate.X);
    TargetEyeOffset.Y = FMath::FInterpTo(TargetEyeOffset.Y, 0.f, DeltaTime, TargetEyeOffsetToNeutralInterpolationRate.Y);
    TargetEyeOffset.Z = FMath::FInterpTo(TargetEyeOffset.Z, 0.f, DeltaTime, TargetEyeOffsetToNeutralInterpolationRate.Z);

    // Force Eye Position when stepping on Movers
    if (auto MovementBase = GetMovementBase())
    {
        if (MovementBase->Mobility == EComponentMobility::Movable)
        {
            EyeOffset.Z = 0.f;
            TargetEyeOffset.Z = 0.f;
        }
    }

    //GAME_LOG(Game, Log, "Ticking EyeOffset: %f, %f, %f)", EyeOffset.X, EyeOffset.Y, EyeOffset.Z);
    CharacterCameraComponent->SetRelativeLocation(FVector(-0.f, 0.f, CrouchEyeOffsetZ) + EyeOffset, false);

    // Update OldLocationZ. Order of operations is important here, this must follow our EyeOffset updates
    OldLocationZ = GetActorLocation().Z;
}

void AUR_Character::LandedViewOffset()
{
    const float FallingVelocityZ = FMath::Abs(URMovementComponent->Velocity.Z);
    if (FallingVelocityZ > (URMovementComponent->JumpZVelocity / 2.f))
    {
        const float LandingBobAlpha = FMath::Clamp<float>(FallingVelocityZ / FallDamageSpeedThreshold, 0.f, 1.f);

        TargetEyeOffset.Z = -1.f * FMath::Lerp(EyeOffsetLandingBobMinimum, EyeOffsetLandingBobMaximum, LandingBobAlpha);
        GAME_LOG(Game, Log, "Landing Bob EyeOffset.Z: %f", TargetEyeOffset.Z);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::CheckJumpInput(float DeltaTime)
{
    if (URMovementComponent)
    {
        URMovementComponent->CheckJumpInput(DeltaTime);

        // If we reached this point and the flag indicating a jump is still set, then perform the jump effects
        if (bPressedJump)
        {
            if (JumpCurrentCount == 0)
            {
                JumpCurrentCount++;

                // Play effects for Jump
                OnJumped();
            }
        }
    }
}

void AUR_Character::ClearJumpInput(float DeltaTime)
{
    Super::ClearJumpInput(DeltaTime);
    if (URMovementComponent)
    {
        URMovementComponent->ClearDodgeInput();
    }
}

void AUR_Character::Landed(const FHitResult& Hit)
{
    PlayLandedEffects(Hit);

    TakeFallingDamage(Hit, GetCharacterMovement()->Velocity.Z);

    GAME_LOG(Game, Log, "Updating OldLocationZ: %f (Old), %f (New)", OldLocationZ, GetActorLocation().Z);
    OldLocationZ = GetActorLocation().Z;

    FootstepTimestamp = GetWorld()->TimeSeconds;

    // Landing View Bob
    LandedViewOffset();

    Super::Landed(Hit);
}

void AUR_Character::PlayLandedEffects(const FHitResult& Hit)
{
    if (LandedParticleSystemClass)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LandedParticleSystemClass, Hit.Location, Hit.Normal.ToOrientationRotator());
    }

    const float LandingVolume = FMath::Clamp<float>(0.2f, 1.f, FMath::Abs(GetVelocity().Z) / FallDamageSpeedThreshold);
    UGameplayStatics::PlaySound2D(GetWorld(), CharacterVoice.FootstepSound, LandingVolume, 1.f);
}

void AUR_Character::TakeFallingDamage(const FHitResult& Hit, float FallingSpeed)
{
    if (AbilitySystemComponent == nullptr || URMovementComponent == nullptr)
    {
        return;
    }

    if (FallingSpeed * -1.f > FallDamageSpeedThreshold)
    {
        const float FallingDamage =  (-1.f * FallDamageScalar) * (FallDamageSpeedThreshold + FallingSpeed);
        GAME_LOG(Game, Log, "Character received Fall Damage (%f)!", FallingDamage);

        if (FallingDamage >= 1.0f)
        {
            FDamageEvent DamageEvent; // @! TODO Real DamageTypes
            TakeDamage(FallingDamage, DamageEvent, Controller, this);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    OnStartCrouchEffects();

    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    CrouchEyeOffset.Z += 64.f - BaseEyeHeight + HalfHeightAdjust; // @! TODO 64.f = StartingBaseEyeHeight

    OldLocationZ = GetActorLocation().Z;

    // Anims, sounds
}

void AUR_Character::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    OnEndCrouchEffects();

    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    CrouchEyeOffset.Z += 64.f - BaseEyeHeight + HalfHeightAdjust; // @! TODO 64.f = StartingBaseEyeHeight

    OldLocationZ = GetActorLocation().Z;

    // Anims, sounds
}

void AUR_Character::OnStartCrouchEffects()
{
    UGameplayStatics::PlaySound2D(GetWorld(), CrouchTransitionSound, 1.0f, 1.f);
}

void AUR_Character::OnEndCrouchEffects()
{
    UGameplayStatics::PlaySound2D(GetWorld(), CrouchTransitionSound, 1.0f, 1.f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_Character::IsDodgeCapable_Implementation() const
{
    return (URMovementComponent != nullptr);
}

bool AUR_Character::IsDodgePermitted_Implementation() const
{
    bool IsPermitted = false;

    if (URMovementComponent != nullptr)
    {
        IsPermitted = !URMovementComponent->IsFlying();
        IsPermitted = IsPermitted && !URMovementComponent->bIsDodging;
        IsPermitted = IsPermitted && GetWorld()->TimeSeconds > URMovementComponent->DodgeResetTime;
    }

    return IsPermitted;
}

bool AUR_Character::CanDodge() const
{
    return IsDodgeCapable() && IsDodgePermitted();
}

void AUR_Character::Dodge(FVector DodgeDir, FVector DodgeCross)
{
    if (CanDodge())
    {
        if (DodgeOverride(DodgeDir, DodgeCross))
        {
            return;
        }

        URMovementComponent->PerformDodge(DodgeDir, DodgeCross);
        return;
    }

    if (URMovementComponent)
    {
        URMovementComponent->ClearDodgeInput();
    }
}

void AUR_Character::OnDodge_Implementation(const FVector& DodgeLocation, const FVector& DodgeDir)
{
    // @! TODO Effects
    if (CharacterVoice.DodgeSound != nullptr)
    {
        if (GetLocalRole() == ROLE_Authority)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CharacterVoice.DodgeSound, GetActorLocation(), GetActorRotation());
        }
    }
}

void AUR_Character::OnWallDodge_Implementation(const FVector& DodgeLocation, const FVector& DodgeDir)
{
    // @! TODO Effects
    if (CharacterVoice.DodgeSound != nullptr)
    {
        if (GetLocalRole() == ROLE_Authority)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CharacterVoice.DodgeSound, GetActorLocation(), GetActorRotation());

            // Modify player view for Dodge
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

float AUR_Character::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float OriginalDamage = Damage;

    GAME_LOG(Game, Log, "Damage Incoming (%f)", OriginalDamage);

    //NOTE:
    // Super() takes care of :
    // - low level checks (returns 0.0 if not pass)
    // - calculate proper splash damage values (original Damage is the full damage regardless of radius)
    // - impart physics impulses to physics components
    //
    //NOTE: Low level checks include an authority check, meaning Super() only imparts components physics on server side,
    // which makes no sense. We might have to rework this.
    //
    //NOTE: Either way, impulses are only for physics-enabled components.
    // There is nothing builtin for character knockback, so we will have to do it ourselves.
    // We can access the projectile or the weapon (hitscan) trough DamageCauser, and fetch knockback properties there.
    // Calculate falloff manually, then apply knockback manually using CharacterMovement->AddImpulse.

    // Calculate damage with falloff if this is splash
    Damage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    // Floor
    Damage = FMath::FloorToFloat(Damage);

    if (Damage == 0.f)
    {
        return 0.f;
    }

    GAME_LOG(Game, Log, "Damage pre-hook = %f", Damage);

    const float Falloff = Damage / OriginalDamage;

    // Calculate knockback power
    float KnockbackPower = 1500.f * OriginalDamage; //TODO: this is where we should fetch projectile/weapon's knockback value
    KnockbackPower *= Falloff;

    // Gamemode hook
    if (AUR_GameMode* URGameMode = GetWorld()->GetAuthGameMode<AUR_GameMode>())
    {
        URGameMode->ModifyDamage(Damage, KnockbackPower, this, EventInstigator, DamageEvent, DamageCauser);

        if (Damage == 0.f)
        {
            return 0.f;
        }
    }

    GAME_LOG(Game, Log, "Damage post-hook = %f", Damage);

    ////////////////////////////////////////////////////////////
    // Apply Damage

    float DamageToShield = 0.f;
    float DamageToArmor = 0.f;
    float DamageToHealth = 0.f;
    float DamageRemaining = Damage;

    if (AttributeSet && AttributeSet->Health.GetCurrentValue() > 0.f)
    {
        const float CurrentShield = FMath::FloorToFloat(AttributeSet->Shield.GetCurrentValue());
        if (CurrentShield > 0.f)
        {
            DamageToShield = FMath::Min(DamageRemaining, CurrentShield);
            AttributeSet->SetShield(CurrentShield - DamageToShield);
            DamageRemaining -= DamageToShield;
        }

        const float CurrentArmor = FMath::FloorToFloat(AttributeSet->Armor.GetCurrentValue());
        const float ArmorAbsorption = AttributeSet->ArmorAbsorptionPercent.GetCurrentValue();
        if (CurrentArmor > 0.f && DamageRemaining > 0.f)
        {
            DamageToArmor = FMath::Min(DamageRemaining * ArmorAbsorption, CurrentArmor);
            DamageToArmor = FMath::FloorToFloat(DamageToArmor);
            AttributeSet->SetArmor(CurrentArmor - DamageToArmor);
            DamageRemaining -= DamageToArmor;
        }

        const float CurrentHealth = AttributeSet->Health.GetCurrentValue();
        if (CurrentHealth > 0.f && DamageRemaining > 0.f)
        {
            DamageToHealth = FMath::Min(DamageRemaining, CurrentHealth);
            AttributeSet->SetHealth(CurrentHealth - DamageToHealth);
            DamageRemaining -= DamageToHealth;
        }
    }

    GAME_LOG(Game, Log, "Damage repartition: Shield(%f), Armor(%f), Health(%f), Extra(%f)", DamageToShield, DamageToArmor, DamageToHealth, DamageRemaining);

    ////////////////////////////////////////////////////////////
    // Apply Knockback

    // Avoid very small knockbacks
    if (KnockbackPower / GetCharacterMovement()->Mass >= 100.f)
    {
        FVector KnockbackDir;
        if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
        {
            FPointDamageEvent* PointDamageEvent = (FPointDamageEvent*)&DamageEvent;

            // Always use shot direction for knockback
            KnockbackDir = PointDamageEvent->ShotDirection;
            // Bias towards +Z
            KnockbackDir = 0.75*KnockbackDir + FVector(0, 0, 0.25);

            GetCharacterMovement()->AddImpulse(KnockbackPower*KnockbackDir);
        }
        else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
        {
            FRadialDamageEvent* RadialDamageEvent = (FRadialDamageEvent*)&DamageEvent;

            // Use no falloff (constant) because we already scaled KnockbackPower
            GetCharacterMovement()->AddRadialImpulse(RadialDamageEvent->Origin, RadialDamageEvent->Params.GetMaxRadius(), KnockbackPower, ERadialImpulseFalloff::RIF_Constant, false);
            //TODO: Want to bias towards +Z as well, but dunno how to deal with Radial
        }
        else
        {
            // This is most likely a code-generated FDamageEvent() with no real source.
        }
    }

    ////////////////////////////////////////////////////////////
    // Other

    // @! TODO Limit Pain by time, vary by damage etc.
    if (DamageToHealth > 10.f)
    {
        //TODO: Replicate
        UGameplayStatics::PlaySoundAtLocation(this, CharacterVoice.PainSound, GetActorLocation(), GetActorRotation());
    }

    ////////////////////////////////////////////////////////////
    // Death

    if (AttributeSet && AttributeSet->Health.GetCurrentValue() <= 0)
    {
        // Can use DamageRemaining here to GIB
        Die(EventInstigator, DamageEvent, DamageCauser);
    }

    return Damage;
}

void AUR_Character::Die(AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser)
{
    // Already killed (might happen when multiple damage sources in 1 frame)
    if (GetTearOff() || IsPendingKillPending())
    {
        return;
    }

    AUR_GameMode* URGameMode = GetWorld()->GetAuthGameMode<AUR_GameMode>();
    if (URGameMode)
    {
        AController* Killed = GetController();

        if (URGameMode->PreventDeath(Killed, Killer, DamageEvent, DamageCauser))
        {
            // Make sure we don't stay with <=0 health or IsAlive() would return false.
            if (AttributeSet->Health.GetCurrentValue() <= 0)
            {
                AttributeSet->SetHealth(1);
            }
            return;
        }

        URGameMode->PlayerKilled(Killed, Killer, DamageEvent, DamageCauser);
        OnDied(Killer, DamageEvent, DamageCauser);
    }

    if (AttributeSet)
    {
        AttributeSet->SetHealth(0);
    }

    // Force stop firing
    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->Deactivate();
    }
    DesiredFireModeNum.Empty();

    // Cut the replication link
    TearOff();

    if (GetNetMode() == NM_DedicatedServer)
    {
        //Destroy();
        SetLifeSpan(0.200f);	// give it time to replicate the tear off ?
        SetActorEnableCollision(false);
    }
    else
    {
        PlayDeath();
    }
}

void AUR_Character::PlayDeath()
{
    APlayerController* OldController = nullptr;
    if (Controller && Controller->GetPawn() == this)
        OldController = Cast<APlayerController>(Controller);

    // Unpossess
    DetachFromControllerPendingDestroy();

    // Set view target back, should auto to 3p cam
    if (OldController)
        OldController->SetViewTarget(this);

    // Improve camera feel
    ThirdPersonArm->bInheritPitch = false;
    ThirdPersonArm->bInheritRoll = false;
    ThirdPersonArm->bEnableCameraLag = true;
    ThirdPersonArm->bEnableCameraRotationLag = true;

    GetCharacterMovement()->StopActiveMovement();

    // Disable capsule
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));

    // Set mesh as root with physics (ragdoll)
    GetMesh()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
    GetMesh()->SetSimulatePhysics(true);
    SetRootComponent(GetMesh());

    // Attach capsule back to mesh
    GetCapsuleComponent()->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, false));

    SetLifeSpan(5.0f);
}

bool AUR_Character::IsAlive() const
{
    if (GetTearOff() || IsPendingKillPending() || AttributeSet == nullptr)
    {
        return false;	// server link has been cut, health might not replicate anymore
    }

    return AttributeSet->GetHealth() > 0;
}

void AUR_Character::ServerSuicide_Implementation()
{
    Die(nullptr, FDamageEvent(), nullptr);
}


/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::BeginPickup()
{
    bIsPickingUp = true;
}

void AUR_Character::EndPickup()
{
    bIsPickingUp = false;
}

void AUR_Character::SelectWeapon1()
{
    FString name;
    name = "Assault Rifle";
    InventoryComponent->SelectWeapon(1);
}

void AUR_Character::SelectWeapon2()
{
    FString name;
    name = "Shotgun";
    InventoryComponent->SelectWeapon(2);
}

void AUR_Character::SelectWeapon3()
{
    FString name;
    name = "Rocket Launcher";
    InventoryComponent->SelectWeapon(3);
}

void AUR_Character::SelectWeapon4()
{
    FString name;
    name = "Grenade Launcher";
    InventoryComponent->SelectWeapon(4);
}

void AUR_Character::SelectWeapon5()
{
    FString name;
    name = "Sniper Rifle";
    InventoryComponent->SelectWeapon(5);
}

void AUR_Character::SelectWeapon0()
{
    FString name;
    name = "Pistol";
    InventoryComponent->SelectWeapon(0);
}

void AUR_Character::ShowInventory()
{
    InventoryComponent->ShowInventory();
}

FName AUR_Character::GetWeaponAttachPoint() const
{
    return WeaponAttachPoint;
}

USkeletalMeshComponent* AUR_Character::GetPawnMesh() const
{
    return MeshFirstPerson;
}

void AUR_Character::WeaponSelect(int32 InWeaponGroup)
{
    InventoryComponent->SelectWeapon(InWeaponGroup);
}

void AUR_Character::NextWeapon()
{
    if (InventoryComponent)
    {
        InventoryComponent->NextWeapon();
    }
}

void AUR_Character::PrevWeapon()
{
    if (InventoryComponent)
    {
        InventoryComponent->PrevWeapon();
    }
}

void AUR_Character::PawnStartFire(uint8 FireModeNum)
{
    /*
    bIsFiring = true;

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->LocalStartFire();
    }
    */

    DesiredFireModeNum.Insert(FireModeNum, 0);

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->RequestStartFire(FireModeNum);
    }
}

void AUR_Character::PawnStopFire(uint8 FireModeNum)
{
    /*
    bIsFiring = false;

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->LocalStopFire();
    }
    */

    DesiredFireModeNum.RemoveSingle(FireModeNum);

    if (InventoryComponent && InventoryComponent->ActiveWeapon)
    {
        InventoryComponent->ActiveWeapon->RequestStopFire(FireModeNum);

        if (DesiredFireModeNum.Num() > 0)
        {
            InventoryComponent->ActiveWeapon->RequestStartFire(DesiredFireModeNum[0]);
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_Character::Server_GiveAbility_Validate(TSubclassOf<UUR_GameplayAbility> InAbilityClass, const int32 InAbilityLevel) { return true; }
void AUR_Character::Server_GiveAbility_Implementation(TSubclassOf<UUR_GameplayAbility> InAbility, const int32 InAbilityLevel)
{
    if (InAbility == nullptr)
    {
        GAME_LOG(Game, Error, "InAbilityClass was Null!");
        return;
    }

    check(AbilitySystemComponent);

    if (AbilitySystemComponent)
    {
        // @! TODO : Unsure of the purpose of InputID, seems to be related to input/keybinding
        const int32 InputID{ 0 }; //static_cast<int32>(Cast<UUR_GameplayAbility>(InAbilityClass.GetDefaultObject())->Input);

        AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(InAbility, InAbilityLevel, InputID, this));
        GameplayAbilities.AddUnique(InAbility);
    }
}

bool AUR_Character::Server_RemoveAbility_Validate(TSubclassOf<UUR_GameplayAbility> InAbilityClass) { return true; }
void AUR_Character::Server_RemoveAbility_Implementation(TSubclassOf<UUR_GameplayAbility> InAbilityClass) const
{
    if (InAbilityClass == nullptr)
    {
        GAME_LOG(Game, Error, "InAbilityClass was Null!");
        return;
    }

    check(AbilitySystemComponent);

    if (AbilitySystemComponent)
    {
        const auto& Abilities = AbilitySystemComponent->GetActivatableAbilities();

        for (const auto& Ability : Abilities)
        {
            if (Ability.Ability->GetClass() == InAbilityClass)
            {
                AbilitySystemComponent->ClearAbility(Ability.Handle);
                break;
            }
        }
    }
}

int32 AUR_Character::GetAbilityLevel(TSubclassOf<UUR_GameplayAbility> InAbilityClass) const
{
    int32 OutLevel{ -1 };

    if (AbilitySystemComponent)
    {
        const auto& Abilities = AbilitySystemComponent->GetActivatableAbilities();

        for (const auto& Ability : Abilities)
        {
            if (Ability.Ability->GetClass() == InAbilityClass)
            {
                OutLevel = Ability.Level;
                break;
            }
        }
    }

    return OutLevel;
}

bool AUR_Character::Server_SetAbilityLevel_Validate(TSubclassOf<UUR_GameplayAbility> InAbilityClass, const int32 InAbilityLevel) { return true; }
void AUR_Character::Server_SetAbilityLevel_Implementation(TSubclassOf<UUR_GameplayAbility> InAbilityClass, const int32 InAbilityLevel)
{
    if (AbilitySystemComponent)
    {
        // Get a reference to our ASC ActiveatableAbilities. Reference so our modifications stick.
        auto& Abilities = AbilitySystemComponent->GetActivatableAbilities();

        for (auto& Ability : Abilities)
        {
            if (Ability.Ability->GetClass() == InAbilityClass)
            {
                Ability.Level = InAbilityLevel;
                AbilitySystemComponent->MarkAbilitySpecDirty(Ability);
                break;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int32 AUR_Character::GetTeamIndex_Implementation()
{
    if (const auto PS = GetPlayerState<AUR_PlayerState>())
    {
        return IUR_TeamInterface::Execute_GetTeamIndex(PS);
    }
    return -1;
}

void AUR_Character::SetTeamIndex_Implementation(int32 NewTeamIndex)
{
    if (auto PS = GetPlayerState<AUR_PlayerState>())
    {
        IUR_TeamInterface::Execute_SetTeamIndex(PS, NewTeamIndex);
    }
}
