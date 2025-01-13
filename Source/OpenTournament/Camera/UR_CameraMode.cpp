// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CameraMode.h"

#include <Components/CapsuleComponent.h>
#include <Engine/Canvas.h>
#include <GameFramework/Character.h>

#include "UR_CameraComponent.h"
#include "UR_PlayerCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_CameraMode)

/////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// FGameCameraModeView
//////////////////////////////////////////////////////////////////////////
FGameCameraModeView::FGameCameraModeView()
    : Location(ForceInit)
  , Rotation(ForceInit)
  , ControlRotation(ForceInit)
  , FieldOfView(OT_CAMERA_DEFAULT_FOV)
{
}

void FGameCameraModeView::Blend(const FGameCameraModeView& Other, float OtherWeight)
{
    if (OtherWeight <= 0.0f)
    {
        return;
    }
    else if (OtherWeight >= 1.0f)
    {
        *this = Other;
        return;
    }

    Location = FMath::Lerp(Location, Other.Location, OtherWeight);

    const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
    Rotation = Rotation + (OtherWeight * DeltaRotation);

    const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
    ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

    FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}


//////////////////////////////////////////////////////////////////////////
// UUR_CameraMode
//////////////////////////////////////////////////////////////////////////
UUR_CameraMode::UUR_CameraMode()
{
    FieldOfView = OT_CAMERA_DEFAULT_FOV;
    ViewPitchMin = OT_CAMERA_DEFAULT_PITCH_MIN;
    ViewPitchMax = OT_CAMERA_DEFAULT_PITCH_MAX;

    BlendTime = 0.5f;
    BlendFunction = EGameCameraModeBlendFunction::EaseOut;
    BlendExponent = 4.0f;
    BlendAlpha = 1.0f;
    BlendWeight = 1.0f;
}

UUR_CameraComponent* UUR_CameraMode::GetGameCameraComponent() const
{
    return CastChecked<UUR_CameraComponent>(GetOuter());
}

