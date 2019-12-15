// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Character.h"

#include "UnrealNetwork.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine.h"

#include "OpenTournament.h"
#include "UR_HealthComponent.h"
#include "UR_ArmorComponent.h"
#include "UR_InventoryComponent.h"
#include "UR_CharacterMovementComponent.h"
#include "UR_PlayerController.h"
#include "UR_Projectile.h"
#include "UR_Projectile_Assault.h"


/////////////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
AUR_Character::AUR_Character(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer.SetDefaultSubobjectClass<UUR_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName)),
    FootstepTimestamp(0.f),
    FootstepTimeIntervalBase(0.300f),
    FallDamageSpeedThreshold(2675.f)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    // Unreal & UT99 Values (Scaling Factor 2.5)
    GetCapsuleComponent()->InitCapsuleSize(42.5, 97.5);

    URMovementComponent = Cast<UUR_CharacterMovementComponent>(GetCharacterMovement());
    URMovementComponent->bUseFlatBaseForFloorChecks = true;

    HealthComponent = Cast<UUR_HealthComponent>(CreateDefaultSubobject<UUR_HealthComponent>(TEXT("HealthComponent")));
	ArmorComponent = Cast<UUR_ArmorComponent>(CreateDefaultSubobject<UUR_ArmorComponent>(TEXT("ArmorComponent")));
	InventoryComponent = Cast<UUR_InventoryComponent>(CreateDefaultSubobject<UUR_InventoryComponent>(TEXT("InventoryComponent")));

    // Create a CameraComponent	
    CharacterCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    CharacterCameraComponent->SetupAttachment(GetCapsuleComponent());
    CharacterCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
    CharacterCameraComponent->bUsePawnControlRotation = true;

    // Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
    MeshFirstPerson = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFirstPerson"));
    MeshFirstPerson->SetOnlyOwnerSee(true);
    MeshFirstPerson->SetupAttachment(CharacterCameraComponent);
    MeshFirstPerson->bCastDynamicShadow = false;
    MeshFirstPerson->CastShadow = false;
    MeshFirstPerson->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
    MeshFirstPerson->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	ConstructorHelpers::FObjectFinder<UAnimationAsset> fireAnimAsset(TEXT("AnimSequence'/Game/FirstPerson/Animations/FirstPerson_Fire.FirstPerson_Fire'"));
	fireAnim = fireAnimAsset.Object;
	WeaponAttachPoint = "GripPoint";

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
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AUR_Character, HealthComponent);
	DOREPLIFETIME(AUR_Character, InventoryComponent);
    DOREPLIFETIME(AUR_Character, DodgeDirection);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::BeginPlay()
{
    Super::BeginPlay();
	HealthComponent->SetHealth(100);
}

void AUR_Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickFootsteps(DeltaTime);
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

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AUR_Character::NextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AUR_Character::PrevWeapon);
}

void AUR_Character::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (IsAlive() && CharacterCameraComponent && CharacterCameraComponent->IsActive())
	{
		CharacterCameraComponent->GetCameraView(DeltaTime, OutResult);
	}
	else if (ThirdPersonCamera && ThirdPersonCamera->IsActive())
	{
		ThirdPersonCamera->GetCameraView(DeltaTime, OutResult);
	}
	else
	{
		Super::CalcCamera(DeltaTime, OutResult);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

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
        AddMovementInput(FVector(0.f,0.f,1.f), InValue);
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
    const float FootstepVolume = FMath::Clamp(0.2f, 1.f, WalkingSpeedPercentage);
    UGameplayStatics::PlaySound2D(GetWorld(), CharacterVoice.FootstepSound, FootstepVolume, 1.f);
}



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

void AUR_Character::Landed(const FHitResult & Hit)
{
    TakeFallingDamage(Hit, GetCharacterMovement()->Velocity.Z);

    Super::Landed(Hit);
}

void AUR_Character::TakeFallingDamage(const FHitResult& Hit, float FallingSpeed)
{
    // Do nothing yet
    // Get our health component & apply damage
    
    if (HasAuthority() && URMovementComponent != nullptr)
    {
        // @! TODO Proper Damage Handling
        if (HealthComponent)
        {
            if (FallingSpeed * -1.f > FallDamageSpeedThreshold)
            {
                const float FallingDamage = 0.15f * (FallDamageSpeedThreshold - FallingSpeed);

                if (FallingDamage >= 1.0f)
                {
                    FDamageEvent DamageEvent; // @! TODO Real DamageTypes
                    TakeDamage(FallingDamage, DamageEvent, Controller, this);
                }
            }
        }
    }
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
    return !URMovementComponent->IsFlying();
}

