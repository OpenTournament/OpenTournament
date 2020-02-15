// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Weapon.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include "OpenTournament.h"
#include "UR_Character.h"
#include "UR_InventoryComponent.h"
#include "UR_Projectile.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Weapon::AUR_Weapon(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Tbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
    Tbox->SetGenerateOverlapEvents(true);
    Tbox->OnComponentBeginOverlap.AddDynamic(this, &AUR_Weapon::OnTriggerEnter);
    Tbox->OnComponentEndOverlap.AddDynamic(this, &AUR_Weapon::OnTriggerExit);

    RootComponent = Tbox;

    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh1P"));
    Mesh1P->SetupAttachment(RootComponent);
    Mesh1P->bOnlyOwnerSee = true;

    Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh3P"));
    Mesh3P->SetupAttachment(RootComponent);
    Mesh3P->bOwnerNoSee = true;

    Sound = CreateDefaultSubobject<UAudioComponent>(TEXT("Sound"));
    Sound->SetupAttachment(RootComponent);

    ProjectileClass = AUR_Projectile::StaticClass();

    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    FireInterval = 1.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AUR_Weapon, AmmoCount, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(AUR_Weapon, bIsEquipped, COND_SkipOwner);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Weapon::BeginPlay()
{
    Super::BeginPlay();
    Sound->SetActive(false);
}

void AUR_Weapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_Weapon::CanFire() const
{
    return false;
}

void AUR_Weapon::Pickup()
{
    GAME_LOG(Game, Log, "Pickup Occurred");

    UGameplayStatics::PlaySoundAtLocation(this, PickupSound, PlayerController->GetActorLocation());

    PlayerController->InventoryComponent->Add(this);
    AttachWeaponToPawn();
}

void AUR_Weapon::GiveTo(AUR_Character* NewOwner)
{
    SetOwner(NewOwner);
    PlayerController = NewOwner;
    AttachWeaponToPawn();
    if (NewOwner && NewOwner->InventoryComponent)
    {
        NewOwner->InventoryComponent->Add(this);
    }

    //tmp - prevent Pickup() call
    Tbox->SetGenerateOverlapEvents(false);
}

void AUR_Weapon::OnRep_Owner()
{
    PlayerController = Cast<AUR_Character>(GetOwner());
    AttachWeaponToPawn();

    // In case Equipped was replicated before Owner
    OnRep_Equipped();
}

void AUR_Weapon::OnRep_Equipped()
{
    if (!PlayerController)
        return;	// owner not replicated yet

    if (PlayerController->IsLocallyControlled())
        return;	// should already be attached locally

    SetEquipped(bIsEquipped);
}

void AUR_Weapon::Fire()
{
    GAME_LOG(Game, Log, "Fire Weapon");

    if (auto World = GetWorld())
    {
        FVector MuzzleLocation{};
        FRotator MuzzleRotation{};

        if (AmmoCount > 0)
        {
            FActorSpawnParameters ProjectileSpawnParameters;

            AUR_Projectile* Projectile = World->SpawnActor<AUR_Projectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ProjectileSpawnParameters);

            UGameplayStatics::PlaySoundAtLocation(this, FireSound, PlayerController->GetActorLocation());

            GAME_LOG(Game, Log, "Fire Occurred");

            if (Projectile)
            {
                FVector Direction = MuzzleRotation.Vector();
                Projectile->FireAt(Direction);
                AmmoCount--;
            }
        }
        else
        {
            GAME_PRINT(1.f, FColor::Red, "Ammo Expended for %s", *WeaponName);
        }
    }
}

void AUR_Weapon::GetPlayer(AActor* Player)
{
    PlayerController = Cast<AUR_Character>(Player);
}

void AUR_Weapon::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor * Other, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    bItemIsWithinRange = true;
    GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("Press E to Pickup %s"), *WeaponName));
    GetPlayer(Other);
}

void AUR_Weapon::OnTriggerExit(UPrimitiveComponent* HitComp, AActor * Other, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
    bItemIsWithinRange = false;
}

EWeaponState::Type AUR_Weapon::GetCurrentState() const
{
    return EWeaponState::Type();
}

int32 AUR_Weapon::GetCurrentAmmo() const
{
    return AmmoCount;
}

int32 AUR_Weapon::GetMaxAmmo() const
{
    return int32();
}

USkeletalMeshComponent* AUR_Weapon::GetWeaponMesh() const
{
    return Mesh1P;
}

AUR_Character * AUR_Weapon::GetPawnOwner() const
{
    return Cast<AUR_Character>(GetOwner());
}

bool AUR_Weapon::IsLocallyControlled() const
{
    APawn* P = Cast<APawn>(GetOwner());
    return P && P->IsLocallyControlled();
}