UWorld* UUR_CameraMode::GetWorld() const
{
    return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UUR_CameraMode::GetTargetActor() const
{
    const UUR_CameraComponent* GameCameraComponent = GetGameCameraComponent();

    return GameCameraComponent->GetTargetActor();
}

FVector UUR_CameraMode::GetPivotLocation() const
{
    const AActor* TargetActor = GetTargetActor();
    check(TargetActor);

    if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
    {
        // Height adjustments for characters to account for crouching.
        if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
        {
            const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
            check(TargetCharacterCDO);

            const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
            check(CapsuleComp);

            const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
            check(CapsuleCompCDO);

            const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
            const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
            const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;

            return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
        }

        return TargetPawn->GetPawnViewLocation();
    }

    return TargetActor->GetActorLocation();
}

FRotator UUR_CameraMode::GetPivotRotation() const
{
    const AActor* TargetActor = GetTargetActor();
    check(TargetActor);

    if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
    {
        return TargetPawn->GetViewRotation();
    }

    return TargetActor->GetActorRotation();
}

void UUR_CameraMode::UpdateCameraMode(float DeltaTime)
{
    UpdateView(DeltaTime);
    UpdateBlending(DeltaTime);
}

void UUR_CameraMode::UpdateView(float DeltaTime)
{
    FVector PivotLocation = GetPivotLocation();
    FRotator PivotRotation = GetPivotRotation();

    PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

    View.Location = PivotLocation;
    View.Rotation = PivotRotation;
    View.ControlRotation = View.Rotation;
    View.FieldOfView = FieldOfView;
}

void UUR_CameraMode::SetBlendWeight(float Weight)
{
    BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

    // Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
    const float InvExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

    switch (BlendFunction)
    {
        case EGameCameraModeBlendFunction::Linear:
            BlendAlpha = BlendWeight;
            break;

        case EGameCameraModeBlendFunction::EaseIn:
            BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
            break;

        case EGameCameraModeBlendFunction::EaseOut:
            BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
            break;

        case EGameCameraModeBlendFunction::EaseInOut:
            BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
            break;

        default:
            checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
            break;
    }
}

void UUR_CameraMode::UpdateBlending(float DeltaTime)
{
    if (BlendTime > 0.0f)
    {
        BlendAlpha += (DeltaTime / BlendTime);
        BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
    }
    else
    {
        BlendAlpha = 1.0f;
    }

    const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

    switch (BlendFunction)
    {
        case EGameCameraModeBlendFunction::Linear:
            BlendWeight = BlendAlpha;
            break;

        case EGameCameraModeBlendFunction::EaseIn:
            BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
            break;

        case EGameCameraModeBlendFunction::EaseOut:
            BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
            break;

        case EGameCameraModeBlendFunction::EaseInOut:
            BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
            break;

        default:
            checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
            break;
    }
}

void UUR_CameraMode::DrawDebug(UCanvas* Canvas) const
{
    check(Canvas);

    FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

    DisplayDebugManager.SetDrawColor(FColor::White);
    DisplayDebugManager.DrawString(FString::Printf(TEXT("      GameCameraMode: %s (%f)"), *GetName(), BlendWeight));
}


//////////////////////////////////////////////////////////////////////////
// UUR_CameraModeStack
//////////////////////////////////////////////////////////////////////////
UUR_CameraModeStack::UUR_CameraModeStack()
{
    bIsActive = true;
}

void UUR_CameraModeStack::ActivateStack()
{
    if (!bIsActive)
    {
        bIsActive = true;

        // Notify camera modes that they are being activated.
        for (UUR_CameraMode* CameraMode : CameraModeStack)
        {
            check(CameraMode);
            CameraMode->OnActivation();
        }
    }
}

void UUR_CameraModeStack::DeactivateStack()
{
    if (bIsActive)
    {
        bIsActive = false;

        // Notify camera modes that they are being deactivated.
        for (UUR_CameraMode* CameraMode : CameraModeStack)
        {
            check(CameraMode);
            CameraMode->OnDeactivation();
        }
    }
}

void UUR_CameraModeStack::PushCameraMode(TSubclassOf<UUR_CameraMode> CameraModeClass)
{
    if (!CameraModeClass)
    {
        return;
    }

    UUR_CameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);
    check(CameraMode);

    int32 StackSize = CameraModeStack.Num();

    if ((StackSize > 0) && (CameraModeStack[0] == CameraMode))
    {
        // Already top of stack.
        return;
    }

    // See if it's already in the stack and remove it.
    // Figure out how much it was contributing to the stack.
    int32 ExistingStackIndex = INDEX_NONE;
    float ExistingStackContribution = 1.0f;

    for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
    {
        if (CameraModeStack[StackIndex] == CameraMode)
        {
            ExistingStackIndex = StackIndex;
            ExistingStackContribution *= CameraMode->GetBlendWeight();
            break;
        }
        else
        {
            ExistingStackContribution *= (1.0f - CameraModeStack[StackIndex]->GetBlendWeight());
        }
    }

    if (ExistingStackIndex != INDEX_NONE)
    {
        CameraModeStack.RemoveAt(ExistingStackIndex);
        StackSize--;
    }
    else
    {
        ExistingStackContribution = 0.0f;
    }

    // Decide what initial weight to start with.
    const bool bShouldBlend = ((CameraMode->GetBlendTime() > 0.0f) && (StackSize > 0));
    const float BlendWeight = (bShouldBlend ? ExistingStackContribution : 1.0f);

    CameraMode->SetBlendWeight(BlendWeight);

    // Add new entry to top of stack.
    CameraModeStack.Insert(CameraMode, 0);

    // Make sure stack bottom is always weighted 100%.
    CameraModeStack.Last()->SetBlendWeight(1.0f);

    // Let the camera mode know if it's being added to the stack.
    if (ExistingStackIndex == INDEX_NONE)
    {
        CameraMode->OnActivation();
    }
}

