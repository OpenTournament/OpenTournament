// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayAbility_RangedWeapon.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "NativeGameplayTags.h"

#include "UR_LogChannels.h"
#include "UR_WeaponDebugCVars.h"
#include "GAS/UR_GameplayAbilityTargetData_SingleTargetHit.h"
#include "Physics/UR_CollisionChannels.h"
#include "Weapons/UR_RangedWeaponInstance.h"
#include "Weapons/UR_WeaponStateComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayAbility_RangedWeapon)

/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////

// Weapon fire will be blocked/canceled if the player has this tag
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_WeaponFireBlocked, "Ability.Weapon.NoFiring");

/////////////////////////////////////////////////////////////////////////////////////////////////

FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
{
    if (ConeHalfAngleRad > 0.f)
    {
        const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);

        // consider the cone a concatenation of two rotations. one "away" from the center line, and another "around" the circle
        // apply the exponent to the away-from-center rotation. a larger exponent will cluster points more tightly around the center
        const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
        const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
        const float AngleAround = FMath::FRand() * 360.0f;

        FRotator Rot = Dir.Rotation();
        FQuat DirQuat(Rot);
        FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
        FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
        FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
        FinalDirectionQuat.Normalize();

        return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
    }
    else
    {
        return Dir.GetSafeNormal();
    }
}

namespace OTConsoleVariables
{
    static float DrawBulletTracesDuration = 0.0f;
    static FAutoConsoleVariableRef CVarDrawBulletTraceDuration
    (
        TEXT("OT.Weapon.DrawBulletTraceDuration"),
        DrawBulletTracesDuration,
        TEXT("Should we do debug drawing for bullet traces (if above zero, sets how long (in seconds))"),
        ECVF_Default
    );

    static float DrawBulletHitDuration = 0.0f;
    static FAutoConsoleVariableRef CVarDrawBulletHits
    (
        TEXT("OT.Weapon.DrawBulletHitDuration"),
        DrawBulletHitDuration,
        TEXT("Should we do debug drawing for bullet impacts (if above zero, sets how long (in seconds))"),
        ECVF_Default
    );

    static float DrawBulletHitRadius = 3.0f;
    static FAutoConsoleVariableRef CVarDrawBulletHitRadius
    (
        TEXT("OT.Weapon.DrawBulletHitRadius"),
        DrawBulletHitRadius,
        TEXT("When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in uu)"),
        ECVF_Default
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameplayAbility_RangedWeapon::UUR_GameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SourceBlockedTags.AddTag(TAG_WeaponFireBlocked);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_RangedWeaponInstance* UUR_GameplayAbility_RangedWeapon::GetWeaponInstance() const
{
    return Cast<UUR_RangedWeaponInstance>(GetAssociatedEquipment());
}

bool UUR_GameplayAbility_RangedWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

    if (bResult)
    {
        if (GetWeaponInstance() == nullptr)
        {
            UE_LOG
            (LogGameAbilitySystem,
                Error,
                TEXT("Weapon ability %s cannot be activated because there is no associated ranged weapon (equipment instance=%s but needs to be derived from %s)"),
                *GetPathName(),
                *GetPathNameSafe(GetAssociatedEquipment()),
                *UUR_RangedWeaponInstance::StaticClass()->GetName());
            bResult = false;
        }
    }

    return bResult;
}

int32 UUR_GameplayAbility_RangedWeapon::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
{
    for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
    {
        const FHitResult& CurHitResult = HitResults[Idx];
        if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
        {
            // If we hit a pawn, we're good
            return Idx;
        }
        else
        {
            AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
            if ((HitActor != nullptr) && (HitActor->GetAttachParentActor() != nullptr) && (Cast<APawn>(HitActor->GetAttachParentActor()) != nullptr))
            {
                // If we hit something attached to a pawn, we're good
                return Idx;
            }
        }
    }

    return INDEX_NONE;
}

void UUR_GameplayAbility_RangedWeapon::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
    if (AActor* Avatar = GetAvatarActorFromActorInfo())
    {
        // Ignore any actors attached to the avatar doing the shooting
        TArray<AActor*> AttachedActors;
        Avatar->GetAttachedActors(/*out*/ AttachedActors);
        TraceParams.AddIgnoredActors(AttachedActors);
    }
}

ECollisionChannel UUR_GameplayAbility_RangedWeapon::DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const
{
    return Game_TraceChannel_Weapon;
}

