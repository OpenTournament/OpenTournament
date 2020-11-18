// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PickupFactory.h"

#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

#include "UR_FunctionLibrary.h"
#include "UR_Pickup.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* NOTES about rotating movement :
*
* Could use RotatingMovementComponent, but I rather cut the overhead by doing directly what we want.
*
* Finished maps can have a lot of pickups in them... weapons, healths, armors, powerups, vials...
* I have seen some UT maps with many rotating pickups, where it had a significant impact on performance.
*
* We should :
* - minimize that impact as much as possible
* - even better, provide a configurable option to disable rotating pickups on client, so we can disable Tick altogether in this class.
*/

AUR_PickupFactory::AUR_PickupFactory()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bAllowTickOnDedicatedServer = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
    //PrimaryActorTick.bRunOnAnyThread = true;	//CRASH?

    //NOTE: Consider using TickInterval to improve performance.
    // Rotating pickup might not need to update any faster than 60hz,
    // so if player is running at higher FPS some frames could be entirely skipped.

    SetReplicates(true);
    SetReplicatingMovement(false);

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    //NOTE: Cannot point to RootComponent here or it is impossible to override in BP construction script.
    AttachComponent = NULL;

    EditorPreview = CreateEditorOnlyDefaultSubobject<UStaticMeshComponent>(TEXT("EditorPreview"), true);
    EditorPreview->SetupAttachment(RootComponent);
    EditorPreview->SetHiddenInGame(true);

    RotationRate = 180;
    BobbingHeight = 0;
    BobbingSpeed = 1.0f;

    InitialSpawnDelay = 0;
    RespawnTime = 5;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PickupFactory::OnConstruction(const FTransform& Transform)
{
    // This is called just after construction script.
    RespawnTime = FMath::Max(0.0f, RespawnTime);
    PreRespawnEffectDuration = FMath::Clamp(PreRespawnEffectDuration, 0.0f, RespawnTime);

    if (!AttachComponent)
    {
        AttachComponent = RootComponent;
    }

    if (EditorPreview)
    {
        EditorPreview->AttachToComponent(AttachComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

        if (PickupClass)
        {
            if (auto CDO = PickupClass->GetDefaultObject<AUR_Pickup>())
            {
                if (CDO->StaticMesh)
                {
                    EditorPreview->SetStaticMesh(CDO->StaticMesh->GetStaticMesh());
                    EditorPreview->OverrideMaterials = CDO->StaticMesh->OverrideMaterials;
                    EditorPreview->SetRelativeScale3D(CDO->StaticMesh->GetRelativeScale3D());
                    SetupEditorPreview(EditorPreview);
                    return;
                }
            }
        }
        //else
        EditorPreview->SetStaticMesh(NULL);
        EditorPreview->OverrideMaterials.Empty();
        EditorPreview->SetRelativeScale3D(FVector(1, 1, 1));
        SetupEditorPreview(EditorPreview);
    }
}

void AUR_PickupFactory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AUR_PickupFactory, PickupClass, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(AUR_PickupFactory, Pickup, COND_None);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PickupFactory::BeginPlay()
{
    Super::BeginPlay();

    if (AttachComponent)
    {
        InitialRelativeLocation = AttachComponent->GetRelativeLocation();
    }

    if (!IsNetMode(NM_DedicatedServer) && AttachComponent && (RotationRate != 0.f || BobbingHeight != 0.f))
    {
        //TODO: configurable rotating pickups ?
        SetActorTickEnabled(true);
    }
    else
    {
        SetActorTickEnabled(false);
    }

    Reset();
}

void AUR_PickupFactory::Reset()
{
    if (HasAuthority())
    {
        if (Pickup)
        {
            Pickup->Destroy();
        }

        GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);

        BeginRespawnTimer(InitialSpawnDelay);
    }

    Super::Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PickupFactory::BeginRespawnTimer(float InRespawnTime)
{
    if (InRespawnTime > 0.f)
    {
        const float PreRespawnTime = InRespawnTime - PreRespawnEffectDuration;
        if (PreRespawnTime > 0.f)
        {
            GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AUR_PickupFactory::PreRespawnTimer, PreRespawnTime, false);
        }
        else
        {
            MulticastWillRespawn();
        }
    }
    else
    {
        // Force a frame to avoid pickup-loop problem
        GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
        RespawnTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AUR_PickupFactory::RespawnTimer);
        //TODO: Might want to come up with something better, as this will spam network heavily.
        // Eg. instant respawn under player's feet but without giving it to him unless he goes out and back in.
        // But that's harder to do with pickup as a separate class.
        // Also must consider when multiple players are standing on it.
    }
}

