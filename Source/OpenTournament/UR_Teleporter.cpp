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
#include "Materials/MaterialInstanceDynamic.h"
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
    TeleportActorClass(ACharacter::StaticClass()),
    TeleportOutSound(nullptr),
    TeleportInSound(nullptr),
    TeleporterEnabledSound(nullptr),
    TeleporterDisabledSound(nullptr),
    TeleportOutParticleSystemClass(nullptr),
    TeleportInParticleSystemClass(nullptr),
    TeleporterEnabledParticleSystemClass(nullptr),
    TeleporterDisabledParticleSystemClass(nullptr),
    bIsEnabled(true),
    bRequiredTagsExact(false),
    bExcludedTagsExact(true),
    TeleporterMaterialInstance(nullptr),
    TeleporterMaterialIndex(INDEX_NONE),
    TeleporterMaterialParameterName("Color"),
    TeleporterMaterialColorBase(FLinearColor(208.f, 160.f, 0.f, 1.f)),
    TeleporterMaterialColorEvent(FLinearColor(250.f, 250.f, 25.f, 1.f)),
    TeleporterMaterialColorInactive(FLinearColor(128.f, 128.f, 160.f, 1.f))
{
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetCapsuleSize(45.f, 90.f, false);
    SetRootComponent(CapsuleComponent);
    CapsuleComponent->SetGenerateOverlapEvents(true);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_Teleporter::OnTriggerEnter);
    CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &AUR_Teleporter::OnTriggerExit);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    ArrowComponent->SetupAttachment(CapsuleComponent);

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

    Teleport(Other);
}

void AUR_Teleporter::OnTriggerExit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (Other)
    {
        const auto ActorIndex = IgnoredActors.Find(Other);
        if (ActorIndex != INDEX_NONE)
        {
            IgnoredActors.RemoveAt(ActorIndex);
        }
    }
}

void AUR_Teleporter::Teleport(AActor* Other)
{
    // If not Enabled, do nothing
    if (!bIsEnabled)
    {
        return;
    }

    if (IsIgnoredActor(Other))
    {
        IgnoredActors.Remove(Other);
        return;
    }
    
    if (IsPermittedToTeleport(Other))
    {
        GAME_LOG(Game, Verbose, "Teleporter (%s) Triggered", *GetName());

        if (InternalTeleport(Other))
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
    if (!TargetActor->GetClass()->IsChildOf(TeleportActorClass))
    {
        return false;
    }

    if (const auto TagActor = Cast<IGameplayTagAssetInterface>(TargetActor))
    {
        // Check if the actor using the Teleporter has any Required or Excluded GameplayTags
        FGameplayTagContainer TargetTags;
        TagActor->GetOwnedGameplayTags(TargetTags);
        return IsPermittedByGameplayTags(TargetTags);
    }

    return true;
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

bool AUR_Teleporter::InternalTeleport(AActor* TargetActor)
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
            TargetActorRotation = Controller->GetControlRotation();
        }
    }
    else
    {
        TargetActorRotation = TargetActor->GetActorRotation();
    }

    // Find out Desired Rotation
    GetDesiredRotation(DesiredRotation, TargetActorRotation, DestinationRotation);

    if (AUR_Teleporter* DestinationTeleporter = Cast<AUR_Teleporter>(DestinationActor))
    {
        DestinationTeleporter->AddIgnoredActor(TargetActor);
    }
    
    // Try to Perform Our Teleport
    const bool bIsTeleportSuccessful{ TargetActor->TeleportTo(DestinationLocation, DestinationRotation) };
    
    if (bIsTeleportSuccessful)
    {
        // Play effects associated with teleportation
        PlayTeleportEffects();
        
        // If we successfully teleported, notify our actor.
        // We need to do this to update CharacterMovementComponent.bJustTeleported property
        // Which is used to update EyePosition
        // (Otherwise our EyePosition interpolates through the world)
        TargetActor->TeleportSucceeded(false);
    
        // Rotate velocity vector relative to the destination teleporter exit heading
        SetTargetVelocity(TargetActor, TargetCharacter, DesiredRotation, DestinationRotation);

        ApplyGameplayTag(TargetActor);
    }

    return bIsTeleportSuccessful;
}

