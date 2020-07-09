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
#include "Kismet/KismetSystemLibrary.h" //debug
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

    BringUpTime = 0.25f;
    PutDownTime = 0.25f;
    CooldownDelaysPutDownByPercent = 0.5f;
    bReducePutDownDelayByPutDownTime = false;

    SetCanBeDamaged(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AUR_Weapon, AmmoCount, COND_OwnerOnly);
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
    CheckWeaponAttachment();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Weapon Attachment

void AUR_Weapon::CheckWeaponAttachment()
{
    switch (WeaponState)
    {
    case EWeaponState::Inactive:
        if (bIsAttached)
        {
            DetachMeshFromPawn();
        }
        break;

    default:
        if (!bIsAttached)
        {
            AttachMeshToPawn();
        }
        break;
    }
}

void AUR_Weapon::AttachMeshToPawn()
{
    if (URCharOwner)
    {
        this->SetActorHiddenInGame(false);
        Mesh1P->AttachToComponent(URCharOwner->MeshFirstPerson, FAttachmentTransformRules::KeepRelativeTransform, URCharOwner->GetWeaponAttachPoint());
        Mesh3P->AttachToComponent(URCharOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName(TEXT("hand_r_Socket")));
        UpdateMeshVisibility();
        bIsAttached = true;
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

    bIsAttached = false;
}

USkeletalMeshComponent* AUR_Weapon::GetVisibleMesh() const
{
    return UUR_FunctionLibrary::IsViewingFirstPerson(URCharOwner) ? Mesh1P : Mesh3P;
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
        //UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("WeaponState: %s"), *UUR_FunctionLibrary::GetEnumValueAsString(TEXT("EWeaponState"), WeaponState)), true, false, FColor::Purple, 2.f);
        OnWeaponStateChanged.Broadcast(this, NewState);
    }

    // check this every state change to support all edge cases
    CheckWeaponAttachment();

    switch (WeaponState)
    {

    case EWeaponState::BringUp:
        Activate();
        break;

    case EWeaponState::Idle:
        if (GetWorld()->GetTimerManager().IsTimerActive(PutDownDelayTimerHandle))
        {
            // if cooldown delays putdown by 100%, the timer can be slightly late.
            // we can force it to happen now.
            GetWorld()->GetTimerManager().ClearTimer(PutDownDelayTimerHandle);
            RequestPutDown();
        }
        else
        {
            // Fire all independent modes in queue
            for (int32 i = DesiredFireModes.Num() - 1; i >= 0; i--)
            {
                if (DesiredFireModes[i]->IsIndependentFireMode())
                {
                    TryStartFire(DesiredFireModes[i]);
                }
            }
            // Fire topmost non-independent mode
            if (DesiredFireModes.Num() > 0)
            {
                TryStartFire(DesiredFireModes[0]);
            }
        }
        break;

    case EWeaponState::PutDown:
    case EWeaponState::Inactive:
        Deactivate();
        break;

    }
}

