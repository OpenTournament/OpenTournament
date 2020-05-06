// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Weapon.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
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
#include "Particles/ParticleSystemComponent.h"

#include "OpenTournament.h"
#include "UR_Character.h"
#include "UR_InventoryComponent.h"
#include "UR_Projectile.h"
#include "UR_PlayerController.h"
#include "UR_FunctionLibrary.h"

#include "UR_FireModeBasic.h"
#include "UR_FireModeCharged.h"
#include "UR_FireModeContinuous.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Weapon::AUR_Weapon(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetGenerateOverlapEvents(false);
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AUR_Weapon::OnTriggerEnter);

    RootComponent = TriggerBox;

    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh1P"));
    Mesh1P->SetupAttachment(RootComponent);
    Mesh1P->bOnlyOwnerSee = true;

    Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh3P"));
    Mesh3P->SetupAttachment(RootComponent);

    //PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    BringUpTime = 0.3f;
    PutDownTime = 0.3f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AUR_Weapon, AmmoCount, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(AUR_Weapon, bIsEquipped, COND_SkipOwner);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Weapon::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    TArray<UUR_FireModeBase*> FireModeComponents;
    GetComponents<UUR_FireModeBase>(FireModeComponents);
    for (auto FireMode : FireModeComponents)
    {
        // sanity check
        if (FireModes.IsValidIndex(FireMode->Index) && FireModes[FireMode->Index])
        {
            GAME_PRINT(6.f, FColor::Red, "ERROR: %s has multiple firemodes with index %i", *GetName(), FireMode->Index);
        }
        else
        {
            if (!FireModes.IsValidIndex(FireMode->Index))
            {
                FireModes.SetNumZeroed(FireMode->Index + 1);
            }
            FireModes[FireMode->Index] = FireMode;

            FireMode->SetCallbackInterface(this);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Very, very basic support for picking up weapons on the ground.

void AUR_Weapon::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() && !GetOwner())
    {
        TriggerBox->SetGenerateOverlapEvents(true);
    }
}

