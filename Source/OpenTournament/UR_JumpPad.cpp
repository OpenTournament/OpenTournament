// Copyright 2019 Open Tournament Project, All Rights Reserved.

#include "UR_JumpPad.h"
#include "OpenTournament.h"

#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystemComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "AutomationTest.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
AUR_JumpPad::AUR_JumpPad(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    Destination(FTransform()),
    bLockDestination(true),
    JumpTime(2.f),
    JumpPadLaunchSound(nullptr),
    bUseJumpPadMaterialInstance(true),
    JumpPadMaterialInstance(nullptr),
    JumpPadMaterialIndex(0),
    JumpPadMaterialParameterName("Color")
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    SceneRoot = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
    RootComponent = SceneRoot;

    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetCapsuleSize(55.f, 55.f, false);
    CapsuleComponent->SetupAttachment(RootComponent);
    
    //SetRootComponent(CapsuleComponent);
    CapsuleComponent->SetGenerateOverlapEvents(true);
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

// Called when the game starts or when spawned
void AUR_JumpPad::BeginPlay()
{
    Super::BeginPlay();

    InitializeDynamicMaterialInstance();
}

// Called every frame
void AUR_JumpPad::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AUR_JumpPad::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    const auto TargetCharacter{ Cast<ACharacter>(Other) };

    if (Other->GetActorLocation().Z > CapsuleComponent->GetComponentTransform().GetLocation().Z)
    {
        if (IsPermittedToJump(TargetCharacter))
        {
            GAME_LOG(Game, Log, "Entered JumpPad (%s)", *GetName());

            if (TargetCharacter)
            {
                TargetCharacter->LaunchCharacter(CalculateJumpVelocity(TargetCharacter), true, true);
                PlayJumpPadEffects();
            }
        }
    }
}

void AUR_JumpPad::PlayJumpPadEffects_Implementation()
{
    if (JumpPadLaunchSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), JumpPadLaunchSound, GetActorLocation());
    }
}

bool AUR_JumpPad::IsPermittedToJump_Implementation(const AActor* InCharacter) const
{
    return true;
}

FVector AUR_JumpPad::CalculateJumpVelocity(const AActor* InCharacter)
{
    float Gravity = GetWorld()->GetGravityZ();
    FVector TargetVector = (GetActorLocation() + Destination.GetLocation()) - InCharacter->GetActorTransform().GetLocation();

    float SizeXY = TargetVector.Size2D() / JumpTime;
    float SizeZ = TargetVector.Z / JumpTime - Gravity * JumpTime / 2.0f;

    return TargetVector.GetSafeNormal2D() * SizeXY + FVector::UpVector * SizeZ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_JumpPad::InitializeDynamicMaterialInstance()
{
    if (MeshComponent && bUseJumpPadMaterialInstance)
    {
        UMaterialInterface* Material = MeshComponent->GetMaterial(JumpPadMaterialIndex);
        JumpPadMaterialInstance =  UMaterialInstanceDynamic::Create(Material, NULL);
        JumpPadMaterialInstance->SetVectorParameterValue(JumpPadMaterialParameterName, JumpPadMaterialColorBase);
        MeshComponent->SetMaterial(JumpPadMaterialIndex, JumpPadMaterialInstance);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
// Workarounds to the editor FTransform widget not having a setting to work in world space
void AUR_JumpPad::EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    FTransform CachedDestination = Destination * ActorToWorld();
    Super::EditorApplyTranslation(DeltaTranslation, bAltDown, bShiftDown, bCtrlDown);

    if (bLockDestination)
    {
        Destination = CachedDestination;
        Destination.SetToRelativeTransform(ActorToWorld());
    }
}

void AUR_JumpPad::EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    FTransform CachedDestination = Destination * ActorToWorld();
    Super::EditorApplyRotation(DeltaRotation, bAltDown, bShiftDown, bCtrlDown);

    if (bLockDestination)
    {
        Destination = CachedDestination;
        Destination.SetToRelativeTransform(ActorToWorld());
    }
}

void AUR_JumpPad::EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    FTransform CachedDestination = Destination * ActorToWorld();
    Super::EditorApplyScale(DeltaScale, PivotLocation, bAltDown, bShiftDown, bCtrlDown);

    if (bLockDestination)
    {
        Destination = CachedDestination;
        Destination.SetToRelativeTransform(ActorToWorld());
    }
}
#endif