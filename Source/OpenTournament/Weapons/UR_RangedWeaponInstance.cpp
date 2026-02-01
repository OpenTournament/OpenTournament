// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_RangedWeaponInstance.h"

#include "NativeGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

#include "Camera/UR_CameraComponent.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "Weapons/UR_WeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_RangedWeaponInstance)

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Game_Weapon_SteadyAimingCamera, "OT.Weapon.SteadyAimingCamera");

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_RangedWeaponInstance::UUR_RangedWeaponInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    HeatToHeatPerShotCurve.EditorCurveData.AddKey(0.0f, 1.0f);
    HeatToCoolDownPerSecondCurve.EditorCurveData.AddKey(0.0f, 2.0f);
}

void UUR_RangedWeaponInstance::PostLoad()
{
    Super::PostLoad();

#if WITH_EDITOR
    UpdateDebugVisualization();
#endif
}

#if WITH_EDITOR
void UUR_RangedWeaponInstance::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    UpdateDebugVisualization();
}

void UUR_RangedWeaponInstance::UpdateDebugVisualization()
{
    ComputeHeatRange(/*out*/ Debug_MinHeat, /*out*/ Debug_MaxHeat);
    ComputeSpreadRange(/*out*/ Debug_MinSpreadAngle, /*out*/ Debug_MaxSpreadAngle);
    Debug_CurrentHeat = CurrentHeat;
    Debug_CurrentSpreadAngle = CurrentSpreadAngle;
    Debug_CurrentSpreadAngleMultiplier = CurrentSpreadAngleMultiplier;
}
#endif

void UUR_RangedWeaponInstance::OnEquipped()
{
    Super::OnEquipped();

    // Start heat in the middle
    float MinHeatRange;
    float MaxHeatRange;
    ComputeHeatRange(/*out*/ MinHeatRange, /*out*/ MaxHeatRange);
    CurrentHeat = (MinHeatRange + MaxHeatRange) * 0.5f;

    // Derive spread
    CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);

    // Default the multipliers to 1x
    CurrentSpreadAngleMultiplier = 1.0f;
    StandingStillMultiplier = 1.0f;
    JumpFallMultiplier = 1.0f;
    CrouchingMultiplier = 1.0f;
}

void UUR_RangedWeaponInstance::OnUnequipped()
{
    Super::OnUnequipped();
}

void UUR_RangedWeaponInstance::Tick(float DeltaSeconds)
{
    APawn* Pawn = GetPawn();
    check(Pawn != nullptr);

    const bool bMinSpread = UpdateSpread(DeltaSeconds);
    const bool bMinMultipliers = UpdateMultipliers(DeltaSeconds);

    bHasFirstShotAccuracy = bAllowFirstShotAccuracy && bMinMultipliers && bMinSpread;

#if WITH_EDITOR
    UpdateDebugVisualization();
#endif
}

void UUR_RangedWeaponInstance::ComputeHeatRange(float& MinHeat, float& MaxHeat)
{
    float Min1;
    float Max1;
    HeatToHeatPerShotCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min1, /*out*/ Max1);

    float Min2;
    float Max2;
    HeatToCoolDownPerSecondCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min2, /*out*/ Max2);

    float Min3;
    float Max3;
    HeatToSpreadCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min3, /*out*/ Max3);

    MinHeat = FMath::Min(FMath::Min(Min1, Min2), Min3);
    MaxHeat = FMath::Max(FMath::Max(Max1, Max2), Max3);
}

void UUR_RangedWeaponInstance::ComputeSpreadRange(float& MinSpread, float& MaxSpread)
{
    HeatToSpreadCurve.GetRichCurveConst()->GetValueRange(/*out*/ MinSpread, /*out*/ MaxSpread);
}

void UUR_RangedWeaponInstance::AddSpread()
{
    // Sample the heat up curve
    const float HeatPerShot = HeatToHeatPerShotCurve.GetRichCurveConst()->Eval(CurrentHeat);
    CurrentHeat = ClampHeat(CurrentHeat + HeatPerShot);

    // Map the heat to the spread angle
    CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);

#if WITH_EDITOR
    UpdateDebugVisualization();
#endif
}

float UUR_RangedWeaponInstance::GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
    const FRichCurve* Curve = DistanceDamageFalloff.GetRichCurveConst();
    return Curve->HasAnyData() ? Curve->Eval(Distance) : 1.0f;
}

float UUR_RangedWeaponInstance::GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
    float CombinedMultiplier = 1.0f;
    if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(PhysicalMaterial))
    {
        for (const FGameplayTag MaterialTag : PhysMatWithTags->Tags)
        {
            if (const float* TagMultiplier = MaterialDamageMultiplier.Find(MaterialTag))
            {
                CombinedMultiplier *= *TagMultiplier;
            }
        }
    }

    return CombinedMultiplier;
}