FHitResult UUR_GameplayAbility_RangedWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const
{
    TArray<FHitResult> HitResults;

    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/ true, /*IgnoreActor=*/ GetAvatarActorFromActorInfo());
    TraceParams.bReturnPhysicalMaterial = true;
    AddAdditionalTraceIgnoreActors(TraceParams);
    //TraceParams.bDebugQuery = true;

    const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, bIsSimulated);

    if (SweepRadius > 0.0f)
    {
        GetWorld()->SweepMultiByChannel(HitResults, StartTrace, EndTrace, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(SweepRadius), TraceParams);
    }
    else
    {
        GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, TraceChannel, TraceParams);
    }

    FHitResult Hit(ForceInit);
    if (HitResults.Num() > 0)
    {
        // Filter the output list to prevent multiple hits on the same actor;
        // this is to prevent a single bullet dealing damage multiple times to
        // a single actor if using an overlap trace
        for (FHitResult& CurHitResult : HitResults)
        {
            auto Pred = [&CurHitResult](const FHitResult& Other)
            {
                return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
            };

            if (!OutHitResults.ContainsByPredicate(Pred))
            {
                OutHitResults.Add(CurHitResult);
            }
        }

        Hit = OutHitResults.Last();
    }
    else
    {
        Hit.TraceStart = StartTrace;
        Hit.TraceEnd = EndTrace;
    }

    return Hit;
}

FVector UUR_GameplayAbility_RangedWeapon::GetWeaponTargetingSourceLocation() const
{
    // Use Pawn's location as a base
    APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
    check(AvatarPawn);

    const FVector SourceLoc = AvatarPawn->GetActorLocation();
    const FQuat SourceRot = AvatarPawn->GetActorQuat();

    FVector TargetingSourceLocation = SourceLoc;

    //@TODO: Add an offset from the weapon instance and adjust based on pawn crouch/aiming/etc...

    return TargetingSourceLocation;
}

FTransform UUR_GameplayAbility_RangedWeapon::GetTargetingTransform(APawn* SourcePawn, EGameAbilityTargetingSource Source) const
{
    check(SourcePawn);
    AController* SourcePawnController = SourcePawn->GetController();
    UUR_WeaponStateComponent* WeaponStateComponent = (SourcePawnController != nullptr) ? SourcePawnController->FindComponentByClass<UUR_WeaponStateComponent>() : nullptr;

    // The caller should determine the transform without calling this if the mode is custom!
    check(Source != EGameAbilityTargetingSource::Custom);

    const FVector ActorLoc = SourcePawn->GetActorLocation();
    FQuat AimQuat = SourcePawn->GetActorQuat();
    AController* Controller = SourcePawn->GetController();
    FVector SourceLoc;

    double FocalDistance = 1024.0f;
    FVector FocalLoc;

    FVector CamLoc;
    FRotator CamRot;
    bool bFoundFocus = false;


    if ((Controller != nullptr) && ((Source == EGameAbilityTargetingSource::CameraTowardsFocus) || (Source == EGameAbilityTargetingSource::PawnTowardsFocus) || (Source == EGameAbilityTargetingSource::WeaponTowardsFocus)))
    {
        // Get camera position for later
        bFoundFocus = true;

        APlayerController* PC = Cast<APlayerController>(Controller);
        if (PC != nullptr)
        {
            PC->GetPlayerViewPoint(/*out*/ CamLoc, /*out*/ CamRot);
        }
        else
        {
            SourceLoc = GetWeaponTargetingSourceLocation();
            CamLoc = SourceLoc;
            CamRot = Controller->GetControlRotation();
        }

        // Determine initial focal point to
        FVector AimDir = CamRot.Vector().GetSafeNormal();
        FocalLoc = CamLoc + (AimDir * FocalDistance);

        // Move the start and focal point up in front of pawn
        if (PC)
        {
            const FVector WeaponLoc = GetWeaponTargetingSourceLocation();
            CamLoc = FocalLoc + (((WeaponLoc - FocalLoc) | AimDir) * AimDir);
            FocalLoc = CamLoc + (AimDir * FocalDistance);
        }
        //Move the start to be the HeadPosition of the AI
        else if (AAIController* AIController = Cast<AAIController>(Controller))
        {
            CamLoc = SourcePawn->GetActorLocation() + FVector(0, 0, SourcePawn->BaseEyeHeight);
        }

        if (Source == EGameAbilityTargetingSource::CameraTowardsFocus)
        {
            // If we're camera -> focus then we're done
            return FTransform(CamRot, CamLoc);
        }
    }

    if ((Source == EGameAbilityTargetingSource::WeaponForward) || (Source == EGameAbilityTargetingSource::WeaponTowardsFocus))
    {
        SourceLoc = GetWeaponTargetingSourceLocation();
    }
    else
    {
        // Either we want the pawn's location, or we failed to find a camera
        SourceLoc = ActorLoc;
    }

    if (bFoundFocus && ((Source == EGameAbilityTargetingSource::PawnTowardsFocus) || (Source == EGameAbilityTargetingSource::WeaponTowardsFocus)))
    {
        // Return a rotator pointing at the focal point from the source
        return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
    }

    // If we got here, either we don't have a camera or we don't want to use it, either way go forward
    return FTransform(AimQuat, SourceLoc);
}

