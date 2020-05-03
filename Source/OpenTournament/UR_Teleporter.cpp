// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Teleporter.h"

#include "Components/ArrowComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "OpenTournament.h"
#include "UR_Character.h"
#include "UR_CharacterMovementComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Teleporter::AUR_Teleporter(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    DestinationActor(nullptr),
    ExitRotationType(EExitRotation::Relative),
    bKeepMomentum(true),
    TeleportOutSound(nullptr),
    TeleportInSound(nullptr),
    bRequiredTagsExact(false),
    bExcludedTagsExact(true)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetCapsuleSize(45.f, 90.f, false);
    SetRootComponent(CapsuleComponent);
    CapsuleComponent->SetGenerateOverlapEvents(true);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_Teleporter::OnTriggerEnter);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

#if WITH_EDITORONLY_DATA
    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    ArrowComponent->SetupAttachment(CapsuleComponent);
#endif

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(CapsuleComponent);

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(RootComponent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Teleporter::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // @! TODO(Pedro): we should store the "teleporting" state in the MovementComponent of the actor in order to query it here
    const bool bIsTeleporting = (bFromSweep == false);

    if (bIsTeleporting)
    {
        return;
    }

    if (IsPermittedToTeleport(Other))
    {
        GAME_LOG(Game, Verbose, "Teleporter (%s) Triggered", *GetName());

        if (PerformTeleport(Other))
        {
            GAME_LOG(Game, Log, "Teleport of Character (%s) succeeded", *Other->GetName());
        }
        else
        {
            GAME_LOG(Game, Warning, "Teleport of Character (%s) failed", *Other->GetName());
        }
    }
}

bool AUR_Teleporter::IsPermittedToTeleport_Implementation(const AActor* TargetActor) const
{
    // @! TODO : Check to see if the component/actor overlapping here matches a LD-specifiable list of classes
    // (e.g. if we want to teleport only characters, or if things such as projectiles, vehicles, etc. may also pass through).
    // This function may also be overridden to determine conditions such as only characters of Red/Blue team may pass through
    const AUR_Character* Character = Cast<AUR_Character>(TargetActor);
    if (Character == nullptr)
    {
        GAME_LOG(Game, Log, "Teleporter Error. Character was invalid.");
        return false;
    }

    // Check if the actor being teleported has any Required or Excluded GameplayTags
    FGameplayTagContainer TargetTags;
    Character->GetOwnedGameplayTags(TargetTags);
    return IsPermittedByGameplayTags(TargetTags);
}

bool AUR_Teleporter::IsPermittedByGameplayTags(const FGameplayTagContainer& TargetTags) const
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

bool AUR_Teleporter::PerformTeleport(AActor* TargetActor)
{
    if (TargetActor == nullptr || (DestinationActor == nullptr && DestinationTransform.GetLocation() == FVector::ZeroVector))
    {
        return false;
    }

    const auto TargetCharacter{ Cast<ACharacter>(TargetActor) };

    const FVector DestinationLocation{ DestinationActor ? DestinationActor->GetActorLocation() : DestinationTransform.GetLocation() + GetActorLocation() };
    FRotator TargetActorRotation{ FRotator::ZeroRotator };
    const FRotator DestinationRotation{ DestinationActor ? DestinationActor->GetActorRotation() : DestinationTransform.GetRotation().Rotator() };
    FRotator DesiredRotation{ DestinationRotation };

    if (TargetCharacter)
    {
        if (const AController* Controller = TargetCharacter->GetController())
        {
            TargetActorRotation = TargetCharacter->GetController()->GetControlRotation();
        }
    }
    else
    {
        TargetActorRotation = TargetActor->GetActorRotation();
    }

    // Play effects associated with teleportation
    PlayTeleportEffects();

    // Move Actor to Destination actor
    if (TargetActor->SetActorLocation(DestinationLocation))
    {
        // If we successfully teleported, notify our actor.
        // We need to do this to update CharacterMovementComponent.bJustTeleported property
        // Which is used to update EyePosition
        // (Otherwise our EyePosition interpolates through the world)
        TargetActor->TeleportSucceeded(false);
    }

    // Find out Desired Rotation
    GetDesiredRotation(DesiredRotation, TargetActorRotation, DestinationRotation);

    // Set the Desired Rotation
    SetTargetRotation(TargetActor, TargetCharacter, DesiredRotation);
    
    // Rotate velocity vector relative to the destination teleporter exit heading
    SetTargetVelocity(TargetActor, TargetCharacter, DesiredRotation, DestinationRotation);

    ApplyGameplayTag(TargetActor);

    return true;
}

void AUR_Teleporter::SetTargetRotation(AActor* TargetActor, ACharacter* TargetCharacter, const FRotator& DesiredRotation)
{
    // Rotate the TargetActor to face the Exit Direction vector
    if (TargetCharacter)
    {
        TargetCharacter->GetController()->SetControlRotation(DesiredRotation);
    }
    else
    {
        TargetActor->SetActorRotation(DesiredRotation);
    }
}