bool AUR_Character::CanDodge() const
{
    return IsDodgeCapable() && IsDodgePermitted();
}

void AUR_Character::Dodge(FVector DodgeDir, FVector DodgeCross)
{
    if (CanDodge())
    {
        if ( DodgeOverride(DodgeDir, DodgeCross) )
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

bool AUR_Character::ServerSetDodgeDirection_Validate(const EDodgeDirection InDodgeDirection) { return true; }
void AUR_Character::ServerSetDodgeDirection_Implementation(const EDodgeDirection InDodgeDirection)
{
    DodgeDirection = InDodgeDirection;
}

void AUR_Character::OnDodge_Implementation(const FVector& DodgeLocation, const FVector& DodgeDir)
{
    // @! TODO Effects
    if (CharacterVoice.DodgeSound != nullptr)
    {
        if (HasAuthority())
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
        if (HasAuthority())
        {
            UGameplayStatics::PlaySoundAtLocation(this, CharacterVoice.DodgeSound, GetActorLocation(), GetActorRotation());
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

float AUR_Character::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser))
	{
		return 0.f;
	}

	// Super() takes care of calculating proper splash damage values and imparting components physics.
	// Then it triggers events : OnTakePointDamage, OnTakeRadialDamage, OnTakeAnyDamage.
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	/*if (HealthComponent)
	{
		HealthComponent->ChangeHealth(-1 * Damage); //leaving this here for reference is need be
	}*/

	float ArmorPct = 0.60f;

	if (HealthComponent)
	{
		int32 ArmorAbsorb = 0;

		if (ArmorComponent)
		{
			if (ArmorComponent->hasBarrier)
				ArmorAbsorb = FMath::Min((int32)ActualDamage, ArmorComponent->Armor);
			else
				ArmorAbsorb = FMath::Min((int32)((float)ActualDamage * ArmorPct), ArmorComponent->Armor);

			ArmorComponent->ChangeArmor(-1 * ArmorAbsorb);

			if (ArmorComponent->Armor == 0)
				ArmorComponent->SetBarrier(false);
		}

		HealthComponent->ChangeHealth(-1 * (ActualDamage - ArmorAbsorb));

		/*
			if (ArmorComponent->Armor < 0.4*Damage && ArmorComponent->Armor > 0)
			{
				int32 currentArmor = ArmorComponent->Armor;
				ArmorComponent->ChangeArmor(-1 * ArmorComponent->Armor);
				HealthComponent->ChangeHealth(-1 * (Damage - currentArmor));
			}
			else if (ArmorComponent->Armor > Damage && ArmorComponent->hasBarrier)
			{
				ArmorComponent->ChangeArmor(-1 * Damage);
			}
			else if (ArmorComponent->Armor <= 0)
			{
				HealthComponent->ChangeHealth(-1 * Damage);
			}
			else if (ArmorComponent->Armor > 0.4*Damage && !ArmorComponent->hasBarrier)
			{
				ArmorComponent->ChangeArmor(-0.6 * Damage);
				HealthComponent->ChangeHealth(-0.4 * Damage);
			}
		*/
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Damage Event 2 - DAMAGE -: %f"), ActualDamage));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Damage Event 2 - Remaining Health -: %d"), HealthComponent->Health));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Damage Event 2 - Remaining Armor -: %d"), ArmorComponent->Armor));


	//NOTE: it seems like we are lacking control of damage momentum (knockback) overall.
	// DamageType has DamageImpulse but it is part of CDO, which is a bit annoying.
	// We cannot adjust DamageImpulse on the fly with gamemode/mutators.
	// Also it only applies to physics-enabled stuff. Characters need custom knockback anyways.

	// We will probably need to find a way back to the projectile/weapon (DamageCauser?),
	// so we can use custom-defined knockback values there.

	// Then we can apply splash falloff using (ActualDamage/Damage) to the knockback power.
	// And finally, apply knockback manually with a custom impulse.

	// For now, let's try basic values
	float KnockbackPower = 1500.f * ActualDamage;

	// Avoid very small knockbacks
	if (KnockbackPower / GetCharacterMovement()->Mass >= 100.f)
	{
		FVector KnockbackDir;
		if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		{
			FPointDamageEvent* PointDamageEvent = (FPointDamageEvent*)&DamageEvent;

			// Always use shot direction for knockback
			KnockbackDir = PointDamageEvent->ShotDirection;
			GetCharacterMovement()->AddImpulse(KnockbackPower*KnockbackDir);
		}
		else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
		{
			FRadialDamageEvent* RadialDamageEvent = (FRadialDamageEvent*)&DamageEvent;

			// Use no falloff (constant) because we already scaled KnockbackPower
			GetCharacterMovement()->AddRadialImpulse(RadialDamageEvent->Origin, RadialDamageEvent->Params.GetMaxRadius(), KnockbackPower, ERadialImpulseFalloff::RIF_Constant, false);

			/*
			// Experimental: use vector (HitLocation->EyeLocation) as knockback direction to make it feel more natural

			KnockbackDir = (GetPawnViewLocation() - RadialDamageEvent->Origin);
			KnockbackDir.Normalize();
			GetCharacterMovement()->AddImpulse(KnockbackPower*KnockbackDir);
			*/
		}
	}

	if (HealthComponent && HealthComponent->Health <= 0)
	{
		Die(EventInstigator, DamageEvent, DamageCauser);
	}

	return ActualDamage;
}

