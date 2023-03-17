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
#include "NavLinkComponent.h"

#include "OpenTournament.h"
#include "UR_Character.h"
#include "AI/UR_NavigationUtilities.h"

#if WITH_EDITOR
#include "Components/SplineComponent.h"
#endif

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_JumpPad::AUR_JumpPad(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    Destination(FTransform()),
    bLockDestination(true),
    bRetainHorizontalVelocity(false),
    JumpActorClass(AUR_Character::StaticClass()),
    JumpDuration(2.f),
    JumpPadLaunchSound(nullptr),
    JumpPadLaunchParticleClass(nullptr),
    SplineProjectionDuration(2.f),
    bRequiredTagsExact(false),
    bExcludedTagsExact(true),
    bUseJumpPadMaterialInstance(true),
    JumpPadMaterialInstance(nullptr),
    JumpPadMaterialIndex(0),
    JumpPadMaterialParameterName("Color")
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    SetRootComponent(SceneRoot);

    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetCapsuleSize(55.f, 55.f, false);
    CapsuleComponent->SetupAttachment(RootComponent);
    CapsuleComponent->SetGenerateOverlapEvents(true);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_JumpPad::OnTriggerEnter);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(RootComponent);

    NavLink = CreateDefaultSubobject<UNavLinkComponent>("NavLink");
    NavLink->SetupAttachment(CapsuleComponent);
    NavLink->Links[0].Left = FVector::ZeroVector;
    NavLink->Links[0].Direction = ENavLinkDirection::LeftToRight;

    Destination = GetActorTransform();
    Destination.SetLocation(Destination.GetLocation() + FVector(0, 0, 1000));

#if WITH_EDITOR
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    SplineComponent->SetupAttachment(RootComponent);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_JumpPad::OnConstruction(const FTransform& Transform)
{
    NavLink->Links[0].Right = Destination.GetLocation();
}

void AUR_JumpPad::BeginPlay()
{
    Super::BeginPlay();

    InitializeDynamicMaterialInstance();

#if WITH_EDITOR
    UpdateSpline();
#endif
}

void AUR_JumpPad::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* TargetCharacter{ Cast<ACharacter>(Other) };

    if (TargetCharacter && TargetCharacter->GetActorLocation().Z > CapsuleComponent->GetComponentTransform().GetLocation().Z)
    {
        if (IsPermittedToJump(TargetCharacter))
        {
            GAME_LOG(Game, Log, "Entered JumpPad (%s)", *GetName());

            TargetCharacter->LaunchCharacter(CalculateJumpVelocity(TargetCharacter), !bRetainHorizontalVelocity, true);
            PlayJumpPadEffects();
            UUR_NavigationUtilities::ForceReachedDestinationWithin(TargetCharacter, HitComp->GetNavigationBounds());
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
    if (!TargetActor->GetClass()->IsChildOf(JumpActorClass))
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

bool AUR_JumpPad::IsPermittedByGameplayTags(const FGameplayTagContainer& TargetTags) const
{
    if (RequiredTags.Num() == 0 || (bRequiredTagsExact && TargetTags.HasAnyExact(RequiredTags)) || (!bRequiredTagsExact && TargetTags.HasAny(RequiredTags)))
    {
        return (ExcludedTags.Num() == 0 || (bExcludedTagsExact && !TargetTags.HasAnyExact(ExcludedTags)) || (!bExcludedTagsExact && TargetTags.HasAny(RequiredTags)));
    }
    return false;
}

FVector AUR_JumpPad::CalculateJumpVelocity(const AActor* InCharacter) const
{
    const float Gravity{ GetWorld()->GetGravityZ() };

    FVector CharacterLocation{ CapsuleComponent->GetComponentLocation() };
    if (InCharacter)
    {
        CharacterLocation = InCharacter->GetActorLocation();
    }

    const FVector WorldDestinationLocation{ ActorToWorld().TransformPosition(Destination.GetLocation()) };
    FVector TargetVector{ WorldDestinationLocation - CharacterLocation };

    const float SizeXY = TargetVector.Size2D() / JumpDuration;
    const float SizeZ = (TargetVector.Z / JumpDuration) - (Gravity * JumpDuration / 2.0f);
    TargetVector = TargetVector.GetSafeNormal2D() * SizeXY + FVector::UpVector * SizeZ;

    return TargetVector;
}

void AUR_JumpPad::SetDestination(const FVector InPosition, const bool IsRelativePosition)
{
    if (IsRelativePosition)
    {
        Destination.SetLocation(InPosition);
    }
    else
    {
        Destination.SetLocation(InPosition + CapsuleComponent->GetComponentLocation());
    }
}

#if WITH_EDITOR
void AUR_JumpPad::UpdateSpline() const
{
    SplineComponent->ClearSplinePoints();

    FPredictProjectilePathParams PredictProjectilePathParams{ 50.f, CapsuleComponent->GetComponentLocation(), CalculateJumpVelocity(nullptr), SplineProjectionDuration };
    FPredictProjectilePathResult PredictProjectilePathResult{ };

    UGameplayStatics::PredictProjectilePath(this, PredictProjectilePathParams, PredictProjectilePathResult);

    for (const auto& Point : PredictProjectilePathResult.PathData)
    {
        SplineComponent->AddSplineWorldPoint(Point.Location);
    }

    SplineComponent->UpdateSpline();
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_JumpPad::InitializeDynamicMaterialInstance()
{
    if (MeshComponent && bUseJumpPadMaterialInstance)
    {
        UMaterialInterface* Material = MeshComponent->GetMaterial(JumpPadMaterialIndex);
        JumpPadMaterialInstance = UMaterialInstanceDynamic::Create(Material, nullptr);
        JumpPadMaterialInstance->SetVectorParameterValue(JumpPadMaterialParameterName, JumpPadMaterialColorBase);
        MeshComponent->SetMaterial(JumpPadMaterialIndex, JumpPadMaterialInstance);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool AUR_JumpPad::CanEditChange(const FProperty* InProperty) const
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

void AUR_JumpPad::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropertyName = PropertyChangedEvent.Property->GetFName();

    if (PropertyName == GET_MEMBER_NAME_CHECKED(AUR_JumpPad, JumpDuration))
    {
        SplineProjectionDuration = JumpDuration;
        UpdateSpline();
    }
    
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AUR_JumpPad, Destination)
        || PropertyName == GET_MEMBER_NAME_CHECKED(AUR_JumpPad, SplineProjectionDuration))
    {
        UpdateSpline();
    }
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

    UpdateSpline();
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

#endif // WITH_DEV_AUTOMATION_TESTS
