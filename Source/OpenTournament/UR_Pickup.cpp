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
#include "UR_FunctionLibrary.h"
#include "UR_GameState.h"
#include "UR_PlayerState.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Pickup::AUR_Pickup(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    DisplayName(TEXT("Item"))
{
    CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
    CollisionComponent->SetCapsuleSize(20.f, 20.f, true);
    CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_Pickup::OnOverlap);
    CollisionComponent->SetGenerateOverlapEvents(true);

    RootComponent = CollisionComponent;

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    StaticMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
    StaticMesh->SetupAttachment(RootComponent);

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(RootComponent);

    SetReplicates(true);
    SetReplicatingMovement(false);
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
    return HasAuthority() && !GetWorld()->LineTraceTestByChannel(PickupCharacter->GetActorLocation(), GetActorLocation(), ECC_Pawn, FCollisionQueryParams(), WorldResponseParams);
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
        if (AUR_GameState* GS = GetWorld()->GetGameState<AUR_GameState>())
        {
            GS->MulticastPickupEvent(this, PickupCharacter->GetPlayerState<AUR_PlayerState>());
        }
        MulticastPickedUp(PickupCharacter);
    }
}

void AUR_Pickup::MulticastPickedUp_Implementation(AUR_Character* PickupCharacter)
{
    bool bDestroy = OnPickup(PickupCharacter);

    if (!IsNetMode(NM_DedicatedServer))
    {
        PlayPickupEffects(PickupCharacter);
    }

    OnPickedUp.Broadcast(this, PickupCharacter);

    if (bDestroy)
    {
        //NOTE: I'm not sure if it is viable to Multicast & Destroy in the same frame.
        // If clients occasionally don't receive it, this will likely be the culprit.
        Destroy();
    }
}

bool AUR_Pickup::OnPickup_Implementation(AUR_Character* PickupCharacter)
{
    return true;
}

void AUR_Pickup::PlayPickupEffects_Implementation(AUR_Character* PickupCharacter)
{
    //TBD: may want to attach effect to character? probably depends on cases.
    // Maybe leave this to BP subclasses?
    UUR_FunctionLibrary::SpawnEffectAtLocation(this, PickupEffect, GetTransform());

    UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupSound, GetActorLocation());
}