void AUR_Weapon::AttachMeshToPawn()
{
    this->SetActorHiddenInGame(false);

    if (PlayerController)
    {
        // Remove and hide both first and third person meshes
        DetachMeshFromPawn();

        /*
        // For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
        FName AttachPoint = PlayerController->GetWeaponAttachPoint();
        if (PlayerController->IsLocallyControlled())
        {
            USkeletalMeshComponent* PawnMesh1p = PlayerController->GetSpecifcPawnMesh(true);
            USkeletalMeshComponent* PawnMesh3p = PlayerController->GetSpecifcPawnMesh(false);
            Mesh1P->SetHiddenInGame(false);
            Mesh3P->SetHiddenInGame(false);
            Mesh1P->AttachToComponent(PawnMesh1p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
            Mesh3P->AttachToComponent(PawnMesh3p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
        }
        else
        {
            USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
            USkeletalMeshComponent* UsePawnMesh = PlayerController->GetPawnMesh();
            UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
            UseWeaponMesh->SetHiddenInGame(false);
        }
        */

        //NOTE: For now, assume that owner(s) are always in 1P, and others always see char from a 3P perspective.
        // That means we will use bOwnerSee/bOwnerNoSee to handle visibility.

        // This will have to be reworked later.
        // Be aware that "owner" means not only the local player, but also anybody looking through character via ViewTarget.
        // And both spectators/localplayer might be in either 1P or 3P, so I believe we cannot rely on bOwnerSee/bOwnerNoSee for this.

        //TODO: See camera management in UR_Character.
        // Here we can use UR_Character::bViewingThirdPerson.

        Mesh1P->AttachToComponent(PlayerController->MeshFirstPerson, FAttachmentTransformRules::KeepRelativeTransform, PlayerController->GetWeaponAttachPoint());
        Mesh1P->SetHiddenInGame(false);

        //NOTE: We don't have proper anim and grip point for 3P weapon.
        //Mesh3P->AttachToComponent(PlayerController->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName(TEXT("ik_hand_gun")));
        //NOTE2: We'll attach to the (invisible) 1P arms for now otherwise it goes all over the place.
        Mesh3P->AttachToComponent(PlayerController->MeshFirstPerson, FAttachmentTransformRules::KeepRelativeTransform, PlayerController->GetWeaponAttachPoint());
        Mesh3P->SetHiddenInGame(false);
    }
}

void AUR_Weapon::AttachWeaponToPawn()
{
    this->SetActorHiddenInGame(true);
    Tbox->SetGenerateOverlapEvents(false);

    if (PlayerController)
    {
        /*
        // For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
        FName AttachPoint = PlayerController->GetWeaponAttachPoint();
        if (PlayerController->IsLocallyControlled() == true)
        {
            //USkeletalMeshComponent* PawnMesh = PlayerController->GetSpecifcPawnMesh(true);
            //this->AttachToComponent(PawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
        }
        else
        {
            //USkeletalMeshComponent* UsePawnMesh = PlayerController->GetPawnMesh();
            //this->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
        }
        */
    }
    this->SetActorHiddenInGame(true);
}

void AUR_Weapon::DetachMeshFromPawn()
{
    Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
    Mesh1P->SetHiddenInGame(true);

    Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
    Mesh3P->SetHiddenInGame(true);
}

void AUR_Weapon::OnEquip(AUR_Weapon * LastWeapon)
{
    LastWeapon->DetachMeshFromPawn();
    this->AttachMeshToPawn();
}

void AUR_Weapon::OnUnEquip()
{
    DetachMeshFromPawn();
}

bool AUR_Weapon::IsEquipped() const
{
    return bIsEquipped;
}

void AUR_Weapon::SetEquipped(bool bEquipped)
{
    bIsEquipped = bEquipped;

    if (bIsEquipped)
    {
        AttachMeshToPawn();

        if (AUR_Character* Char = Cast<AUR_Character>(GetOwner()))
        {
            if (Char->IsLocallyControlled() && Char->bIsFiring)
            {
                LocalStartFire();
            }
        }
    }
    else
    {
        DetachMeshFromPawn();
        LocalStopFire();
    }
}

bool AUR_Weapon::IsAttachedToPawn() const
{
    return false;
}

//============================================================
// Basic firing loop for basic fire mode.
//============================================================

void AUR_Weapon::LocalStartFire()
{
    bFiring = true;

    // Already firing or in cooldown
    if (FireLoopTimerHandle.IsValid())
        return;

    // Start fire loop
    LocalFireLoop();
}

void AUR_Weapon::LocalStopFire()
{
    //NOTE: Do not clear timer here, or repeated clicks will bypass fire interval.
    bFiring = false;
}

void AUR_Weapon::LocalFireLoop()
{
    //UKismetSystemLibrary::PrintString(this, TEXT("LocalFireLoop()"));

    FireLoopTimerHandle.Invalidate();

    // Here we stop the loop if player isn't firing anymore
    if (!bFiring)
        return;

    // Additional checks to stop firing automatically
    if (!PlayerController || !PlayerController->bIsFiring || !PlayerController->IsAlive() || !PlayerController->GetController() || !bIsEquipped)
    {
        bFiring = false;
        return;
    }

    // Not sure what this is
    //if (!CanFire())
        //return;

    if (AmmoCount <= 0)
    {
        // Play out-of-ammo sound ?
        GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("%s out of ammo"), *WeaponName));
        // Auto switch weapon ?
        return;
    }

    LocalFire();

    GetWorld()->GetTimerManager().SetTimer(FireLoopTimerHandle, this, &AUR_Weapon::LocalFireLoop, FireInterval, false);
}

