// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_JumpPad.h"

#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "OpenTournament.h"
#include "UR_Character.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_JumpPad::AUR_JumpPad(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    Destination(FTransform()),
    bLockDestination(true),
    JumpTime(2.f),
    JumpPadLaunchSound(nullptr),
    bRequiredTagsExact(false),
    bExcludedTagsExact(true),
    bUseJumpPadMaterialInstance(true),
    JumpPadMaterialInstance(nullptr),
    JumpPadMaterialIndex(0),
    JumpPadMaterialParameterName("Color")
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    SetRootComponent(SceneRoot);

    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetCapsuleSize(55.f, 55.f, false);
    CapsuleComponent->SetupAttachment(RootComponent);
    CapsuleComponent->SetGenerateOverlapEvents(true);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);
    CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_JumpPad::OnTriggerEnter);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(RootComponent);

    Destination = GetActorTransform();
    Destination.SetLocation(Destination.GetLocation() + FVector(0, 0, 1000));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_JumpPad::BeginPlay()
{
    Super::BeginPlay();

    InitializeDynamicMaterialInstance();
}

void AUR_JumpPad::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* TargetCharacter{ Cast<ACharacter>(Other) };

    if (TargetCharacter && TargetCharacter->GetActorLocation().Z > CapsuleComponent->GetComponentTransform().GetLocation().Z)
    {
        if (IsPermittedToJump(TargetCharacter))
        {
            GAME_LOG(Game, Log, "Entered JumpPad (%s)", *GetName());

            TargetCharacter->LaunchCharacter(CalculateJumpVelocity(TargetCharacter), true, true);
            PlayJumpPadEffects();
        }
    }
}

void AUR_JumpPad::PlayJumpPadEffects_Implementation()
{
    if (JumpPadLaunchSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), JumpPadLaunchSound, GetActorLocation());

        if (auto PSComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), JumpPadLaunchParticleClass, GetActorTransform()))
        {
            // TODO : Modify PSComponent if needed, based on Team, etc.
        }
    }
}

bool AUR_JumpPad::IsPermittedToJump_Implementation(const AActor* TargetActor) const
{
    // @! TODO : Check to see if the component/actor overlapping here matches a LD-specifiable list of classes
    // (e.g. if we want to jump only characters, or if things such as projectiles, vehicles, etc. may also interact).
    const AUR_Character* Character = Cast<AUR_Character>(TargetActor);
    if (Character == nullptr)
    {
        GAME_LOG(Game, Log, "Teleporter Error. Character was invalid.");
        return false;
    }

    // Check if the actor being teleported has any Required or Excluded GameplayTags
    // e.g. Check for Red/Blue team tag, or exclude Flag-carrier tag, etc.
    FGameplayTagContainer TargetTags;
    Character->GetOwnedGameplayTags(TargetTags);
    return IsPermittedByGameplayTags(TargetTags);
}

bool AUR_JumpPad::IsPermittedByGameplayTags(const FGameplayTagContainer& TargetTags) const
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

FVector AUR_JumpPad::CalculateJumpVelocity(const AActor* InCharacter) const
{
    const float Gravity = GetWorld()->GetGravityZ();
    const FVector TargetVector = (GetActorLocation() + Destination.GetLocation()) - InCharacter->GetActorTransform().GetLocation();

    const float SizeXY = TargetVector.Size2D() / JumpTime;
    const float SizeZ = TargetVector.Z / JumpTime - Gravity * JumpTime / 2.0f;

    return TargetVector.GetSafeNormal2D() * SizeXY + FVector::UpVector * SizeZ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_JumpPad::InitializeDynamicMaterialInstance()
{
    if (MeshComponent && bUseJumpPadMaterialInstance)
    {
        UMaterialInterface* Material = MeshComponent->GetMaterial(JumpPadMaterialIndex);
        JumpPadMaterialInstance =  UMaterialInstanceDynamic::Create(Material, nullptr);
        JumpPadMaterialInstance->SetVectorParameterValue(JumpPadMaterialParameterName, JumpPadMaterialColorBase);
        MeshComponent->SetMaterial(JumpPadMaterialIndex, JumpPadMaterialInstance);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool AUR_JumpPad::CanEditChange(const UProperty* InProperty) const
{
    const bool ParentVal = Super::CanEditChange(InProperty);

    // Can we edit bRequiredTagsExact?
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AUR_JumpPad, bRequiredTagsExact))
    {
        return RequiredTags.Num() > 0;
    }

    // Can we edit bExcludedTagsExact?
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AUR_JumpPad, bExcludedTagsExact))
    {
        return ExcludedTags.Num() > 0;
    }

    return ParentVal;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
// Workarounds to the editor FTransform widget not having a setting to work in world space
void AUR_JumpPad::EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    const FTransform CachedDestination{ Destination * ActorToWorld() };
    Super::EditorApplyTranslation(DeltaTranslation, bAltDown, bShiftDown, bCtrlDown);

    if (bLockDestination)
    {
        Destination = CachedDestination;
        Destination.SetToRelativeTransform(ActorToWorld());
    }
}

void AUR_JumpPad::EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    const FTransform CachedDestination{ Destination * ActorToWorld() };
    Super::EditorApplyRotation(DeltaRotation, bAltDown, bShiftDown, bCtrlDown);

    if (bLockDestination)
    {
        Destination = CachedDestination;
        Destination.SetToRelativeTransform(ActorToWorld());
    }
}

void AUR_JumpPad::EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    const FTransform CachedDestination{ Destination * ActorToWorld() };
    Super::EditorApplyScale(DeltaScale, PivotLocation, bAltDown, bShiftDown, bCtrlDown);

    if (bLockDestination)
    {
        Destination = CachedDestination;
        Destination.SetToRelativeTransform(ActorToWorld());
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpenTournamentJumpPadTest, "OpenTournament.Feature.Levels.LevelFeatures.Actor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOpenTournamentJumpPadTest::RunTest(const FString& Parameters)
{
    // TODO : Automated Tests

    return true;
}

#endif WITH_DEV_AUTOMATION_TESTS