bool UUR_CameraModeStack::EvaluateStack(float DeltaTime, FGameCameraModeView& OutCameraModeView)
{
    if (!bIsActive)
    {
        return false;
    }

    UpdateStack(DeltaTime);
    BlendStack(OutCameraModeView);

    return true;
}

UUR_CameraMode* UUR_CameraModeStack::GetCameraModeInstance(TSubclassOf<UUR_CameraMode> CameraModeClass)
{
    check(CameraModeClass);

    // First see if we already created one.
    for (UUR_CameraMode* CameraMode : CameraModeInstances)
    {
        if ((CameraMode != nullptr) && (CameraMode->GetClass() == CameraModeClass))
        {
            return CameraMode;
        }
    }

    // Not found, so we need to create it.
    UUR_CameraMode* NewCameraMode = NewObject<UUR_CameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
    check(NewCameraMode);

    CameraModeInstances.Add(NewCameraMode);

    return NewCameraMode;
}

void UUR_CameraModeStack::UpdateStack(float DeltaTime)
{
    const int32 StackSize = CameraModeStack.Num();
    if (StackSize <= 0)
    {
        return;
    }

    int32 RemoveCount = 0;
    int32 RemoveIndex = INDEX_NONE;

    for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
    {
        UUR_CameraMode* CameraMode = CameraModeStack[StackIndex];
        check(CameraMode);

        CameraMode->UpdateCameraMode(DeltaTime);

        if (CameraMode->GetBlendWeight() >= 1.0f)
        {
            // Everything below this mode is now irrelevant and can be removed.
            RemoveIndex = (StackIndex + 1);
            RemoveCount = (StackSize - RemoveIndex);
            break;
        }
    }

    if (RemoveCount > 0)
    {
        // Let the camera modes know they being removed from the stack.
        for (int32 StackIndex = RemoveIndex; StackIndex < StackSize; ++StackIndex)
        {
            UUR_CameraMode* CameraMode = CameraModeStack[StackIndex];
            check(CameraMode);

            CameraMode->OnDeactivation();
        }

        CameraModeStack.RemoveAt(RemoveIndex, RemoveCount);
    }
}

void UUR_CameraModeStack::BlendStack(FGameCameraModeView& OutCameraModeView) const
{
    const int32 StackSize = CameraModeStack.Num();
    if (StackSize <= 0)
    {
        return;
    }

    // Start at the bottom and blend up the stack
    const UUR_CameraMode* CameraMode = CameraModeStack[StackSize - 1];
    check(CameraMode);

    OutCameraModeView = CameraMode->GetCameraModeView();

    for (int32 StackIndex = (StackSize - 2); StackIndex >= 0; --StackIndex)
    {
        CameraMode = CameraModeStack[StackIndex];
        check(CameraMode);

        OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
    }
}

void UUR_CameraModeStack::DrawDebug(UCanvas* Canvas) const
{
    check(Canvas);

    FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

    DisplayDebugManager.SetDrawColor(FColor::Green);
    DisplayDebugManager.DrawString(FString(TEXT("   --- Camera Modes (Begin) ---")));

    for (const UUR_CameraMode* CameraMode : CameraModeStack)
    {
        check(CameraMode);
        CameraMode->DrawDebug(Canvas);
    }

    DisplayDebugManager.SetDrawColor(FColor::Green);
    DisplayDebugManager.DrawString(FString::Printf(TEXT("   --- Camera Modes (End) ---")));
}

void UUR_CameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
    if (CameraModeStack.Num() == 0)
    {
        OutWeightOfTopLayer = 1.0f;
        OutTagOfTopLayer = FGameplayTag();
        return;
    }
    else
    {
        UUR_CameraMode* TopEntry = CameraModeStack.Last();
        check(TopEntry);
        OutWeightOfTopLayer = TopEntry->GetBlendWeight();
        OutTagOfTopLayer = TopEntry->GetCameraTypeTag();
    }
}