void AUR_Weapon::LocalFire()
{
    ServerFire();
    PlayFireEffects();
    LocalFireTime = GetWorld()->GetTimeSeconds();
}

void AUR_Weapon::ServerFire_Implementation()
{
    //if (!CanFire())
        //return;

    // No ammo, discard this shot
    if (AmmoCount <= 0)
    {
        return;
    }

    // Client asking to fire while not equipped
    // Could be a slightly desynced swap, try to delay a bit
    if (!bIsEquipped)
    {
        FTimerDelegate TimerCallback;
        TimerCallback.BindLambda([this]
        {
            if (bIsEquipped)
                ServerFire_Implementation();
        });
        GetWorld()->GetTimerManager().SetTimer(DelayedFireTimerHandle, TimerCallback, 0.1f, false);
        return;
    }

    // Check if client is asking us to fire too early
    float Delay = FireInterval - GetWorld()->TimeSince(LastFireTime);
    if (Delay > 0.0f)
    {
        if (Delay > FMath::Min(0.200f, FireInterval / 2.f))
            return;	// discard this shot

        // Delay a bit and fire
        GetWorld()->GetTimerManager().SetTimer(DelayedFireTimerHandle, this, &AUR_Weapon::ServerFire_Implementation, Delay, false);
        return;
    }

    SpawnShot();
    LastFireTime = GetWorld()->GetTimeSeconds();
    ConsumeAmmo();
    MulticastFired();
}

void AUR_Weapon::SpawnShot()
{
    if (ProjectileClass)
    {
        SpawnShot_Projectile();
    }
    else
    {
        UKismetSystemLibrary::PrintString(this, TEXT("SpawnShot() not implemented"));
    }
}

void AUR_Weapon::ConsumeAmmo()
{
    AmmoCount -= 1;
}

void AUR_Weapon::MulticastFired_Implementation()
{
    if (!IsNetMode(NM_Client))
        return;

    if (PlayerController && PlayerController->IsLocallyControlled())
    {
        // Server just fired, adjust our fire loop accordingly
        float FirePing = GetWorld()->TimeSince(LocalFireTime);
        float Delay = FireInterval - FirePing / 2.f;
        if (Delay > 0.0f)
            GetWorld()->GetTimerManager().SetTimer(FireLoopTimerHandle, this, &AUR_Weapon::LocalFireLoop, Delay, false);
        else
            LocalFireLoop();
    }
    else
    {
        PlayFireEffects();
    }
}

void AUR_Weapon::PlayFireEffects()
{
    //TODO: Play muzzle flash
    //TODO: Play fire sound

    if (PlayerController && PlayerController->MeshFirstPerson)
    {
        PlayerController->MeshFirstPerson->PlayAnimation(PlayerController->FireAnimation, false);
        //TODO: play 3p anim when we have one
    }
}

//============================================================
// Helpers
//============================================================

void AUR_Weapon::GetFireVector(FVector& FireLoc, FRotator& FireRot)
{
    if (PlayerController)
    {
        // Careful, in URCharacter we are using a custom 1p camera.
        // This means GetActorEyesViewPoint is wrong because it uses a hardcoded offest.
        // Either access camera directly, or override GetActorEyesViewPoint.
        FVector CameraLoc = PlayerController->CharacterCameraComponent->GetComponentLocation();
        FireLoc = CameraLoc;
        FireRot = PlayerController->GetViewRotation();

        if (ProjectileClass)
        {
            // Use centered projectiles as it is a lot simpler with less edge cases.
            FireLoc += FireRot.Vector() * PlayerController->MuzzleOffset.Size();	//TODO: muzzle offset should be part of weapon, not character

            // Avoid spawning projectile within/behind geometry because of the offset.
            FCollisionQueryParams TraceParams(FCollisionQueryParams::DefaultQueryParam);
            TraceParams.AddIgnoredActor(this);
            TraceParams.AddIgnoredActor(PlayerController);
            FHitResult Hit;
            if (GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, FireLoc, ECollisionChannel::ECC_Visibility, TraceParams))
            {
                FireLoc = Hit.Location;
            }
        }
        else
        {
            // For hitscan, use straight line from camera to crosshair.

            // Muzzle offset should be used only to adjust the fire effect (beam) start loc.
        }
    }
    else
    {
        GetActorEyesViewPoint(FireLoc, FireRot);
    }
}

void AUR_Weapon::SpawnShot_Projectile()
{
    FVector FireLoc;
    FRotator FireRot;
    GetFireVector(FireLoc, FireRot);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = GetInstigator() ? GetInstigator() : Cast<APawn>(GetOwner());
    //SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AUR_Projectile* Projectile = GetWorld()->SpawnActor<AUR_Projectile>(ProjectileClass, FireLoc, FireRot, SpawnParams);
    if (Projectile)
    {
        Projectile->FireAt(FireRot.Vector());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to spawn projectile ??"));
    }
}