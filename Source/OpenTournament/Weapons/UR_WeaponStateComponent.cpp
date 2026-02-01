// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_WeaponStateComponent.h"

#include "GameplayEffectTypes.h"
#include "NativeGameplayTags.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

#include "Equipment/UR_EquipmentManagerComponent.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "Teams/UR_TeamSubsystem.h"
#include "Weapons/UR_RangedWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_WeaponStateComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Gameplay_Zone, "Gameplay.Zone");

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_WeaponStateComponent::UUR_WeaponStateComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsReplicatedByDefault(true);

    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.bCanEverTick = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_WeaponStateComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (APawn* Pawn = GetPawn<APawn>())
    {
        if (UUR_EquipmentManagerComponent* EquipmentManager = Pawn->FindComponentByClass<UUR_EquipmentManagerComponent>())
        {
            if (UUR_RangedWeaponInstance* CurrentWeapon = Cast<UUR_RangedWeaponInstance>(EquipmentManager->GetFirstInstanceOfType(UUR_RangedWeaponInstance::StaticClass())))
            {
                CurrentWeapon->Tick(DeltaTime);
            }
        }
    }
}

bool UUR_WeaponStateComponent::ShouldShowHitAsSuccess(const FHitResult& Hit) const
{
    //AActor* HitActor = Hit.GetActor();

    //@TODO: Don't treat a hit that dealt no damage (due to invulnerability or similar) as a success
    UWorld* World = GetWorld();
    if (UUR_TeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UUR_TeamSubsystem>(World))
    {
        return TeamSubsystem->CanCauseDamage(GetController<APlayerController>(), Hit.GetActor());
    }

    return false;
}

bool UUR_WeaponStateComponent::ShouldUpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext) const
{
    //@TODO: Implement me, for the purposes of this component we really only care about damage caused by a weapon
    // or projectile fired from a weapon, and should filter to that
    // (or perhaps see if the causer is also the source of our active reticle config)
    return EffectContext.GetEffectCauser() != nullptr;
}

void UUR_WeaponStateComponent::ClientConfirmTargetData_Implementation(uint16 UniqueId, bool bSuccess, const TArray<uint8>& HitReplaces)
{
    for (int i = 0; i < UnconfirmedServerSideHitMarkers.Num(); i++)
    {
        FGameServerSideHitMarkerBatch& Batch = UnconfirmedServerSideHitMarkers[i];
        if (Batch.UniqueId == UniqueId)
        {
            if (bSuccess && (HitReplaces.Num() != Batch.Markers.Num()))
            {
                bool bFoundShowAsSuccessHit = false;

                int32 HitLocationIndex = 0;
                for (const FGameScreenSpaceHitLocation& Entry : Batch.Markers)
                {
                    if (!HitReplaces.Contains(HitLocationIndex) && Entry.bShowAsSuccess)
                    {
                        // Only need to do this once
                        if (!bFoundShowAsSuccessHit)
                        {
                            ActuallyUpdateDamageInstigatedTime();
                        }

                        bFoundShowAsSuccessHit = true;

                        LastWeaponDamageScreenLocations.Add(Entry);
                    }
                    ++HitLocationIndex;
                }
            }

            UnconfirmedServerSideHitMarkers.RemoveAt(i);
            break;
        }
    }
}

void UUR_WeaponStateComponent::AddUnconfirmedServerSideHitMarkers(const FGameplayAbilityTargetDataHandle& InTargetData, const TArray<FHitResult>& FoundHits)
{
    FGameServerSideHitMarkerBatch& NewUnconfirmedHitMarker = UnconfirmedServerSideHitMarkers.Emplace_GetRef(InTargetData.UniqueId);

    if (APlayerController* OwnerPC = GetController<APlayerController>())
    {
        for (const FHitResult& Hit : FoundHits)
        {
            FVector2D HitScreenLocation;
            if (UGameplayStatics::ProjectWorldToScreen(OwnerPC, Hit.Location, /*out*/ HitScreenLocation, /*bPlayerViewportRelative=*/ false))
            {
                FGameScreenSpaceHitLocation& Entry = NewUnconfirmedHitMarker.Markers.AddDefaulted_GetRef();
                Entry.Location = HitScreenLocation;
                Entry.bShowAsSuccess = ShouldShowHitAsSuccess(Hit);

                // Determine the hit zone
                if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(Hit.PhysMaterial.Get()))
                {
                    for (const FGameplayTag MaterialTag : PhysMatWithTags->Tags)
                    {
                        if (MaterialTag.MatchesTag(TAG_Gameplay_Zone))
                        {
                            Entry.HitZone = MaterialTag;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void UUR_WeaponStateComponent::UpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext)
{
    if (ShouldUpdateDamageInstigatedTime(EffectContext))
    {
        ActuallyUpdateDamageInstigatedTime();
    }
}

void UUR_WeaponStateComponent::ActuallyUpdateDamageInstigatedTime()
{
    // If our LastWeaponDamageInstigatedTime was not very recent, clear our LastWeaponDamageScreenLocations array
    UWorld* World = GetWorld();
    if (World->GetTimeSeconds() - LastWeaponDamageInstigatedTime > 0.1)
    {
        LastWeaponDamageScreenLocations.Reset();
    }
    LastWeaponDamageInstigatedTime = World->GetTimeSeconds();
}

double UUR_WeaponStateComponent::GetTimeSinceLastHitNotification() const
{
    UWorld* World = GetWorld();
    return World->TimeSince(LastWeaponDamageInstigatedTime);
}