FHitResult UUR_GameplayAbility_RangedWeapon::DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const
{
#if ENABLE_DRAW_DEBUG
    if (OTConsoleVariables::DrawBulletTracesDuration > 0.0f)
    {
        static float DebugThickness = 1.0f;
        DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, OTConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
    }
#endif // ENABLE_DRAW_DEBUG

    FHitResult Impact;

    // Trace and process instant hit if something was hit
    // First trace without using sweep radius
    if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
    {
        Impact = WeaponTrace(StartTrace, EndTrace, /*SweepRadius=*/ 0.0f, bIsSimulated, /*out*/ OutHits);
    }

    if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
    {
        // If this weapon didn't hit anything with a line trace and supports a sweep radius, try that
        if (SweepRadius > 0.0f)
        {
            TArray<FHitResult> SweepHits;
            Impact = WeaponTrace(StartTrace, EndTrace, SweepRadius, bIsSimulated, /*out*/ SweepHits);

            // If the trace with sweep radius enabled hit a pawn, check if we should use its hit results
            const int32 FirstPawnIdx = FindFirstPawnHitResult(SweepHits);
            if (SweepHits.IsValidIndex(FirstPawnIdx))
            {
                // If we had a blocking hit in our line trace that occurs in SweepHits before our
                // hit pawn, we should just use our initial hit results since the Pawn hit should be blocked
                bool bUseSweepHits = true;
                for (int32 Idx = 0; Idx < FirstPawnIdx; ++Idx)
                {
                    const FHitResult& CurHitResult = SweepHits[Idx];

                    auto Pred = [&CurHitResult](const FHitResult& Other)
                    {
                        return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
                    };
                    if (CurHitResult.bBlockingHit && OutHits.ContainsByPredicate(Pred))
                    {
                        bUseSweepHits = false;
                        break;
                    }
                }

                if (bUseSweepHits)
                {
                    OutHits = SweepHits;
                }
            }
        }
    }

    return Impact;
}

void UUR_GameplayAbility_RangedWeapon::PerformLocalTargeting(OUT TArray<FHitResult>& OutHits)
{
    APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());

    UUR_RangedWeaponInstance* WeaponData = GetWeaponInstance();
    if (AvatarPawn && AvatarPawn->IsLocallyControlled() && WeaponData)
    {
        FRangedWeaponFiringInput InputData;
        InputData.WeaponData = WeaponData;
        InputData.bCanPlayBulletFX = (AvatarPawn->GetNetMode() != NM_DedicatedServer);

        //@TODO: Should do more complicated logic here when the player is close to a wall, etc...
        const FTransform TargetTransform = GetTargetingTransform(AvatarPawn, EGameAbilityTargetingSource::CameraTowardsFocus);
        InputData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
        InputData.StartTrace = TargetTransform.GetTranslation();

        InputData.EndAim = InputData.StartTrace + InputData.AimDir * WeaponData->GetMaxDamageRange();

#if ENABLE_DRAW_DEBUG
        if (OTConsoleVariables::DrawBulletTracesDuration > 0.0f)
        {
            static float DebugThickness = 2.0f;
            DrawDebugLine(GetWorld(), InputData.StartTrace, InputData.StartTrace + (InputData.AimDir * 100.0f), FColor::Yellow, false, OTConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
        }
#endif

        TraceBulletsInCartridge(InputData, /*out*/ OutHits);
    }
}

void UUR_GameplayAbility_RangedWeapon::TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits)
{
    UUR_RangedWeaponInstance* WeaponData = InputData.WeaponData;
    check(WeaponData);

    const int32 BulletsPerCartridge = WeaponData->GetBulletsPerCartridge();

    for (int32 BulletIndex = 0; BulletIndex < BulletsPerCartridge; ++BulletIndex)
    {
        const float BaseSpreadAngle = WeaponData->GetCalculatedSpreadAngle();
        const float SpreadAngleMultiplier = WeaponData->GetCalculatedSpreadAngleMultiplier();
        const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

        const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

        const FVector BulletDir = VRandConeNormalDistribution(InputData.AimDir, HalfSpreadAngleInRadians, WeaponData->GetSpreadExponent());

        const FVector EndTrace = InputData.StartTrace + (BulletDir * WeaponData->GetMaxDamageRange());
        FVector HitLocation = EndTrace;

        TArray<FHitResult> AllImpacts;

        FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, WeaponData->GetBulletTraceSweepRadius(), /*bIsSimulated=*/ false, /*out*/ AllImpacts);

        const AActor* HitActor = Impact.GetActor();

        if (HitActor)
        {
#if ENABLE_DRAW_DEBUG
            if (OTConsoleVariables::DrawBulletHitDuration > 0.0f)
            {
                DrawDebugPoint(GetWorld(), Impact.ImpactPoint, OTConsoleVariables::DrawBulletHitRadius, FColor::Red, false, OTConsoleVariables::DrawBulletHitRadius);
            }
#endif

            if (AllImpacts.Num() > 0)
            {
                OutHits.Append(AllImpacts);
            }

            HitLocation = Impact.ImpactPoint;
        }

        // Make sure there's always an entry in OutHits so the direction can be used for tracers, etc...
        if (OutHits.Num() == 0)
        {
            if (!Impact.bBlockingHit)
            {
                // Locate the fake 'impact' at the end of the trace
                Impact.Location = EndTrace;
                Impact.ImpactPoint = EndTrace;
            }

            OutHits.Add(Impact);
        }
    }
}