void AUR_PickupFactory::SpawnPickup()
{
    if (Pickup)
        Pickup->Destroy();

    auto ClassToSpawn = GetPickupClass();
    if (!ClassToSpawn)
        return;

    FTransform Transform = AttachComponent ? AttachComponent->GetComponentTransform() : GetTransform();

    //NOTE: Use deferred spawn so we can bind OnPickedUp event before actor is added to the scene.
    // Otherwise it can trigger during SpawnActor routine, and if pickup destroys itself, we just get NULL.

    Pickup = Cast<AUR_Pickup>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ClassToSpawn, Transform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this));

    if (!Pickup)
    {
        //??!?! :enragedrabbit:
        UE_LOG(LogTemp, Warning, TEXT("SpawnPickup Pickup is NULL !"));
        BeginRespawnTimer(RespawnTime);
        return;
    }

    Pickup->OnPickedUp.AddDynamic(this, &AUR_PickupFactory::OnPickupPickedUp);

    // Blueprint init properties here
    PreInitializePickup(Pickup, Transform);

    // Blueprint may decide to destroy pickup
    if (!Pickup)
        return;

    UGameplayStatics::FinishSpawningActor(Pickup, Transform);

    // FinishSpawning may trigger overlap + destroy
    if (!Pickup)
        return;

    if (AttachComponent)
    {
        //TBD: AttachmentRules
        Pickup->AttachToComponent(AttachComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

        // Attaching may trigger overlap + destroy
        if (!Pickup)
            return;
    }

    // Blueprint do scene adjustements here
    PostInitializePickup(Pickup);
}

void AUR_PickupFactory::OnRep_PickupClass()
{
    // It may happen that Pickup is replicated before PickupClass,
    // In which case we may want to refresh whatever preview we have.
    ShowPickupAvailable(Pickup ? true : false);
}

void AUR_PickupFactory::OnRep_Pickup()
{
    ShowPickupAvailable(Pickup ? true : false);
}

void AUR_PickupFactory::OnPickupPickedUp_Implementation(AUR_Pickup* Other, APawn* Recipient)
{
    if (Other == Pickup)
    {
        Pickup = NULL;
        BeginRespawnTimer(RespawnTime);
    }
}

void AUR_PickupFactory::PreRespawnTimer()
{
    MulticastWillRespawn();
}

void AUR_PickupFactory::MulticastWillRespawn_Implementation()
{
    if (!IsNetMode(NM_DedicatedServer))
    {
        PlayRespawnEffects();
    }

    if (HasAuthority())
    {
        if (PreRespawnEffectDuration > 0.f)
        {
            GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AUR_PickupFactory::RespawnTimer, PreRespawnEffectDuration, false);
        }
        else
        {
            RespawnTimer();
        }
    }
}

void AUR_PickupFactory::PlayRespawnEffects_Implementation()
{
    // Attach effect to RootComponent not AttachComponent so it doesn't get rotated/bobbed
    UUR_FunctionLibrary::SpawnEffectAttached(RespawnEffect, FTransform(FRotator(), InitialRelativeLocation, GetActorScale()), RootComponent, NAME_None, EAttachLocation::KeepRelativeOffset);

    UGameplayStatics::SpawnSoundAttached(RespawnSound, RootComponent, NAME_None, InitialRelativeLocation, EAttachLocation::KeepRelativeOffset);
}

void AUR_PickupFactory::ShowPickupAvailable_Implementation(bool bAvailable)
{
}

void AUR_PickupFactory::RespawnTimer()
{
    SpawnPickup();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PickupFactory::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ShouldSkipTick())
    {
        if (RotationRate > 0.0f)
        {
            AttachComponent->AddLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));
        }

        if (BobbingHeight > 0.0f)
        {
            FVector Loc(InitialRelativeLocation);
            Loc.Z += BobbingHeight * FMath::Sin(BobbingSpeed * PI * GetWorld()->TimeSeconds);
            AttachComponent->SetRelativeLocation(Loc);
        }
    }
}

// See MovementComponent.cpp @ 329
bool AUR_PickupFactory::ShouldSkipTick()
{
    if (!AttachComponent || !AttachComponent->IsVisible())
    {
        return true;
    }

    const float RenderTimeThreshold = 0.41f;
    UWorld* TheWorld = GetWorld();

    if (const UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(AttachComponent))
    {
        if (TheWorld->TimeSince(PrimitiveComp->GetLastRenderTime()) <= RenderTimeThreshold)
        {
            return false; // Rendered, don't skip it.
        }
    }

    // Most components used with movement components don't actually render, so check attached children render times.
    TArray<USceneComponent*> RotatingChildren;
    AttachComponent->GetChildrenComponents(true, RotatingChildren);
    for (auto Child : RotatingChildren)
    {
        if (const UPrimitiveComponent* PrimitiveChild = Cast<UPrimitiveComponent>(Child))
        {
            if (PrimitiveChild->IsRegistered() && TheWorld->TimeSince(PrimitiveChild->GetLastRenderTime()) <= RenderTimeThreshold)
            {
                return false; // Rendered, don't skip it.
            }
        }
    }

    // No children were recently rendered, safely skip the update.
    return true;
}