void AUR_Teleporter::GetDesiredRotation(FRotator& DesiredRotation, const FRotator& TargetActorRotation, const FRotator& DestinationRotation) const
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

void AUR_Teleporter::ApplyGameplayTag(AActor* TargetActor)
{
    FGameplayTagContainer TargetTags;
    if (const auto TagActor = Cast<IGameplayTagAssetInterface>(TargetActor))
    {
        TagActor->GetOwnedGameplayTags(TargetTags);
        TargetTags.AddTag(TeleportTag);
    }
}

bool AUR_Teleporter::IsIgnoredActor(AActor* InActor) const
{
    return IgnoredActors.Find(InActor) != INDEX_NONE;
}

void AUR_Teleporter::AddIgnoredActor(AActor* InActor)
{
    IgnoredActors.AddUnique(InActor);
}

void AUR_Teleporter::SetTeleporterState(const bool bInEnabled)
{
    bInEnabled ? Enable() : Disable();
}

void AUR_Teleporter::Enable()
{
    CapsuleComponent->UpdateOverlaps();    
    bIsEnabled = true;
    
    TArray<AActor*> OverlappingActors;
    CapsuleComponent->GetOverlappingActors(OverlappingActors, TeleportActorClass);
    for (auto& OverlappingActor : OverlappingActors)
    {
        GAME_LOG(Game, Log, "Overlapping Actor (%s)", *OverlappingActor->GetName());
        Teleport(OverlappingActor);
    }

    if (TeleporterEnabledSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleporterEnabledSound, GetActorLocation());
    }

    if (TeleporterEnabledParticleSystemClass)
    {
        UGameplayStatics::SpawnEmitterAttached(TeleporterEnabledParticleSystemClass, RootComponent);
    }
    
    OnEnabled();
}

void AUR_Teleporter::OnEnabled_Implementation()
{
    AudioComponent->Play();
    ParticleSystemComponent->Activate();

    if (TeleporterMaterialInstance)
    {
        TeleporterMaterialInstance->SetVectorParameterValue(TeleporterMaterialParameterName, TeleporterMaterialColorInactive);
    }
}

void AUR_Teleporter::Disable()
{
    bIsEnabled = false;
    
    if (TeleporterDisabledSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleporterDisabledSound, GetActorLocation());
    }

    if (TeleporterEnabledParticleSystemClass)
    {
        UGameplayStatics::SpawnEmitterAttached(TeleporterDisabledParticleSystemClass, RootComponent);
    }
    
    OnDisabled();
}

void AUR_Teleporter::OnDisabled_Implementation()
{
    AudioComponent->Stop();
    ParticleSystemComponent->Deactivate();

    if (TeleporterMaterialInstance)
    {
        TeleporterMaterialInstance->SetVectorParameterValue(TeleporterMaterialParameterName, TeleporterMaterialColorBase);
    }
}

void AUR_Teleporter::SetTeleportDestination(const FTransform& InTransform)
{
    // If this function is called, we want to ensure that we teleport to the new Transform
    DestinationActor = nullptr;

    DestinationTransform = InTransform;
}

void AUR_Teleporter::SetTeleportDestinationActor(AActor* InActor)
{
    if (InActor)
    {
        DestinationActor = InActor;
    }
    else
    {
        GAME_LOG(Game, Warning, "Setting Teleporter (%s) DestinationActor to nullptr Actor!", *this->GetName());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Teleporter::InitializeDynamicMaterialInstance()
{
    if (MeshComponent && TeleporterMaterialIndex != INDEX_NONE)
    {
        if (UMaterialInterface* Material = MeshComponent->GetMaterial(TeleporterMaterialIndex))
        {
            TeleporterMaterialInstance =  UMaterialInstanceDynamic::Create(Material, nullptr);
            TeleporterMaterialInstance->SetVectorParameterValue(TeleporterMaterialParameterName, TeleporterMaterialColorBase);
            MeshComponent->SetMaterial(TeleporterMaterialIndex, TeleporterMaterialInstance);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool AUR_Teleporter::CanEditChange(const FProperty* InProperty) const
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

#endif // WITH_DEV_AUTOMATION_TESTS