bool UUR_RangedWeaponInstance::UpdateSpread(float DeltaSeconds)
{
    const float TimeSinceFired = GetWorld()->TimeSince(LastFireTime);

    if (TimeSinceFired > SpreadRecoveryCooldownDelay)
    {
        const float CooldownRate = HeatToCoolDownPerSecondCurve.GetRichCurveConst()->Eval(CurrentHeat);
        CurrentHeat = ClampHeat(CurrentHeat - (CooldownRate * DeltaSeconds));
        CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);
    }

    float MinSpread;
    float MaxSpread;
    ComputeSpreadRange(/*out*/ MinSpread, /*out*/ MaxSpread);

    return FMath::IsNearlyEqual(CurrentSpreadAngle, MinSpread, KINDA_SMALL_NUMBER);
}

bool UUR_RangedWeaponInstance::UpdateMultipliers(float DeltaSeconds)
{
    constexpr float MultiplierNearlyEqualThreshold = 0.05f;

    APawn* Pawn = GetPawn();
    check(Pawn != nullptr);
    const UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());

    // See if we are standing still, and if so, smoothly apply the bonus
    const float PawnSpeed = Pawn->GetVelocity().Size();
    const float MovementTargetValue = FMath::GetMappedRangeValueClamped
    (
        /*InputRange=*/ FVector2D(StandingStillSpeedThreshold, StandingStillSpeedThreshold + StandingStillToMovingSpeedRange),
        /*OutputRange=*/
        FVector2D(SpreadAngleMultiplier_StandingStill, 1.0f),
        /*Alpha=*/
        PawnSpeed);
    StandingStillMultiplier = FMath::FInterpTo(StandingStillMultiplier, MovementTargetValue, DeltaSeconds, TransitionRate_StandingStill);
    const bool bStandingStillMultiplierAtMin = FMath::IsNearlyEqual(StandingStillMultiplier, SpreadAngleMultiplier_StandingStill, SpreadAngleMultiplier_StandingStill * 0.1f);

    // See if we are crouching, and if so, smoothly apply the bonus
    const bool bIsCrouching = (MovementComponent != nullptr) && MovementComponent->IsCrouching();
    const float CrouchingTargetValue = bIsCrouching ? SpreadAngleMultiplier_Crouching : 1.0f;
    CrouchingMultiplier = FMath::FInterpTo(CrouchingMultiplier, CrouchingTargetValue, DeltaSeconds, TransitionRate_Crouching);
    const bool bCrouchingMultiplierAtTarget = FMath::IsNearlyEqual(CrouchingMultiplier, CrouchingTargetValue, MultiplierNearlyEqualThreshold);

    // See if we are in the air (jumping/falling), and if so, smoothly apply the penalty
    const bool bIsJumpingOrFalling = (MovementComponent != nullptr) && MovementComponent->IsFalling();
    const float JumpFallTargetValue = bIsJumpingOrFalling ? SpreadAngleMultiplier_JumpingOrFalling : 1.0f;
    JumpFallMultiplier = FMath::FInterpTo(JumpFallMultiplier, JumpFallTargetValue, DeltaSeconds, TransitionRate_JumpingOrFalling);
    const bool bJumpFallMultiplierIs1 = FMath::IsNearlyEqual(JumpFallMultiplier, 1.0f, MultiplierNearlyEqualThreshold);

    // Determine if we are aiming down sights, and apply the bonus based on how far into the camera transition we are
    float AimingAlpha = 0.0f;
    if (const UUR_CameraComponent* CameraComponent = UUR_CameraComponent::FindCameraComponent(Pawn))
    {
        float TopCameraWeight;
        FGameplayTag TopCameraTag;
        CameraComponent->GetBlendInfo(/*out*/ TopCameraWeight, /*out*/ TopCameraTag);

        AimingAlpha = (TopCameraTag == TAG_Game_Weapon_SteadyAimingCamera) ? TopCameraWeight : 0.0f;
    }
    const float AimingMultiplier = FMath::GetMappedRangeValueClamped
    (
        /*InputRange=*/ FVector2D(0.0f, 1.0f),
        /*OutputRange=*/
        FVector2D(1.0f, SpreadAngleMultiplier_Aiming),
        /*Alpha=*/
        AimingAlpha);
    const bool bAimingMultiplierAtTarget = FMath::IsNearlyEqual(AimingMultiplier, SpreadAngleMultiplier_Aiming, KINDA_SMALL_NUMBER);

    // Combine all the multipliers
    const float CombinedMultiplier = AimingMultiplier * StandingStillMultiplier * CrouchingMultiplier * JumpFallMultiplier;
    CurrentSpreadAngleMultiplier = CombinedMultiplier;

    // need to handle these spread multipliers indicating we are not at min spread
    return bStandingStillMultiplierAtMin && bCrouchingMultiplierAtTarget && bJumpFallMultiplierIs1 && bAimingMultiplierAtTarget;
}
