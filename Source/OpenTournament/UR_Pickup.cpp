// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Pickup.h"

#include <Engine/World.h>

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

#include "OpenTournament.h"
#include "UR_Character.h"
#include "UR_FunctionLibrary.h"
#include "UR_GameState.h"
#include "UR_LogChannels.h"
#include "UR_PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_Pickup)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Pickup::AUR_Pickup(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , DisplayName(FText::FromString(TEXT("Item")))
    , bBroadcastPickupEvent(false)
{
    CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
    CollisionComponent->SetCapsuleSize(20.f, 20.f, true);
    CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlap);
    CollisionComponent->SetGenerateOverlapEvents(true);

    RootComponent = CollisionComponent;

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    StaticMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
    StaticMesh->SetupAttachment(RootComponent);

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(RootComponent);

    bReplicates = true;
    SetReplicatingMovement(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Pickup::OnOverlap(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AUR_Character* URCharacter = Cast<AUR_Character>(Other))
    {
        GAME_LOG(LogGame, Log, "Overlap on Pickup (%s) by Character (%s)", *GetName(), *URCharacter->GetName());

        if (IsPickupValid(URCharacter))
        {
            GAME_LOG(LogGame, Log, "Valid Pickup of PickupActor (%s) by Character (%s)", *GetName(), *URCharacter->GetName());

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
        GAME_LOG(LogGame, Log, "Error. Character was invalid.");
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

/**
* NOTE: Here is a typical UE4 unpractical situation.
*
* When picked up, the pickup might not be relevant to everyone.
* For players whom it is relevant to, we want a multicast here to play effects and sounds.
*
* In parallel, we also want to multicast a global event for potential pickup announcement (powerup announcement / spectator pickup awareness).
*
* --------------------
*
* The traditional UT way is to replicate the pickup CLASS for the announcement, and use CDO data which is always available on clients.
* This is generally done by passing pickup class as OptionalObject for a LocalMessage.
* This is highly unpractical because the CDO cannot be customized in any way.
* A good example/use case would be for weapon-bases and/or dropped-weapon pickups.
* We dont want to prepare a XXXGunPickup & DroppedXXXGunPickup class for every single weapon.
* Rather, we want a single WeaponPickup & DroppedWeaponPickup class in which the weapon is a property, set at runtime.
* The CDO is useless in this case.
*
* To achieve this I want to try a slightly different approach.
* The idea is to force pickup relevancy to everyone upon pickup, just before destroying.
* Therefore we can have any amount of customizeable properties within pickup, to be replicated for the PickupEvent.
* There is also a side little advantage to this : we can do both things through a single replication channel.
*
* --------------------
*
* In retrospect, this is maybe a bit overkill.
* We should also take into account that the pickups that need to be broadcasted globally are only a small fraction of them.
* Example: When picking up ODamage, all spectators should see "X got ODamage" and we might also want an ingame announcement.
* However when picking up vials, weapons, or ammo, nobody cares except recipient and his direct spectators.
*
* Therefore for standard pickups we could stick to standard multicast, that would trigger only on relevant clients.
* It would trigger Recipient->PickupEvent (a character event, not a gamestate event), to be primarily used for UI pickup message.
*
* And then for specific pickups (powerups) we can add a second replication path for global pickup announcements through gamestate.
* This other path however will not have the Pickup object available due to possible lack of relevance, so we pass the class.
* As stated above passing the class prevents runtime customizability, but then again globally-announced pickups are sparse so it's a good compromise.
*/

void AUR_Pickup::Pickup(AUR_Character* PickupCharacter)
{
    if (IsPickupPermitted(PickupCharacter))
    {
        if (bBroadcastPickupEvent)
        {
            if (AUR_GameState* GS = GetWorld()->GetGameState<AUR_GameState>())
            {
                GS->MulticastPickupEvent(this->GetClass(), PickupCharacter->GetPlayerState<AUR_PlayerState>());
            }
        }
        /*
        ForceNetRelevant();
        */
        MulticastPickedUp(PickupCharacter);
    }
}

void AUR_Pickup::MulticastPickedUp_Implementation(AUR_Character* PickupCharacter)
{
    bool bDestroy = OnPickup(PickupCharacter);
    // NOTE: we should probably always destroy?

    if (!IsNetMode(NM_DedicatedServer))
    {
        PlayPickupEffects(PickupCharacter);
    }

    OnPickedUp.Broadcast(this, PickupCharacter);

    if (PickupCharacter)
    {
        PickupCharacter->PickupEvent.Broadcast(this);
    }

    if (bDestroy)
    {
        // Let the Pickup replicate & multicast to clients?
        /*
        SetLifeSpan(0.200f);
        SetActorEnableCollision(false);
        SetHidden(true);
        */
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