void AUR_Weapon::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor * Other, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (HasAuthority())
    {
        if (AUR_Character* URChar = Cast<AUR_Character>(Other))
        {
            UGameplayStatics::PlaySoundAtLocation(this, PickupSound, URCharOwner->GetActorLocation());
            GiveTo(URChar);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Weapon possession

void AUR_Weapon::GiveTo(AUR_Character* NewOwner)
{
    TriggerBox->SetGenerateOverlapEvents(false);

    if (GetNetMode() != NM_DedicatedServer)
    {
        SetActorHiddenInGame(true);
    }

    SetOwner(NewOwner);
    URCharOwner = NewOwner;
    if (NewOwner && NewOwner->InventoryComponent)
    {
        NewOwner->InventoryComponent->Add(this);
    }
}

void AUR_Weapon::OnRep_Owner()
{
    URCharOwner = Cast<AUR_Character>(GetOwner());
    SetActorHiddenInGame(true);

    // In case Equipped was replicated before Owner
    OnRep_Equipped();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Some getters

bool AUR_Weapon::IsLocallyControlled() const
{
    APawn* P = Cast<APawn>(GetOwner());
    return P && P->IsLocallyControlled();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Equipping (placeholder)

void AUR_Weapon::SetEquipped(bool bEquipped)
{
    bIsEquipped = bEquipped;

    if (bIsEquipped)
    {
        AttachMeshToPawn();
        RequestBringUp();
    }
    else
    {
        SetWeaponState(EWeaponState::Inactive);
        DetachMeshFromPawn();
    }
}

void AUR_Weapon::OnRep_Equipped()
{
    if (!GetOwner())
        return;	// owner not replicated yet

    if (IsLocallyControlled())
        return;	// should already be attached locally

    SetEquipped(bIsEquipped);
}

void AUR_Weapon::AttachMeshToPawn()
{
    this->SetActorHiddenInGame(false);

    if (URCharOwner)
    {
        Mesh1P->AttachToComponent(URCharOwner->MeshFirstPerson, FAttachmentTransformRules::KeepRelativeTransform, URCharOwner->GetWeaponAttachPoint());
        //TODO: merge in Mogno's socket thing
        Mesh3P->AttachToComponent(URCharOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName(TEXT("ik_hand_gun")));

        UpdateMeshVisibility();
    }
}

void AUR_Weapon::UpdateMeshVisibility()
{
    if (UUR_FunctionLibrary::IsViewingFirstPerson(URCharOwner))
    {
        Mesh1P->SetHiddenInGame(false);
        Mesh3P->SetHiddenInGame(true);
    }
    else
    {
        Mesh1P->SetHiddenInGame(true);
        Mesh3P->SetHiddenInGame(false);
        Mesh3P->bOwnerNoSee = false;
    }
}

void AUR_Weapon::DetachMeshFromPawn()
{
    Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
    Mesh1P->SetHiddenInGame(true);

    Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
    Mesh3P->SetHiddenInGame(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

//============================================================
// WeaponStates Core
//============================================================

void AUR_Weapon::SetWeaponState(EWeaponState NewState)
{
    if (NewState != WeaponState)
    {
        WeaponState = NewState;
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("WeaponState: %s"), *UUR_FunctionLibrary::GetEnumValueAsString(TEXT("EWeaponState"), WeaponState)), true, false, FColor::Purple, 2.f);
        OnWeaponStateChanged.Broadcast(this, NewState);
    }

    switch (WeaponState)
    {

    case EWeaponState::BringUp:
        // On BringUp, read current desired fire mode from player
        if (IsLocallyControlled() && URCharOwner)
        {
            if (URCharOwner->DesiredFireModeNum.Num() > 0)
            {
                RequestStartFire(URCharOwner->DesiredFireModeNum[0]);
            }
        }
        break;

    case EWeaponState::Idle:
        if (GetWorld()->GetTimerManager().IsTimerActive(PutDownDelayTimerHandle))
        {
            // if cooldown delays putdown by 100%, the timer can be slightly late.
            // we can force it to happen now.
            GetWorld()->GetTimerManager().ClearTimer(PutDownDelayTimerHandle);
            RequestPutDown();
        }
        else if (DesiredFireModes.Num() > 0)
        {
            TryStartFire(DesiredFireModes[0]);
        }
        break;

    case EWeaponState::PutDown:
    case EWeaponState::Inactive:
        StopAllFire();
        break;

    }
}

void AUR_Weapon::BringUp(float FromPosition)
{
    GetWorld()->GetTimerManager().ClearTimer(SwapAnimTimerHandle);

    SetWeaponState(EWeaponState::BringUp);

    if (BringUpMontage && URCharOwner && URCharOwner->MeshFirstPerson && URCharOwner->MeshFirstPerson->GetAnimInstance())
    {
        float Duration = BringUpMontage->GetPlayLength();
        float PlayRate = Duration / BringUpTime;
        float StartTime = FromPosition * Duration;
        //TODO: not sure if start time accounts for the play rate or not. need check
        URCharOwner->MeshFirstPerson->GetAnimInstance()->Montage_Play(BringUpMontage, PlayRate, EMontagePlayReturnType::MontageLength, StartTime);
    }

    float Delay = (1.f - FromPosition)*BringUpTime;
    if (Delay > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(SwapAnimTimerHandle, this, &AUR_Weapon::BringUpCallback, Delay, false);
    }
    else
    {
        BringUpCallback();
    }
}

void AUR_Weapon::BringUpCallback()
{
    if (WeaponState == EWeaponState::BringUp)
    {
        // Weird use-case where weapon swaps are faster than cooldown times, firemode might still be busy
        if (CurrentFireMode && CurrentFireMode->IsBusy())
        {
            SetWeaponState(EWeaponState::Firing);
        }
        else
        {
            SetWeaponState(EWeaponState::Idle);
        }
    }
}

void AUR_Weapon::PutDown(float FromPosition)
{
    GetWorld()->GetTimerManager().ClearTimer(SwapAnimTimerHandle);

    SetWeaponState(EWeaponState::PutDown);

    if (PutDownMontage && URCharOwner && URCharOwner->MeshFirstPerson && URCharOwner->MeshFirstPerson->GetAnimInstance())
    {
        float Duration = PutDownMontage->GetPlayLength();
        float PlayRate = Duration / PutDownTime;
        float StartTime = FromPosition * Duration;
        //TODO: not sure if start time accounts for the play rate or not. need check
        URCharOwner->MeshFirstPerson->GetAnimInstance()->Montage_Play(PutDownMontage, PlayRate, EMontagePlayReturnType::MontageLength, StartTime);
    }

    float Delay = FromPosition * PutDownTime;
    if (Delay > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(SwapAnimTimerHandle, this, &AUR_Weapon::PutDownCallback, Delay, false);
    }
    else
    {
        PutDownCallback();
    }
}

void AUR_Weapon::PutDownCallback()
{
    if (WeaponState == EWeaponState::PutDown)
    {
        SetWeaponState(EWeaponState::Inactive);
    }
}

void AUR_Weapon::StopAllFire()
{
    DesiredFireModes.Empty();

    // this should be enough
    if (CurrentFireMode)
    {
        CurrentFireMode->StopFire();
    }

    /*
    // Normally, only CurrentFireMode should be firing.
    // But iterate anyways just to make sure.
    for (auto FireMode : FireModes)
    {
        if (FireMode && FireMode->IsBusy())
        {
            FireMode->StopFire();
        }
    }
    */
}


//============================================================
// External API
//============================================================

void AUR_Weapon::RequestStartFire(uint8 FireModeIndex)
{
    if (FireModes.IsValidIndex(FireModeIndex))
    {
        auto FireMode = FireModes[FireModeIndex];
        if (FireMode)
        {
            DesiredFireModes.Remove(FireMode);
            DesiredFireModes.Insert(FireMode, 0);
            TryStartFire(FireMode);
        }
    }
}

void AUR_Weapon::RequestStopFire(uint8 FireModeIndex)
{
    if (FireModes.IsValidIndex(FireModeIndex))
    {
        auto FireMode = FireModes[FireModeIndex];
        if (FireMode)
        {
            DesiredFireModes.Remove(FireMode);

            if (FireMode->IsBusy())
            {
                FireMode->StopFire();
            }
        }
    }
}

/**
* TODO: RequestPutDown()
*
* weapon swap procedure :
* - character requests inventory to swap
* - inventory requests weapon to putdown
* - weapon putdown when possible
* - weapon notify inventory when done (event dispatcher?)
* - inventory changes active weapon
* - inventory requests new weapon to bring up
*/

void AUR_Weapon::RequestBringUp()
{
    GetWorld()->GetTimerManager().ClearTimer(PutDownDelayTimerHandle);

    switch (WeaponState)
    {

    case EWeaponState::Inactive:
        BringUp(0.f);
        break;

    case EWeaponState::PutDown:
        BringUp(GetWorld()->GetTimerManager().GetTimerRemaining(SwapAnimTimerHandle) / PutDownTime);
        break;

    }
}

void AUR_Weapon::RequestPutDown()
{
    switch (WeaponState)
    {

    case EWeaponState::BringUp:
        PutDown(GetWorld()->GetTimerManager().GetTimerElapsed(SwapAnimTimerHandle) / BringUpTime);
        return;

    case EWeaponState::Idle:
        PutDown(1.f);
        return;

    case EWeaponState::Firing:
    {
        float Delay = 0.f;

        float CooldownStartTime = CurrentFireMode->GetCooldownStartTime();
        float CooldownRemaining = CurrentFireMode->GetTimeUntilIdle();

        // Bit of an edge case - if fire mode returns future cooldown start, this is an indication to prevent put down.
        // Used by charging firemode so we dont allow swap while charging, even if CooldownPercent is at 0.
        if (GetWorld()->TimeSince(CooldownStartTime) < 0.f)
        {
            Delay = FMath::Max(CooldownRemaining * CooldownDelaysPutDownByPercent, 0.1f);
        }
        else if (CooldownRemaining > 0.f && CooldownDelaysPutDownByPercent > 0.f)
        {
            float TotalCooldown = GetWorld()->TimeSince(CooldownStartTime) + CooldownRemaining;
            float TotalPutDownDelay = TotalCooldown * CooldownDelaysPutDownByPercent;
            if (bReducePutDownDelayByPutDownTime)
            {
                TotalPutDownDelay -= PutDownTime;
            }
            float PutDownStartTime = CooldownStartTime + TotalPutDownDelay;
            Delay = PutDownStartTime - GetWorld()->GetTimeSeconds();
        }

        if (Delay > 0.f)
        {
            // We call back RequestPutDown until delay is 0, and only then we will call PutDown.
            // This is because some fire modes may not have proper cooldown information at all times (eg. charging).
            // NOTE: this loop can be canceled anytime by a subsequent RequestBringUp() call.
            GetWorld()->GetTimerManager().SetTimer(PutDownDelayTimerHandle, this, &AUR_Weapon::RequestPutDown, Delay, false);
        }
        else
        {
            PutDown(1.f);
        }
        break;
    }

    case EWeaponState::Busy:
        // Stub. Just wait. SetWeaponState(Idle) will notice and cancel the loop, and call this back.
        GetWorld()->GetTimerManager().SetTimer(PutDownDelayTimerHandle, this, &AUR_Weapon::RequestPutDown, 1.f, false);
        break;

    }
}


//============================================================
// Helpers
//============================================================

void AUR_Weapon::TryStartFire(UUR_FireModeBase* FireMode)
{
    if (WeaponState == EWeaponState::Idle)
    {
        if (HasEnoughAmmoFor(FireMode))
        {
            FireMode->StartFire();
        }
        else
        {
            // Out of ammo
            UGameplayStatics::PlaySound2D(GetWorld(), OutOfAmmoSound);

            // loop as long as user is holding fire
            FTimerDelegate TimerCallback;
            TimerCallback.BindLambda([this, FireMode]
            {
                if (DesiredFireModes.Num() > 0 && DesiredFireModes[0] == FireMode)
                {
                    TryStartFire(FireMode);
                }
            });
            GetWorld()->GetTimerManager().SetTimer(RetryStartFireTimerHandle, TimerCallback, 0.5f, false);
            return;
        }
    }
}


void AUR_Weapon::GetFireVector(FVector& FireLoc, FRotator& FireRot)
{
    if (URCharOwner)
    {
        // Careful, in URCharacter we are using a custom 1p camera.
        // This means GetActorEyesViewPoint is wrong because it uses a hardcoded offest.
        // Either access camera directly, or override GetActorEyesViewPoint.
        FVector CameraLoc = URCharOwner->CharacterCameraComponent->GetComponentLocation();
        FireLoc = CameraLoc;
        FireRot = URCharOwner->GetViewRotation();
    }
    else
    {
        GetActorEyesViewPoint(FireLoc, FireRot);
    }
}

AUR_Projectile* AUR_Weapon::SpawnProjectile(TSubclassOf<AUR_Projectile> InProjectileClass, const FVector& StartLoc, const FRotator& StartRot)
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = GetInstigator() ? GetInstigator() : Cast<APawn>(GetOwner());
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AUR_Projectile* Projectile = GetWorld()->SpawnActor<AUR_Projectile>(InProjectileClass, StartLoc, StartRot, SpawnParams);
    if (Projectile)
    {
        Projectile->FireAt(StartRot.Vector());
        return Projectile;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to spawn projectile ??"));
    }

    return nullptr;
}

void AUR_Weapon::HitscanTrace(const FVector& TraceStart, const FVector& TraceEnd, FHitResult& OutHit)
{
    ECollisionChannel TraceChannel = ECollisionChannel::ECC_GameTraceChannel2;  //WeaponTrace
    FCollisionShape SweepShape = FCollisionShape::MakeSphere(5.f);

    // fill in info in case we get 0 results from sweep
    OutHit.TraceStart = TraceStart;
    OutHit.TraceEnd = TraceEnd;
    OutHit.bBlockingHit = false;
    OutHit.ImpactNormal = TraceStart - TraceEnd;
    OutHit.ImpactNormal.Normalize();

    TArray<FHitResult> Hits;
    GetWorld()->SweepMultiByChannel(Hits, TraceStart, TraceEnd, FQuat(), TraceChannel, SweepShape);
    for (const FHitResult& Hit : Hits)
    {
        if (Hit.bBlockingHit || HitscanShouldHitActor(Hit.GetActor()))
        {
            OutHit = Hit;
            OutHit.bBlockingHit = true;
            break;
        }
    }
}

bool AUR_Weapon::HitscanShouldHitActor_Implementation(AActor* Other)
{
    //NOTE: here we can implement firing through teammates
    if (APawn* Pawn = Cast<APawn>(Other))
    {
        return Pawn != GetInstigator();
    }
    else if (AUR_Projectile* Proj = Cast<AUR_Projectile>(Other))
    {
        return Proj->CanBeDamaged();
    }
    return false;
}

bool AUR_Weapon::HasEnoughAmmoFor(UUR_FireModeBase* FireMode)
{
    return AmmoCount >= 1;
}

void AUR_Weapon::ConsumeAmmo(UUR_FireModeBase* FireMode)
{
    AmmoCount -= 1;
}


//============================================================
// FireModeBase interface
//============================================================

void AUR_Weapon::FireModeChangedStatus_Implementation(UUR_FireModeBase* FireMode)
{
    if (FireMode->IsBusy())
    {
        CurrentFireMode = FireMode;
        SetWeaponState(EWeaponState::Firing);
    }
    else if (FireMode == CurrentFireMode)
    {
        CurrentFireMode = nullptr;
        if (WeaponState == EWeaponState::Firing)
        {
            SetWeaponState(EWeaponState::Idle);
        }
    }
}

float AUR_Weapon::TimeUntilReadyToFire_Implementation(UUR_FireModeBase* FireMode)
{
    float Delay;
    switch (WeaponState)
    {
    case EWeaponState::BringUp:
        Delay = GetWorld()->GetTimerManager().GetTimerRemaining(SwapAnimTimerHandle);
        break;

    case EWeaponState::Idle:
        Delay = 0.f;
        break;

    case EWeaponState::Firing:
        Delay = CurrentFireMode->GetTimeUntilIdle();
        break;

    default:
        Delay = 1.f;    //prevent
        break;
    }

    if (Delay <= 0.f && !HasEnoughAmmoFor(FireMode))
    {
        Delay = 1.f;    //prevent
    }

    return Delay;
}


//============================================================
// FireModeBasic interface
//============================================================

void AUR_Weapon::SimulateShot_Implementation(UUR_FireModeBasic* FireMode, FSimulatedShotInfo& OutSimulatedInfo)
{
    FVector FireLoc;
    FRotator FireRot;
    GetFireVector(FireLoc, FireRot);

    if (FireMode->ProjectileClass)
    {
        FVector MuzzleLoc = Mesh1P->GetSocketLocation(FireMode->MuzzleSocketName);
        FVector MuzzleOffset = MuzzleLoc - FireLoc;
        if (!MuzzleOffset.IsNearlyZero())
        {
            FVector OriginalFireLoc = FireLoc;

            // Offset projectile forward but stay centered
            FireLoc += FireRot.Vector() * MuzzleOffset.Size();

            // Avoid spawning projectile within/behind geometry because of the offset.
            FCollisionQueryParams TraceParams(FCollisionQueryParams::DefaultQueryParam);
            TraceParams.AddIgnoredActor(this);
            TraceParams.AddIgnoredActor(URCharOwner);
            FHitResult Hit;
            if (GetWorld()->LineTraceSingleByChannel(Hit, OriginalFireLoc, FireLoc, ECollisionChannel::ECC_Visibility, TraceParams))
            {
                FireLoc = Hit.Location;
            }
        }
    }

    OutSimulatedInfo.Vectors.EmplaceAt(0, FireLoc);
    OutSimulatedInfo.Vectors.EmplaceAt(1, FireRot.Vector());
}

void AUR_Weapon::SimulateHitscanShot_Implementation(UUR_FireModeBasic* FireMode, FSimulatedShotInfo& OutSimulatedInfo, FHitscanVisualInfo& OutHitscanInfo)
{
    FVector FireLoc;
    FRotator FireRot;
    GetFireVector(FireLoc, FireRot);

    OutSimulatedInfo.Vectors.EmplaceAt(0, FireLoc);
    OutSimulatedInfo.Vectors.EmplaceAt(1, FireRot.Vector());

    FVector TraceEnd = FireLoc + FireMode->HitscanTraceDistance * FireRot.Vector();

    FHitResult Hit;
    HitscanTrace(FireLoc, TraceEnd, Hit);

    OutHitscanInfo.Vectors.EmplaceAt(0, Hit.bBlockingHit ? Hit.Location : Hit.TraceEnd);
    OutHitscanInfo.Vectors.EmplaceAt(1, Hit.ImpactNormal);
}

void AUR_Weapon::AuthorityShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo)
{
    if (FireMode->ProjectileClass)
    {
        //TODO: validate passed in fire location - use server location if bad - needs a basic rewinding implementation to check
        FVector FireLoc = SimulatedInfo.Vectors[0];

        // Fire direction doesn't need validation
        const FVector& FireDir = SimulatedInfo.Vectors[1];

        SpawnProjectile(FireMode->ProjectileClass, FireLoc, FireDir.Rotation());

        ConsumeAmmo(FireMode);
    }
}

void AUR_Weapon::AuthorityHitscanShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo, FHitscanVisualInfo& OutHitscanInfo)
{
    //TODO: validate passed in start location
    FVector TraceStart = SimulatedInfo.Vectors[0];

    FVector FireDir = SimulatedInfo.Vectors[1];
    FireDir.Normalize();

    FVector TraceEnd = TraceStart + FireMode->HitscanTraceDistance * FireDir;

    FHitResult Hit;
    HitscanTrace(TraceStart, TraceEnd, Hit);

    if (Hit.bBlockingHit && Hit.GetActor())
    {
        float Damage = FireMode->HitscanDamage;
        auto DamType = FireMode->HitscanDamageType;
        UGameplayStatics::ApplyPointDamage(Hit.GetActor(), Damage, FireDir, Hit, GetInstigatorController(), this, DamType);
    }

    OutHitscanInfo.Vectors.EmplaceAt(0, Hit.bBlockingHit ? Hit.Location : Hit.TraceEnd);
    OutHitscanInfo.Vectors.EmplaceAt(1, Hit.ImpactNormal);

    ConsumeAmmo(FireMode);
}

void AUR_Weapon::PlayFireEffects_Implementation(UUR_FireModeBasic* FireMode)
{
    if (UUR_FunctionLibrary::IsViewingFirstPerson(URCharOwner))
    {
        UGameplayStatics::SpawnSoundAttached(FireMode->FireSound, Mesh1P, FireMode->MuzzleSocketName, FVector(0, 0, 0), EAttachLocation::SnapToTarget);
        UGameplayStatics::SpawnEmitterAttached(FireMode->MuzzleFlashFX, Mesh1P, FireMode->MuzzleSocketName, FVector(0, 0, 0), FRotator(0, 0, 0), EAttachLocation::SnapToTargetIncludingScale);
        if (URCharOwner->MeshFirstPerson && URCharOwner->MeshFirstPerson->GetAnimInstance())
        {
            //TODO: fire animation should be in weapon, maybe even in firemode?
            URCharOwner->MeshFirstPerson->GetAnimInstance()->Montage_Play(URCharOwner->FireAnimation);
        }
    }
    else
    {
        UGameplayStatics::SpawnSoundAttached(FireMode->FireSound, Mesh3P, FireMode->MuzzleSocketName, FVector(0, 0, 0), EAttachLocation::SnapToTarget);
        UGameplayStatics::SpawnEmitterAttached(FireMode->MuzzleFlashFX, Mesh3P, FireMode->MuzzleSocketName, FVector(0, 0, 0), FRotator(0, 0, 0), EAttachLocation::SnapToTargetIncludingScale);
        //TODO: play 3p anim
    }
}

void AUR_Weapon::PlayHitscanEffects_Implementation(UUR_FireModeBasic* FireMode, const FHitscanVisualInfo& HitscanInfo)
{
    FVector BeamStart;
    if (UUR_FunctionLibrary::IsViewingFirstPerson(URCharOwner))
    {
        BeamStart = Mesh1P->GetSocketLocation(FireMode->MuzzleSocketName);
    }
    else
    {
        BeamStart = Mesh3P->GetSocketLocation(FireMode->MuzzleSocketName);
    }

    const FVector& BeamEnd = HitscanInfo.Vectors[0];
    FVector BeamVector = BeamEnd - BeamStart;

    UFXSystemComponent* BeamComp = UUR_FunctionLibrary::SpawnEffectAtLocation(GetWorld(), FireMode->BeamTemplate, FTransform(BeamStart));
    if (BeamComp)
    {
        BeamComp->SetVectorParameter(FireMode->BeamVectorParamName, BeamVector);
    }

    // Impact fx & sound
    const FVector& ImpactNormal = HitscanInfo.Vectors[1];
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireMode->BeamImpactTemplate, FTransform(ImpactNormal.Rotation(), BeamEnd));
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireMode->BeamImpactSound, BeamEnd);
}


//============================================================
// FireModeCharged interface
//============================================================

void AUR_Weapon::ChargeLevel_Implementation(UUR_FireModeCharged* FireMode)
{

}


//============================================================
// FireModeContinuous interface
//============================================================

void AUR_Weapon::FiringTick_Implementation(UUR_FireModeContinuous* FireMode)
{

}