void AUR_Character::Die(AController* Killer, const FDamageEvent& DamageEvent, AActor* DamageCauser)
{
	// Already killed (might happen when multiple damage sources in 1 frame)
	if (GetTearOff() || IsPendingKillPending())
		return;

	// Here we can hook game mode + mutators for things like :
	// - prevent death
	// - announce kill (death message, kill message, sprees, etc)
	// - score kill (add to score / team score)

	if (HealthComponent)
	{
		HealthComponent->SetHealth(0);
	}

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

	// Not sure why these are still rendered as "owned" after unpossessing. Maybe because it is ViewTarget.
	GetMesh()->SetOwnerNoSee(false);
	MeshFirstPerson->SetVisibility(false, true);

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

bool AUR_Character::IsAlive()
{
	if (GetTearOff() || IsPendingKillPending())
		return false;	// server link has been cut, health might not replicate anymore

	if (HealthComponent)
		return HealthComponent->Health > 0;

	// not sure if we should return true or false when no HealthComponent
	return true;
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

USkeletalMeshComponent* AUR_Character::GetSpecifcPawnMesh(bool WantFirstPerson) const
{
	return MeshFirstPerson;

}


bool AUR_Character::IsFirstPerson() const
{
	return Controller && Controller->IsLocalPlayerController();
}


void AUR_Character::WeaponSelect(int32 number) {
	InventoryComponent->SelectWeapon(number);
}

void AUR_Character::NextWeapon()
{
	if (InventoryComponent)
		InventoryComponent->NextWeapon();
}

void AUR_Character::PrevWeapon()
{
	if (InventoryComponent)
		InventoryComponent->PrevWeapon();
}

void AUR_Character::PawnStartFire(uint8 FireModeNum)
{
	isFiring = true;
	//Fire();

	if (InventoryComponent && InventoryComponent->ActiveWeapon)
	{
		InventoryComponent->ActiveWeapon->LocalStartFire();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("NO WEAPON SELECTED!")));
	}
}

void AUR_Character::PawnStopFire(uint8 FireModeNum)
{
	isFiring = false;

	if (InventoryComponent && InventoryComponent->ActiveWeapon)
	{
		InventoryComponent->ActiveWeapon->LocalStopFire();
	}
}

//deprecated
void AUR_Character::Fire()
{
	if (isFiring) {
		if (InventoryComponent->ActiveWeapon != NULL) {
			if (InventoryComponent->ActiveWeapon->ProjectileClass)
			{
				GetActorEyesViewPoint(InventoryComponent->ActiveWeapon->Location, InventoryComponent->ActiveWeapon->Rotation);
				FVector MuzzleLocation = InventoryComponent->ActiveWeapon->Location + FTransform(InventoryComponent->ActiveWeapon->Rotation).TransformVector(MuzzleOffset);
				FRotator MuzzleRotation = InventoryComponent->ActiveWeapon->Rotation;

				UWorld* World = GetWorld();
				if (World)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.Owner = this;
					SpawnParams.Instigator = this;
					InventoryComponent->ActiveWeapon->Fire(World, MuzzleLocation, MuzzleRotation, SpawnParams);
					MeshFirstPerson->PlayAnimation(fireAnim, false);
				}
			}
		}
		else
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("NO WEAPON SELECTED!")));
	}
}
