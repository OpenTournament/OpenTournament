// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Character.h"

#include "UnrealNetwork.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

#include "OpenTournament.h"
#include "UR_HealthComponent.h"
#include "UR_CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
AUR_Character::AUR_Character(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UUR_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
    , FallDamageSpeedThreshold(2675.f)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    // Unreal & UT99 Values (Scaling Factor 2.5)
    GetCapsuleComponent()->InitCapsuleSize(42.5, 97.5);

    URMovementComponent = Cast<UUR_CharacterMovementComponent>(GetCharacterMovement());
    URMovementComponent->bUseFlatBaseForFloorChecks = true;

    HealthComponent = Cast<UUR_HealthComponent>(CreateDefaultSubobject<UUR_HealthComponent>(TEXT("HealthComponent")));

    // Create a CameraComponent	
    CharacterCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    CharacterCameraComponent->SetupAttachment(GetCapsuleComponent());
    CharacterCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
    CharacterCameraComponent->bUsePawnControlRotation = true;

    // Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
    MeshFirstPerson = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFirstPerson"));
    MeshFirstPerson->SetOnlyOwnerSee(true);
    MeshFirstPerson->SetupAttachment(CharacterCameraComponent);
    MeshFirstPerson->bCastDynamicShadow = false;
    MeshFirstPerson->CastShadow = false;
    MeshFirstPerson->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
    MeshFirstPerson->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

    // Create a gun mesh component
    MeshWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshWeapon"));
    MeshWeapon->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
    MeshWeapon->bCastDynamicShadow = false;
    MeshWeapon->CastShadow = false;
    MeshWeapon->SetupAttachment(MeshFirstPerson, TEXT("GripPoint"));
    //FP_Gun->SetupAttachment(RootComponent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    DOREPLIFETIME(AUR_Character, HealthComponent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::BeginPlay()
{
    Super::BeginPlay();
    
}

void AUR_Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AUR_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::MoveForward(const float InValue)
{
    if (InValue != 0.0f)
    {
        // find out which way is forward
        const FRotator ControlRotation = GetControlRotation();
        FRotator ForwardRotation = ControlRotation;

        if (URMovementComponent != nullptr && URMovementComponent->Is3DMovementMode())
        {
            ForwardRotation = FRotator(0, ControlRotation.Yaw, 0);
        }

        // add movement in forward direction
        AddMovementInput(FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::X), InValue);
    }
}

void AUR_Character::MoveRight(const float InValue)
{
    if (InValue != 0.0f)
    {
        // find out which way is right
        const FRotator ControlRotation = GetControlRotation();
        const FRotator ForwardRotation(0, ControlRotation.Yaw, 0);

        // add movement in right direction
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

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Character::CheckJumpInput(float DeltaTime)
{
    if (URMovementComponent)
    {
        URMovementComponent->CheckJumpInput(DeltaTime);

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

void AUR_Character::Landed(const FHitResult & Hit)
{
    TakeFallingDamage(Hit, GetCharacterMovement()->Velocity.Z);

    Super::Landed(Hit);
}

void AUR_Character::TakeFallingDamage(const FHitResult& Hit, float FallingSpeed)
{
    // Do nothing yet
    // Get our health component & apply damage
    
    if (Role == ROLE_Authority && URMovementComponent != nullptr)
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
        OnDodge(GetActorLocation(), DodgeDir);
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
        if (Role == ROLE_Authority)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CharacterVoice.DodgeSound, GetActorLocation(), GetActorRotation());
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

float AUR_Character::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
    AActor* DamageCauser)
{
    if (!ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser))
    {
        return 0.f;
    }

    if (HealthComponent)
    {
        HealthComponent->ChangeHealth(-1 * Damage);
    }

    return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}
