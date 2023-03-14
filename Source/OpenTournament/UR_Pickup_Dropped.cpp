// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Pickup_Dropped.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Pickup_Dropped::AUR_Pickup_Dropped(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    //CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    //CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);

    // Projectile should do the job as it is blocked by walls and movers, and overlaps pawns
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->SetUpdatedComponent(RootComponent);
    ProjectileMovementComponent->Velocity = FVector(1.f, 0.f, 0.1f);
    ProjectileMovementComponent->InitialSpeed = 1100.0f;
    ProjectileMovementComponent->MaxSpeed = 1100.0f;
    ProjectileMovementComponent->ProjectileGravityScale = 1.f;
    ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &AUR_Pickup_Dropped::OnProjectileStop);

    InitialLifeSpan = 10.f;
    RemainingLifeSpan = -1;
    ExpireEffectDuration = 2.f;
    RotationRate = 0.f;

    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.bCanEverTick = true;
}

void AUR_Pickup_Dropped::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AUR_Pickup_Dropped, RemainingLifeSpan, COND_InitialOnly);
}

void AUR_Pickup_Dropped::BeginPlay()
{
    Super::BeginPlay();

    CreatedAt = GetWorld()->GetTimeSeconds();

    if (HasAuthority() && InitialLifeSpan > 0)
    {
        RemainingLifeSpan = InitialLifeSpan;

        // Update sparsely
        GetWorldTimerManager().SetTimer(RefreshRemainingLifeSpanTimerHandle, this, &AUR_Pickup_Dropped::RefreshRemainingLifeSpan, 0.5f, true);
        /**
        * Ideally we should update the variable whenever the actor becomes relevant to a new client and is about to replicate initially.
        * Unfortunately there is no such event, so we have to update it on a tick or timer basis :(
        */
    }

    if (RemainingLifeSpan > 0 && ExpireEffectDuration > 0)
    {
        GetWorldTimerManager().SetTimer(ExpireTimerHandle, this, &AUR_Pickup_Dropped::PlayExpire, FMath::Max(InitialLifeSpan - ExpireEffectDuration, 0.001f), false);
    }
}

void AUR_Pickup_Dropped::RefreshRemainingLifeSpan()
{
    // Send this for bRepInitial
    RemainingLifeSpan = GetLifeSpan() - 0.25f;
}

void AUR_Pickup_Dropped::OnProjectileStop(const FHitResult& ImpactResult)
{
    SetReplicateMovement(false);
}

void AUR_Pickup_Dropped::PlayExpire()
{
    bExpiring = true;
    OriginalScale = GetActorScale3D();
    SetActorTickEnabled(true);
    // Speed up the effect if we don't have enough time left
    ExpireEffectDuration = GetRemainingLifeSpan();
}

void AUR_Pickup_Dropped::Tick(float DeltaTime)
{
    if (bExpiring)
    {
        SetActorScale3D(FMath::VInterpConstantTo(GetActorScale3D(), FVector::ZeroVector, DeltaTime, OriginalScale.Size() / ExpireEffectDuration));
        RotationRate += DeltaTime * (720.f / ExpireEffectDuration);
        AddActorWorldRotation(DeltaTime * RotationRate * FRotator(0.f, 1.f, 0.f));
    }
}

bool AUR_Pickup_Dropped::IsPickupPermitted(const AUR_Character* PickupCharacter) const 
{
    // If pickup was dropped by someone, do not allow owner to pick it during a delay (must be short enough to allow juggling tho)
    if ((AActor*)PickupCharacter == GetOwner() && GetWorld()->TimeSince(CreatedAt) < 0.8f)
    {
        return false;
    }
    return Super::IsPickupPermitted(PickupCharacter);
}

float AUR_Pickup_Dropped::GetRemainingLifeSpan()
{
    if (HasAuthority())
    {
        return GetLifeSpan();
    }
    else
    {
        return FMath::Max(0.f, RemainingLifeSpan - GetWorld()->TimeSince(CreatedAt));
    }
}