void UUR_GameplayAbility_RangedWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // Bind target data callback
    UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
    check(MyAbilityComponent);

    OnTargetDataReadyCallbackDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

    // Update the last firing time
    UUR_RangedWeaponInstance* WeaponData = GetWeaponInstance();
    check(WeaponData);
    WeaponData->UpdateFiringTime();

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UUR_GameplayAbility_RangedWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (IsEndAbilityValid(Handle, ActorInfo))
    {
        if (ScopeLockCount > 0)
        {
            WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
            return;
        }

        UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
        check(MyAbilityComponent);

        // When ability ends, consume target data and remove delegate
        MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
        MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
}

void UUR_GameplayAbility_RangedWeapon::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
    UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
    check(MyAbilityComponent);

    if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
    {
        FScopedPredictionWindow ScopedPrediction(MyAbilityComponent);

        // Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
        FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

        const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
        if (bShouldNotifyServer)
        {
            MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, MyAbilityComponent->ScopedPredictionKey);
        }

        const bool bIsTargetDataValid = true;

        bool bProjectileWeapon = false;

#if WITH_SERVER_CODE
        if (!bProjectileWeapon)
        {
            if (AController* Controller = GetControllerFromActorInfo())
            {
                if (Controller->GetLocalRole() == ROLE_Authority)
                {
                    // Confirm hit markers
                    if (UUR_WeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<UUR_WeaponStateComponent>())
                    {
                        TArray<uint8> HitReplaces;
                        for (uint8 i = 0; (i < LocalTargetDataHandle.Num()) && (i < 255); ++i)
                        {
                            if (FGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = static_cast<FGameplayAbilityTargetData_SingleTargetHit*>(LocalTargetDataHandle.Get(i)))
                            {
                                if (SingleTargetHit->bHitReplaced)
                                {
                                    HitReplaces.Add(i);
                                }
                            }
                        }

                        WeaponStateComponent->ClientConfirmTargetData(LocalTargetDataHandle.UniqueId, bIsTargetDataValid, HitReplaces);
                    }
                }
            }
        }
#endif //WITH_SERVER_CODE


        // See if we still have ammo
        if (bIsTargetDataValid && CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
        {
            // We fired the weapon, add spread
            UUR_RangedWeaponInstance* WeaponData = GetWeaponInstance();
            check(WeaponData);
            WeaponData->AddSpread();

            // Let the blueprint do stuff like apply effects to the targets
            OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
        }
        else
        {
            UE_LOG(LogGameAbilitySystem, Warning, TEXT("Weapon ability %s failed to commit (bIsTargetDataValid=%d)"), *GetPathName(), bIsTargetDataValid ? 1 : 0);
            K2_EndAbility();
        }
    }

    // We've processed the data
    MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UUR_GameplayAbility_RangedWeapon::StartRangedWeaponTargeting()
{
    check(CurrentActorInfo);

    AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
    check(AvatarActor);

    UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
    check(MyAbilityComponent);

    AController* Controller = GetControllerFromActorInfo();
    check(Controller);
    UUR_WeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<UUR_WeaponStateComponent>();

    FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

    TArray<FHitResult> FoundHits;
    PerformLocalTargeting(/*out*/ FoundHits);

    // Fill out the target data from the hit results
    FGameplayAbilityTargetDataHandle TargetData;
    TargetData.UniqueId = WeaponStateComponent ? WeaponStateComponent->GetUnconfirmedServerSideHitMarkerCount() : 0;

    if (FoundHits.Num() > 0)
    {
        const int32 CartridgeID = FMath::Rand();

        for (const FHitResult& FoundHit : FoundHits)
        {
            FUR_GameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FUR_GameplayAbilityTargetData_SingleTargetHit();
            NewTargetData->HitResult = FoundHit;
            NewTargetData->CartridgeID = CartridgeID;

            TargetData.Add(NewTargetData);
        }
    }

    // Send hit marker information
    if (WeaponStateComponent != nullptr)
    {
        WeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetData, FoundHits);
    }

    // Process the target data immediately
    OnTargetDataReadyCallback(TargetData, FGameplayTag());
}