void AUR_Weapon::BringUp(float FromPosition)
{
    GetWorld()->GetTimerManager().ClearTimer(SwapAnimTimerHandle);

    SetWeaponState(EWeaponState::BringUp);

    if (GetNetMode() != NM_DedicatedServer
        && BringUpMontage
        && URCharOwner
        && URCharOwner->MeshFirstPerson
        && URCharOwner->MeshFirstPerson->GetAnimInstance())
    {
        float Duration = BringUpMontage->GetPlayLength();
        float PlayRate = Duration / BringUpTime;
        float StartTime = FromPosition * Duration;
        //note: start time must not account for the play rate
        URCharOwner->MeshFirstPerson->GetAnimInstance()->Montage_Play(BringUpMontage, PlayRate, EMontagePlayReturnType::MontageLength, StartTime);
        //TODO: 3p animation
    }

    float Delay = (1.f - FromPosition) * BringUpTime;
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
        // When swapping back and forth while firing, this may happen
        if (CurrentFireMode && CurrentFireMode->IsBusy())
        {
            SetWeaponState(EWeaponState::Firing);

            // PutDown might have initiated spindown. Resume spinup if desired.
            if (DesiredFireModes.Num() > 0 && DesiredFireModes[0] == CurrentFireMode)
            {
                TryStartFire(CurrentFireMode);
            }
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

    if (GetNetMode() != NM_DedicatedServer
        && PutDownMontage
        && URCharOwner
        && URCharOwner->MeshFirstPerson
        && URCharOwner->MeshFirstPerson->GetAnimInstance())
    {
        float Duration = PutDownMontage->GetPlayLength();
        float PlayRate = Duration / PutDownTime;
        float StartTime = (1.f - FromPosition) * Duration;
        URCharOwner->MeshFirstPerson->GetAnimInstance()->Montage_Play(PutDownMontage, PlayRate, EMontagePlayReturnType::MontageLength, StartTime);
        //TODO: 3p animation
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

void AUR_Weapon::Activate()
{
    for (UUR_FireModeBase* FireMode : FireModes)
    {
        FireMode->Activate();
    }

    // Read desired fire modes from player
    if (UUR_FunctionLibrary::IsLocallyControlled(this) && URCharOwner)
    {
        // (read in reverse order to rebuild stack in right order)
        for (int32 i = URCharOwner->DesiredFireModeNum.Num() - 1; i >= 0; i--)
        {
            RequestStartFire(URCharOwner->DesiredFireModeNum[i]);
        }
    }
}

void AUR_Weapon::Deactivate()
{
    DesiredFireModes.Empty();

    for (UUR_FireModeBase* FireMode : FireModes)
    {
        FireMode->Deactivate();
    }
}

void AUR_Weapon::OnRep_AmmoCount()
{
    if (CurrentFireMode && CurrentFireMode->IsBusy())
    {
        if (auto FMCharged = Cast<UUR_FireModeCharged>(CurrentFireMode))
        {
            // Do nothing here. FMCharged goes through idle state between each shot.
            // We don't need to stopfire when out of ammo.
        }
        else if (auto FMContinuous = Cast<UUR_FireModeContinuous>(CurrentFireMode))
        {
            if (AmmoCount < 1 && FMContinuous->AmmoCostPerSecond > 0.f)
            {
                // The continuous mode allows firing with 0 ammo up till the next ammo consumption
                float Delay = 1.f / FMContinuous->AmmoCostPerSecond;
                FTimerHandle Handle;
                FTimerDelegate Callback;
                Callback.BindLambda([this, FMContinuous]
                {
                    if (AmmoCount < 1 && FMContinuous)
                    {
                        FMContinuous->StopFire();
                    }
                });
                GetWorld()->GetTimerManager().SetTimer(Handle, Callback, Delay, false);
                return;
            }
        }
        else if (!HasEnoughAmmoFor(CurrentFireMode))
        {
            CurrentFireMode->StopFire();
        }
    }

    if (AmmoCount == 0)
    {
        //TODO: auto swap here
    }
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
            if (FireMode->IsIndependentFireMode())
            {
                // Add independent modes at the bottom of the stack
                DesiredFireModes.Add(FireMode);
            }
            else
            {
                // Insert standard modes at the top
                DesiredFireModes.Insert(FireMode, 0);
            }
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

void AUR_Weapon::RequestBringUp()
{
    GetWorld()->GetTimerManager().ClearTimer(PutDownDelayTimerHandle);

    // If a FireMode is active, cancel the RequestIdle
    // (happens if we swap back and forth very fast while firing)
    if (CurrentFireMode)
    {
        CurrentFireMode->SetRequestIdle(false);

        // The RequestPutDown call might have initiated spindown.
        // If firemode is still desired, try to resume spinup now.
        if (DesiredFireModes.Num() > 0 && DesiredFireModes[0] == CurrentFireMode)
        {
            TryStartFire(CurrentFireMode);
        }
    }

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
        PutDown(1.f - GetWorld()->GetTimerManager().GetTimerRemaining(SwapAnimTimerHandle) / BringUpTime);
        return;

    case EWeaponState::Idle:
        PutDown(1.f);
        return;

    case EWeaponState::Firing:
    {
        // Request firemode to go idle whenever it sees opportunity.
        CurrentFireMode->SetRequestIdle(true);

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
    // Independent fire modes should not mess with weapon state
    if (FireMode->IsIndependentFireMode())
    {
        if (WeaponState == EWeaponState::Idle || WeaponState == EWeaponState::Firing)
        {
            if (HasEnoughAmmoFor(FireMode))
            {
                FireMode->RequestStartFire();
            }
            else
            {
                UGameplayStatics::PlaySound2D(GetWorld(), OutOfAmmoSound);
            }

            // Don't stay as a desired mode
            DesiredFireModes.Remove(FireMode);
        }
        return;
    }

    // Allow calling RequestStartFire on the currently active firemode so we can spinup while spindown
    if (WeaponState == EWeaponState::Idle || (WeaponState == EWeaponState::Firing && CurrentFireMode == FireMode))
    {
        if (HasEnoughAmmoFor(FireMode))
        {
            FireMode->RequestStartFire();
        }
        else
        {
            // Out of ammo
            UGameplayStatics::PlaySound2D(GetWorld(), OutOfAmmoSound);

            // Loop as long as user is holding fire
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
        FireRot = URCharOwner->GetBaseAimRotation();
    }
    else
    {
        GetActorEyesViewPoint(FireLoc, FireRot);
    }
}

void AUR_Weapon::OffsetFireLoc(FVector& FireLoc, const FRotator& FireRot, FName OffsetSocketName)
{
    // Offset by socket if there is one
    if (OffsetSocketName != NAME_None)
    {
        FVector MuzzleLoc = Mesh1P->GetSocketLocation(OffsetSocketName);
        FVector MuzzleOffset = MuzzleLoc - FireLoc;
        if (!MuzzleOffset.IsNearlyZero())
        {
            FVector OriginalFireLoc = FireLoc;

            // Offset forward but stay centered
            FireLoc += MuzzleOffset.Size() * FireRot.Vector();

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
}

void AUR_Weapon::GetValidatedFireVector(const FSimulatedShotInfo& SimulatedInfo, FVector& FireLoc, FRotator& FireRot, FName OffsetSocketName)
{
    // Authority location & rotation
    GetFireVector(FireLoc, FireRot);

    //NOTE: We assume convention that FireLoc and FireDir are always the first two vectors of SimulatedInfo.

    if (SimulatedInfo.Vectors.Num() >= 2)
    {
        // Client rotation is always accepted
        FireRot = SimulatedInfo.Vectors[1].Rotation();
    }

    // Shortcut for standalone
    if (GetNetMode() == NM_Standalone)
    {
        if (SimulatedInfo.Vectors.Num() >= 1)
        {
            FireLoc = SimulatedInfo.Vectors[0];
        }
        return;
    }

    // Offset authority loc
    if (OffsetSocketName != NAME_None)
    {
        OffsetFireLoc(FireLoc, FireRot, OffsetSocketName);
    }

    // Validate client fire location
    if (SimulatedInfo.Vectors.Num() >= 1)
    {
        FVector ClientLoc = SimulatedInfo.Vectors[0];

        if ((ClientLoc - FireLoc).SizeSquared() < 400.f)
        {
            // Acceptable position error
            // Need one more trace to confirm, or cheater could fire thru thin walls... :(
            FCollisionQueryParams TraceParams(FCollisionQueryParams::DefaultQueryParam);
            TraceParams.AddIgnoredActor(this);
            TraceParams.AddIgnoredActor(URCharOwner);
            FHitResult Hit;
            if (GetWorld()->LineTraceSingleByChannel(Hit, FireLoc, ClientLoc, ECollisionChannel::ECC_Visibility, TraceParams))
            {
                FireLoc = Hit.Location;
            }
            else
            {
                FireLoc = ClientLoc;    // Accept
            }
        }
    }
}

FVector AUR_Weapon::SeededRandCone(const FVector& Dir, float ConeHalfAngleDeg, int32 Seed)
{
    FVector Dir2 = Dir.GetSafeNormal();
    if (!Dir2.IsZero() && ConeHalfAngleDeg > 0.f && ConeHalfAngleDeg < 90.f)
    {
        // Find a normal vector to use as our opposite side of the triangle
        FVector OppositeVector = FVector(-Dir2.Y, Dir2.X, 0.f).GetSafeNormal();
        if (OppositeVector.IsZero())
        {
            OppositeVector = FVector(0.f, -Dir2.Z, Dir2.Y).GetSafeNormal();
        }

        // Max opposite side size is dictated by supplied max angle
        float Tan = FMath::Tan(FMath::DegreesToRadians(ConeHalfAngleDeg));
        float MaxOppositeSize = Tan * 1.f;

        // Set random seed
        FMath::SRandInit(Seed);

        // Find a random point on that opposite side
        FVector Point = Dir2 + FMath::SRand() * MaxOppositeSize * OppositeVector;

        // Rotate point around axis by a random angle
        FQuat RandRotation = FQuat(Dir2, FMath::DegreesToRadians(FMath::SRand()*360.f));

        return RandRotation.RotateVector(Point).GetSafeNormal();
    }
    else
    {
        return Dir2;
    }
}

AUR_Projectile* AUR_Weapon::SpawnProjectile_Implementation(TSubclassOf<AUR_Projectile> InProjectileClass, const FVector& StartLoc, const FRotator& StartRot)
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
    OutHit.Location = TraceEnd;
    OutHit.ImpactNormal = (TraceEnd - TraceStart).GetSafeNormal();

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
    return AmmoCount >= FireMode->InitialAmmoCost;
}

void AUR_Weapon::ConsumeAmmo(int32 Amount)
{
    AmmoCount = FMath::Clamp(AmmoCount - Amount, 0, 999);

    OnRep_AmmoCount();
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

    case EWeaponState::PutDown:
    {
        // delay to check if we're late in a very quick putdown-bringup-idle scenario
        // delay by the amount of time it would take to reach idle if we called bringup now
        float BringUpPct = GetWorld()->GetTimerManager().GetTimerRemaining(SwapAnimTimerHandle) / PutDownTime;
        Delay = FMath::Max(0.001f, (1.f - BringUpPct) * BringUpTime);
        break;
    }

    case EWeaponState::Idle:
        Delay = 0.f;
        break;

    case EWeaponState::Firing:
        if (FireMode == CurrentFireMode && FireMode->SpinUpTime > 0.f)
        {
            Delay = 0.f;    // we can resume spinning up at any point during spindown
        }
        else
        {
            Delay = CurrentFireMode->GetTimeUntilIdle();
        }
        break;

    default:
        return TIMEUNTILFIRE_NEVER;
    }

    if (Delay <= 0.f && !HasEnoughAmmoFor(FireMode))
    {
        return TIMEUNTILFIRE_NEVER;
    }

    return Delay;
}

/**
* Implementation of spinup/spindown mechanisms are very specific to the individual weapons,
* I am not going to provide a default implementation here in UR_Weapon.
* The typical workflow however should be something like this :
* - BeginSpinUp -> enable tick
* - BeginSpinDown -> enable tick
* - Tick -> update parameters (sound pitch/volume, FX intensity, barrel rotation rate)
* - SpinDone -> disable tick
*/

void AUR_Weapon::BeginSpinUp_Implementation(UUR_FireModeBase* FireMode, float CurrentSpinValue)
{
    GEngine->AddOnScreenDebugMessage(117, 3.f, FColor::Blue, *FString::Printf(TEXT("SPINNING UP (from %f)"), CurrentSpinValue));
}

void AUR_Weapon::BeginSpinDown_Implementation(UUR_FireModeBase* FireMode, float CurrentSpinValue)
{
    GEngine->AddOnScreenDebugMessage(117, 3.f, FColor::Blue, *FString::Printf(TEXT("SPINNING DOWN (from %f)"), CurrentSpinValue));
}

void AUR_Weapon::SpinDone_Implementation(UUR_FireModeBase* FireMode, bool bFullySpinnedUp)
{
    if (bFullySpinnedUp)
    {
        GEngine->AddOnScreenDebugMessage(117, 3.f, FColor::Blue, *FString::Printf(TEXT("SPINUP DONE")));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(117, 3.f, FColor::Blue, *FString::Printf(TEXT("SPINDOWN DONE")));
    }
}


//============================================================
// FireModeBasic interface
//============================================================

void AUR_Weapon::SimulateShot_Implementation(UUR_FireModeBasic* FireMode, FSimulatedShotInfo& OutSimulatedInfo)
{
    FVector FireLoc;
    FRotator FireRot;
    GetFireVector(FireLoc, FireRot);
    OffsetFireLoc(FireLoc, FireRot, FireMode->MuzzleSocketName);
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

    if (FireMode->Spread > 0.f)
    {
        int32 Seed = FMath::GetRandSeed();
        FireRot = SeededRandCone(FireRot.Vector(), FireMode->Spread, Seed).Rotation();
        OutSimulatedInfo.Seed = Seed;
        /**
        * NOTE: might want to rethink about this a bit.
        * I'm not sure there is actually a point in sending Seed, over simply sending the altered FireRot.
        *
        * I thought sending a Seed would be less cheatable,
        * but not really because a cheater can calculate seed result, and retro-adjust his FireRot.
        * So it doesn't really make any difference.
        *
        * The seed parameter can still be useful for a case like hitscan shotgun though.
        */
    }

    FVector TraceEnd = FireLoc + FireMode->HitscanTraceDistance * FireRot.Vector();

    FHitResult Hit;
    HitscanTrace(FireLoc, TraceEnd, Hit);

    OutHitscanInfo.Vectors.EmplaceAt(0, Hit.Location);
    OutHitscanInfo.Vectors.EmplaceAt(1, Hit.ImpactNormal);
}

void AUR_Weapon::AuthorityShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo)
{
    if (FireMode->ProjectileClass)
    {
        FVector FireLoc;
        FRotator FireRot;
        GetValidatedFireVector(SimulatedInfo, FireLoc, FireRot, FireMode->MuzzleSocketName);

        // Add spread (if we do implement client-side projectiles, we'll need to replicate a Seed)
        // OR just replicate the altered FireRot? see note above
        if (FireMode->Spread > 0.f)
        {
            FireRot = FMath::VRandCone(FireRot.Vector(), FMath::DegreesToRadians(FireMode->Spread)).Rotation();
        }

        SpawnProjectile(FireMode->ProjectileClass, FireLoc, FireRot);

        // Charged mode consumes ammo while charging, not when releasing shot
        if (!Cast<UUR_FireModeCharged>(FireMode))
        {
            ConsumeAmmo(FireMode->InitialAmmoCost);
        }
    }
}

void AUR_Weapon::AuthorityHitscanShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo, FHitscanVisualInfo& OutHitscanInfo)
{
    FVector TraceStart;
    FRotator FireRot;
    GetValidatedFireVector(SimulatedInfo, TraceStart, FireRot);

    if (FireMode->Spread > 0.f)
    {
        FireRot = SeededRandCone(FireRot.Vector(), FireMode->Spread, SimulatedInfo.Seed).Rotation();
    }

    FVector TraceEnd = TraceStart + FireMode->HitscanTraceDistance * FireRot.Vector();

    FHitResult Hit;
    HitscanTrace(TraceStart, TraceEnd, Hit);

    if (Hit.bBlockingHit && Hit.GetActor())
    {
        float Damage = FireMode->HitscanDamage;
        auto DamType = FireMode->HitscanDamageType;
        UGameplayStatics::ApplyPointDamage(Hit.GetActor(), Damage, FireRot.Vector(), Hit, GetInstigatorController(), this, DamType);
    }

    OutHitscanInfo.Vectors.EmplaceAt(0, Hit.Location);
    OutHitscanInfo.Vectors.EmplaceAt(1, Hit.ImpactNormal);

    // Charged mode consumes ammo while charging, not when releasing shot
    if (!Cast<UUR_FireModeCharged>(FireMode))
    {
        ConsumeAmmo(FireMode->InitialAmmoCost);
    }
}

void AUR_Weapon::PlayFireEffects_Implementation(UUR_FireModeBasic* FireMode)
{
    if (UUR_FunctionLibrary::IsViewingFirstPerson(URCharOwner))
    {
        UGameplayStatics::SpawnSoundAttached(FireMode->FireSound, Mesh1P, FireMode->MuzzleSocketName, FVector(0), EAttachLocation::SnapToTarget);
        UUR_FunctionLibrary::SpawnEffectAttached(FireMode->MuzzleFlashTemplate, FTransform(), Mesh1P, FireMode->MuzzleSocketName, EAttachLocation::SnapToTargetIncludingScale);
        if (URCharOwner->MeshFirstPerson && URCharOwner->MeshFirstPerson->GetAnimInstance())
        {
            //TODO: fire animation should be in weapon, maybe even in firemode?
            URCharOwner->MeshFirstPerson->GetAnimInstance()->Montage_Play(URCharOwner->FireAnimation);
        }
    }
    else
    {
        UGameplayStatics::SpawnSoundAttached(FireMode->FireSound, Mesh3P, FireMode->MuzzleSocketName, FVector(0), EAttachLocation::SnapToTarget);
        UUR_FunctionLibrary::SpawnEffectAttached(FireMode->MuzzleFlashTemplate, FTransform(), Mesh3P, FireMode->MuzzleSocketName, EAttachLocation::SnapToTargetIncludingScale);
        //TODO: play 3p anim
    }
}

void AUR_Weapon::PlayHitscanEffects_Implementation(UUR_FireModeBasic* FireMode, const FHitscanVisualInfo& HitscanInfo)
{
    const FVector& BeamStart = GetVisibleMesh()->GetSocketLocation(FireMode->MuzzleSocketName);
    const FVector& BeamEnd = HitscanInfo.Vectors[0];
    FVector BeamVector = BeamEnd - BeamStart;

    UFXSystemComponent* BeamComp = UUR_FunctionLibrary::SpawnEffectAtLocation(this, FireMode->BeamTemplate, FTransform(BeamStart));
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

void AUR_Weapon::ChargeLevel_Implementation(UUR_FireModeCharged* FireMode, int32 ChargeLevel, bool bWasPaused)
{
    // Default ammo consumption = 1 per charge
    if (HasAuthority())
    {
        // If a charge was loaded, consume ammo for it
        if (!bWasPaused)
        {
            ConsumeAmmo(1);
        }

        // If we don't have enough ammo for next charge, stop charging
        if (AmmoCount < 1)
        {
            FireMode->BlockNextCharge(FireMode->MaxChargeHoldTime);
        }
    }

    // Default hitscan damage = linear scale
    FireMode->HitscanDamage = FMath::Lerp(FireMode->HitscanDamageMin, FireMode->HitscanDamageMax, FireMode->GetTotalChargePercent(false));

    GEngine->AddOnScreenDebugMessage(118, 3.f, FColor::Blue, *FString::Printf(TEXT("CHARGE LEVEL %i (%i)"), ChargeLevel, bWasPaused?1:0));
}

/**
* SOME NOTES:
*
* FireModeCharged extends FireModeBasic, and uses the same interface callbacks for the firing part.
* In order to access ChargeLevel, you will need to cast FireMode to a FireModeCharged.
*
* Implementations of charged shots are very specific to the individual weapons,
* so I am not going to provide default implementations here in UR_Weapon.
*
* For charged projectiles, implementers can override AuthorityShot or SpawnProjectile,
* cast to FireModeCharged, and alter the spawned projectile(s) as desired.
*
* For charged hitscan, implementers must override PlayFireEffects/PlayHitscanEffects to adjust visuals.
* For damage, either override AuthorityHitscanShot, or modify FireMode properties on the fly every ChargeLevel.
*/


//============================================================
// FireModeContinuous interface
//============================================================

void AUR_Weapon::SimulateContinuousHitCheck_Implementation(UUR_FireModeContinuous* FireMode)
{
    /**
    * TODO: Client side hitreg, under some specific conditions.
    *
    * For a straight beam we can try an implementation as described in FireModeContinuous.h
    *
    * For something like minigun, we should just do server hit detection,
    * because I don't feel like syncing up each individual randomly spreaded bullet.
    */
}

void AUR_Weapon::AuthorityStartContinuousFire_Implementation(UUR_FireModeContinuous* FireMode)
{
    if (!HasEnoughAmmoFor(FireMode))
    {
        FireMode->StopFire();
        return;
    }

    ConsumeAmmo(FireMode->InitialAmmoCost);
    FireMode->AmmoCostAccumulator = 0.f;
}

void AUR_Weapon::AuthorityContinuousHitCheck_Implementation(UUR_FireModeContinuous* FireMode)
{
    FireMode->AmmoCostAccumulator += FireMode->AmmoCostPerSecond * FireMode->HitCheckInterval;
    while (FireMode->AmmoCostAccumulator >= 1.f)
    {
        //NOTE: Here we consume ammo before the fact, not after.
        // Eg. if we have 1 ammo, it drops to zero and then we can continue firing until next ammo consumption.
        // --> This is because we have an initial ammo cost to prevent abuse.
        // This is easier to handle (and prevent abuse) than the other way around.
        if (AmmoCount <= 0)
        {
            FireMode->StopFire();
            return;
        }
        ConsumeAmmo(1);
        FireMode->AmmoCostAccumulator -= 1.f;
    }

    FVector FireLoc;
    FRotator FireRot;
    GetFireVector(FireLoc, FireRot);

    if (FireMode->Spread > 0.f)
    {
        FireRot = FMath::VRandCone(FireRot.Vector(), FMath::DegreesToRadians(FireMode->Spread)).Rotation();
    }

    FVector TraceEnd = FireLoc + FireMode->TraceDistance * FireRot.Vector();

    FHitResult Hit;
    HitscanTrace(FireLoc, TraceEnd, Hit);

    if (Hit.bBlockingHit && Hit.GetActor())
    {
        float Damage = FireMode->Damage;
        auto DamType = FireMode->DamageType;
        UGameplayStatics::ApplyPointDamage(Hit.GetActor(), Damage, FireRot.Vector(), Hit, GetInstigatorController(), this, DamType);
    }
}

void AUR_Weapon::AuthorityStopContinuousFire_Implementation(UUR_FireModeContinuous* FireMode)
{
    //Nothing to do
}

void AUR_Weapon::StartContinuousEffects_Implementation(UUR_FireModeContinuous* FireMode)
{
    if (!FireMode->BeamComponent || FireMode->BeamComponent->IsBeingDestroyed())
    {
        //UKismetSystemLibrary::PrintString(this, TEXT("NEW PARTICLE"));
        FireMode->BeamComponent = UUR_FunctionLibrary::SpawnEffectAttached(FireMode->BeamTemplate, FTransform(), GetVisibleMesh(), FireMode->MuzzleSocketName, EAttachLocation::SnapToTargetIncludingScale);
    }
    if (FireMode->BeamComponent)
    {
        FireMode->BeamComponent->Activate(true);
    }

    if (!FireMode->FireLoopAudioComponent || FireMode->FireLoopAudioComponent->IsBeingDestroyed())
    {
        //UKismetSystemLibrary::PrintString(this, TEXT("NEW SOUND"));
        FireMode->FireLoopAudioComponent = UGameplayStatics::SpawnSoundAttached(FireMode->FireLoopSound, GetVisibleMesh(), FireMode->MuzzleSocketName, FVector(0), EAttachLocation::SnapToTarget, true);
    }
    if (FireMode->FireLoopAudioComponent)
    {
        FireMode->FireLoopAudioComponent->Activate(true);
    }
}

void AUR_Weapon::UpdateContinuousEffects_Implementation(UUR_FireModeContinuous* FireMode, float DeltaTime)
{
    if (FireMode->BeamComponent)
    {
        FVector FireLoc;
        FRotator FireRot;
        GetFireVector(FireLoc, FireRot);

        if (FireMode->Spread > 0.f)
        {
            FireRot = FMath::VRandCone(FireRot.Vector(), FMath::DegreesToRadians(FireMode->Spread)).Rotation();
        }

        FVector TraceEnd = FireLoc + FireMode->TraceDistance * FireRot.Vector();

        FHitResult Hit;
        HitscanTrace(FireLoc, TraceEnd, Hit);

        FVector BeamVector = Hit.Location - FireMode->BeamComponent->GetComponentLocation();
        BeamVector = FireMode->BeamComponent->GetComponentTransform().InverseTransformVector(BeamVector);

        FireMode->BeamComponent->SetVectorParameter(FireMode->BeamVectorParamName, BeamVector);
        FireMode->BeamComponent->SetVectorParameter(FireMode->BeamImpactNormalParamName, Hit.ImpactNormal);

        //TODO: There is an issue here, if player/viewer changes 1P/3P perspective while firing,
        // the effect (and sound) need to be re-attached to the appropriate weapon mesh.
    }
}

void AUR_Weapon::StopContinuousEffects_Implementation(UUR_FireModeContinuous* FireMode)
{
    if (FireMode->BeamComponent)
    {
        FireMode->BeamComponent->Deactivate();
    }
    if (FireMode->FireLoopAudioComponent)
    {
        FireMode->FireLoopAudioComponent->Deactivate();
    }
}