void AUR_Teleporter::SetTargetVelocity(AActor* TargetActor, ACharacter* TargetCharacter, const FRotator& DesiredRotation, const FRotator& DestinationRotation)
{
    UPawnMovementComponent* CharacterMovement{ TargetCharacter->GetMovementComponent() };

    if (!bKeepMomentum)
    {
        if (TargetCharacter)
        {
            CharacterMovement->Velocity = FVector::ZeroVector;
        }
        else
        {
            TargetActor->GetRootComponent()->ComponentVelocity = FVector::ZeroVector;
        }
    }
    else
    {
        if (ExitRotationType == EExitRotation::Relative)
        {
            // Rotate velocity vector relatively to the Exit Direction of the Destination actor
            FRotator MomentumRotator = DesiredRotation - GetRootComponent()->GetComponentRotation();
            MomentumRotator.Yaw = FMath::UnwindDegrees(MomentumRotator.Yaw + 180);

            if (TargetCharacter)
            {
                const FVector NewTargetVelocity{ MomentumRotator.RotateVector(CharacterMovement->Velocity) };
                CharacterMovement->Velocity = NewTargetVelocity;
            }
            else
            {
                const FVector NewTargetVelocity{ MomentumRotator.RotateVector(TargetActor->GetRootComponent()->ComponentVelocity) };
                TargetActor->GetRootComponent()->ComponentVelocity = NewTargetVelocity;
            }
        }
        else if (ExitRotationType == EExitRotation::Fixed)
        {
            // Rotate velocity vector to face the Exit Direction of the Destination actor
            if (TargetCharacter)
            {
                auto NewTargetVelocity = DestinationRotation.RotateVector(FVector::ForwardVector * CharacterMovement->Velocity.Size2D());
                NewTargetVelocity.Z = CharacterMovement->Velocity.Z;
                CharacterMovement->Velocity = NewTargetVelocity;
            }
            else
            {
                auto NewTargetVelocity = DestinationRotation.RotateVector(FVector::ForwardVector * TargetActor->GetRootComponent()->ComponentVelocity.Size2D());
                NewTargetVelocity.Z = TargetActor->GetRootComponent()->ComponentVelocity.Z;
                TargetActor->GetRootComponent()->ComponentVelocity = NewTargetVelocity;
            }
        }
    }
}

void AUR_Teleporter::ApplyGameplayTag(AActor* TargetActor)
{
    FGameplayTagContainer TargetTags;
    if (const auto TagActor = Cast<IGameplayTagAssetInterface>(TargetActor))
    {
        TagActor->GetOwnedGameplayTags(TargetTags);
        TargetTags.AddTag(TeleportTag);
    }
}

void AUR_Teleporter::PlayTeleportEffects_Implementation()
{
    if (TeleportOutSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportOutSound, GetActorLocation());
    }

    if (TeleportOutParticleSystemClass)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TeleportOutParticleSystemClass, CapsuleComponent->GetComponentTransform());
    }

    const FVector DestinationLocation{ DestinationActor ? DestinationActor->GetActorLocation() : DestinationTransform.GetLocation() + GetActorLocation() };
    if (TeleportInSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportInSound, DestinationLocation);
    }

    if (TeleportInParticleSystemClass)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TeleportInParticleSystemClass, DestinationLocation, FRotator(), FVector(1.f, 1.f, 1.f));
    }
}

void AUR_Teleporter::GetDesiredRotation(FRotator& DesiredRotation, const FRotator& TargetActorRotation, const FRotator& DestinationRotation)
{
    if (ExitRotationType == EExitRotation::Relative)
    {
        DesiredRotation = DestinationRotation + TargetActorRotation - this->GetActorRotation();
        DesiredRotation.Yaw += 180;
    }
    else
    {
        DesiredRotation = DestinationRotation;
    }

    DesiredRotation.Yaw = FMath::UnwindDegrees(DesiredRotation.Yaw);
    DesiredRotation.Pitch = 0.0f;
    DesiredRotation.Roll = 0.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool AUR_Teleporter::CanEditChange(const UProperty* InProperty) const
{
    const bool ParentVal = Super::CanEditChange(InProperty);

    // Can we edit bRequiredTagsExact?
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AUR_Teleporter, bRequiredTagsExact))
    {
        return RequiredTags.Num() > 0;
    }

    // Can we edit bExcludedTagsExact?
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AUR_Teleporter, bExcludedTagsExact))
    {
        return ExcludedTags.Num() > 0;
    }

    // Can we edit DestinationTransform?
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AUR_Teleporter, DestinationTransform))
    {
        return DestinationActor == nullptr;
    }

    return ParentVal;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpenTournamentTeleporterTest, "OpenTournament.Feature.Levels.LevelFeatures.Actor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOpenTournamentTeleporterTest::RunTest(const FString& Parameters)
{
    // TODO : Automated Tests

    return true;
}

#endif WITH_DEV_AUTOMATION_TESTS