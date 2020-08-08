// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Pickup.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"

#include "OpenTournament.h"
#include "UR_Character.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Pickup::AUR_Pickup(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    PickupState(EPickupState::Active),
    RespawnInterval(20.f),
    DisplayName("Pickup")
{
    CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
    CollisionComponent->SetCapsuleSize(20.f, 20.f, true);
    CollisionComponent->Mobility = EComponentMobility::Static;
    CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    RootComponent = CollisionComponent;

    CollisionComponent->SetGenerateOverlapEvents(true);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_Pickup::OnOverlap);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    StaticMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
    StaticMesh->SetupAttachment(RootComponent);

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(RootComponent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Pickup::OnOverlap(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AUR_Character* URCharacter = Cast<AUR_Character>(Other))
    {
        GAME_LOG(Game, Log, "Overlap on Pickup (%s) by Character (%s)", *GetName(), *URCharacter->GetName());

        if (IsPickupValid(URCharacter))
        {
            GAME_LOG(Game, Log, "Valid Pickup of PickupActor (%s) by Character (%s)", *GetName(), *URCharacter->GetName());

            Pickup(URCharacter);
        }
    }
}

bool AUR_Pickup::IsPickupValid(const AUR_Character* PickupCharacter) const
{
    return !GetWorld()->LineTraceTestByChannel(PickupCharacter->GetActorLocation(), GetActorLocation(), ECC_Pawn, FCollisionQueryParams(), WorldResponseParams);
}

bool AUR_Pickup::IsPickupPermitted(const AUR_Character* PickupCharacter) const
{
    if (PickupCharacter == nullptr)
    {
        GAME_LOG(Game, Log, "Error. Character was invalid.");
        return false;
    }

    // Check if the actor doing the pickup has any Required or Excluded GameplayTags
    // e.g. Check for Red/Blue team tag, or exclude Flag-carrier tag, etc.
    FGameplayTagContainer TargetTags;
    PickupCharacter->GetOwnedGameplayTags(TargetTags);

    return IsPermittedByGameplayTags(TargetTags);
}

bool AUR_Pickup::IsPermittedByGameplayTags(const FGameplayTagContainer& TargetTags) const
{
    if (RequiredTags.Num() == 0 || (bRequiredTagsExact && TargetTags.HasAnyExact(RequiredTags)) || (!bRequiredTagsExact && TargetTags.HasAny(RequiredTags)))
    {
        return (ExcludedTags.Num() == 0 || (bExcludedTagsExact && !TargetTags.HasAnyExact(ExcludedTags)) || (!bExcludedTagsExact && TargetTags.HasAny(RequiredTags)));
    }
    else
    {
        return false;
    }
}

void AUR_Pickup::Pickup(AUR_Character* PickupCharacter)
{
    if (IsPickupPermitted(PickupCharacter))
    {
        OnPickup(PickupCharacter);
        SetPickupState(EPickupState::Inactive);

        // TODO : Actual Pickup change for Character
    }
}

void AUR_Pickup::OnPickup_Implementation(AUR_Character* PickupCharacter)
{
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupSound, GetActorLocation());

    if (auto PC = Cast<APlayerController>(PickupCharacter->GetController()))
    {
        PC->ClientMessage(TEXT("Got Pickup"));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Pickup::SetPickupState(const EPickupState NewState)
{
    if (PickupState == NewState)
    {
        return;
    }

    PickupState = NewState;

    if (PickupState == EPickupState::Active)
    {
        StaticMesh->SetVisibility(true);
        SetActorEnableCollision(true);

        GetWorld()->GetTimerManager().ClearTimer(RespawnHandle);
    }
    else if (PickupState == EPickupState::Inactive)
    {
        StaticMesh->SetVisibility(false);
        SetActorEnableCollision(false);

        GetWorld()->GetTimerManager().SetTimer(RespawnHandle, this, &AUR_Pickup::RespawnPickup, RespawnInterval, false);
    }
}

void AUR_Pickup::RespawnPickup()
{
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), RespawnSound, GetActorLocation());

    SetPickupState(EPickupState::Active);
}
