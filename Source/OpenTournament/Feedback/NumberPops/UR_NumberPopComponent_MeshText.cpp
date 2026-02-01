// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_NumberPopComponent_MeshText.h"

#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/Package.h"

#include "Feedback/NumberPops/UR_DamagePopStyle.h"
#include "Feedback/NumberPops/UR_NumberPopComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_NumberPopComponent_MeshText)

/////////////////////////////////////////////////////////////////////////////////////////////////

class UStaticMesh;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_NumberPopComponent_MeshText::UUR_NumberPopComponent_MeshText(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ComponentLifespan = 1.f;

    SignDigitParameterName = FName(TEXT("+Or-"));
    ColorParameterName = FName(TEXT("Color"));
    AnimationLifespanParameterName = FName(TEXT("Animation Lifespan"));
    IsCriticalHitParameterName = FName(TEXT("isCriticalHit?"));
    MoveToCameraParameterName = FName(TEXT("MoveToCamera"));
    PositionParameterNames = { TEXT("0a"), TEXT("1a"), TEXT("2a"), TEXT("3a"), TEXT("4a"), TEXT("5a"), TEXT("6a"), TEXT("7a"), TEXT("8a") };
    ScaleRotationAngleParameterNames = { TEXT("0b"), TEXT("1b"), TEXT("2b"), TEXT("3b"), TEXT("4b"), TEXT("5b"), TEXT("6b"), TEXT("7b"), TEXT("8b") };
    DurationParameterNames = { TEXT("0c"), TEXT("1c"), TEXT("2c"), TEXT("3c"), TEXT("4c"), TEXT("5c"), TEXT("6c"), TEXT("7c"), TEXT("8c") };

    SpacingPercentageForOnes = 0.8f;

    DistanceFromCameraBeforeDoublingSize = 1024.f;
    CriticalHitSizeMultiplier = 1.7f;

    FontXSize = 10.920001f;
    FontYSize = 21.0f;

    NumberOfNumberRotations = 1.f;
}

void UUR_NumberPopComponent_MeshText::AddNumberPop(const FGameNumberPopRequest& NewRequest)
{
    // Drop requests for remote players on the floor
    // (this prevents multiple pops from showing up for the host of a listen server)
    if (const APlayerController* PC = GetController<APlayerController>())
    {
        if (!PC->IsLocalController())
        {
            return;
        }
    }

    FTempNumberPopInfo PreparedNumberInfo;

    // Prepare the DamageNumberArray with the digits from the damage.
    {
        int32 LocalDamage = NewRequest.NumberToDisplay;
        PreparedNumberInfo.DamageNumberArray.Empty();

        if (LocalDamage == 0)
        {
            // We want to just show a zero
            PreparedNumberInfo.DamageNumberArray.Insert(0, 0);
        }
        else
        {
            // Parse the base10 number into an array
            while (LocalDamage > 0)
            {
                PreparedNumberInfo.DamageNumberArray.Insert(LocalDamage % 10, 0);
                LocalDamage /= 10;
            }
        }

        // Insert a zero to reserve space for + or -. Used by the blueprint
        PreparedNumberInfo.DamageNumberArray.Insert(0, 0);
    }

    // Grab a component from the pool for this number or create one
    {
        UStaticMesh* MeshToUse = DetermineStaticMesh(NewRequest);
        if (MeshToUse == nullptr)
        {
            return;
        }

        FPooledNumberPopComponentList& ComponentPool = PooledComponentMap.FindOrAdd(MeshToUse);

        UStaticMeshComponent* ComponentToUse = nullptr;
        if (ComponentPool.Components.Num() > 0)
        {
            ComponentToUse = ComponentPool.Components.Pop();
        }
        else
        {
            ComponentToUse = NewObject<UStaticMeshComponent>(GetOwner());
            ComponentToUse->SetupAttachment(nullptr);
            ComponentToUse->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
            ComponentToUse->SetStaticMesh(MeshToUse);

            // Used to allow post-processes to opt out of affecting the number pop digits
            ComponentToUse->SetRenderCustomDepth(true);
            ComponentToUse->SetCustomDepthStencilValue(123);

            // The digits travel a great distance from their original bounds due to
            // world position offset (WPO) animation in the material, so expand bounds
            ComponentToUse->SetBoundsScale(2000.0f);

            // We'll be overriding values like the desired color and digits to use, so we need MIDs
            for (int32 MatIdx = 0; MatIdx < ComponentToUse->GetNumMaterials(); ++MatIdx)
            {
                ComponentToUse->CreateDynamicMaterialInstance(MatIdx);
            }
        }

        // Register
        check(ComponentToUse);
        ComponentToUse->RegisterComponent();

        // Add to the "live" list
        UWorld* LocalWorld = GetWorld();
        check(LocalWorld);
        LiveComponents.Emplace(ComponentToUse, &ComponentPool, LocalWorld->GetTimeSeconds() + ComponentLifespan);

        // Assign struct pointers
        PreparedNumberInfo.StaticMeshComponent = ComponentToUse;
        for (int32 MatIdx = 0; MatIdx < ComponentToUse->GetNumMaterials(); ++MatIdx)
        {
            UMaterialInstanceDynamic* NewMID = Cast<UMaterialInstanceDynamic>(ComponentToUse->GetMaterial(MatIdx));
            PreparedNumberInfo.MeshMIDs.Add(NewMID);
        }

        // Start the timer if it wasn't already running
        if (!LocalWorld->GetTimerManager().IsTimerActive(ReleaseTimerHandle))
        {
            LocalWorld->GetTimerManager().SetTimer(ReleaseTimerHandle, this, &ThisClass::ReleaseNextComponents, ComponentLifespan);
        }
    }

    // Determine the position
    FTransform CameraTransform;
    FVector NumberLocation(NewRequest.WorldLocation);
    if (APlayerController* PC = GetController<APlayerController>())
    {
        if (APlayerCameraManager* PlayerCameraManager = PC->PlayerCameraManager)
        {
            CameraTransform = FTransform(PlayerCameraManager->GetCameraRotation(), PlayerCameraManager->GetCameraLocation());

            FVector LocationOffset(ForceInitToZero);

            const float RandomMagnitude = 5.0f; //@TODO: Make this style driven
            LocationOffset += FMath::RandPointInBox(FBox(FVector(-RandomMagnitude), FVector(RandomMagnitude)));

            NumberLocation += LocationOffset;
        }
    }
    PreparedNumberInfo.StaticMeshComponent->SetWorldTransform(FTransform(CameraTransform.GetRotation(), NumberLocation));

    // Now apply the material parameters to make the digits, etc...
    SetMaterialParameters(NewRequest, PreparedNumberInfo, CameraTransform, NumberLocation);
}

void UUR_NumberPopComponent_MeshText::ReleaseNextComponents()
{
    UWorld* LocalWorld = GetWorld();
    check(LocalWorld);

    const float CurrentTime = LocalWorld->GetTimeSeconds();

    int32 NumReleased = 0;
    for (const FLiveNumberPopEntry& LiveComp : LiveComponents)
    {
        if (CurrentTime >= LiveComp.ReleaseTime)
        {
            NumReleased++;
            if (ensure(LiveComp.Component))
            {
                LiveComp.Component->UnregisterComponent();

                if (ensure(LiveComp.Pool))
                {
                    // Return this component to the pool
                    LiveComp.Pool->Components.Push(LiveComp.Component);
                }
                else
                {
                    // No pool. Just remove it.
                    LiveComp.Component->SetFlags(RF_Transient);
                    LiveComp.Component->Rename(nullptr, GetTransientPackage(), RF_NoFlags);
                }
            }
        }
        else
        {
            // These are in chronological order so none of the other elements will be deleted
            break;
        }
    }

    // Actually remove it from the live components array
    LiveComponents.RemoveAt(0, NumReleased);

    // If we still have live components animating, set the timer to remove the next one
    if (LiveComponents.Num() > 0)
    {
        const float TimeUntilNextRelease = LiveComponents[0].ReleaseTime - CurrentTime;
        LocalWorld->GetTimerManager().SetTimer(ReleaseTimerHandle, this, &ThisClass::ReleaseNextComponents, TimeUntilNextRelease);
    }
}

FLinearColor UUR_NumberPopComponent_MeshText::DetermineColor(const FGameNumberPopRequest& Request) const
{
    for (UUR_DamagePopStyle* Style : Styles)
    {
        if ((Style != nullptr) && Style->bOverrideColor)
        {
            if (Style->MatchPattern.Matches(Request.TargetTags))
            {
                return Request.bIsCriticalDamage ? Style->CriticalColor : Style->Color;
            }
        }
    }

    return FLinearColor::White;
}

UStaticMesh* UUR_NumberPopComponent_MeshText::DetermineStaticMesh(const FGameNumberPopRequest& Request) const
{
    for (UUR_DamagePopStyle* Style : Styles)
    {
        if ((Style != nullptr) && Style->bOverrideMesh)
        {
            if (Style->MatchPattern.Matches(Request.TargetTags))
            {
                return Style->TextMesh;
            }
        }
    }

    return nullptr;
}

void UUR_NumberPopComponent_MeshText::SetMaterialParameters(const FGameNumberPopRequest& Request, FTempNumberPopInfo& NewDamageNumberInfo, const FTransform& CameraTransform, const FVector& NumberLocation)
{
    UWorld* World = GetWorld();
    if (World && GEngine)
    {
        const float RealGameTime = World->GetRealTimeSeconds();

        // Whether we should show a sign as the first digit, and if so which one
        // (if bIsSignNegative is true, we show minus, false is plus)
        const bool bShouldShowSign = false;
        const bool bIsSignNegative = true;

        for (UMaterialInstanceDynamic* MeshMID : NewDamageNumberInfo.MeshMIDs)
        {
            MeshMID->SetScalarParameterValue(SignDigitParameterName, bIsSignNegative ? 0.5f : 0.0f);
            MeshMID->SetVectorParameterValue(ColorParameterName, DetermineColor(Request));

            // IF the damage number has more digits than we support
            // THEN force the damage number to the highest number we can support
            const int32 MaxSupportedDigits = FMath::Min(FMath::Min(PositionParameterNames.Num(), ScaleRotationAngleParameterNames.Num()), DurationParameterNames.Num());
            if (!ensure(NewDamageNumberInfo.DamageNumberArray.Num() <= MaxSupportedDigits))
            {
                NewDamageNumberInfo.DamageNumberArray.SetNum(MaxSupportedDigits);

                // Set all number digits to 9 so we show the largest number we can
                // Skip digit 0 because that digit is for the +/- sign
                for (int32 DigitIndex = 1; DigitIndex < NewDamageNumberInfo.DamageNumberArray.Num(); ++DigitIndex)
                {
                    NewDamageNumberInfo.DamageNumberArray[DigitIndex] = 9;
                }
            }

            MeshMID->SetScalarParameterValue(AnimationLifespanParameterName, ComponentLifespan);
            MeshMID->SetScalarParameterValue(IsCriticalHitParameterName, Request.bIsCriticalDamage ? 1.f : 0.f);

            const int32 DamageNumberArrayLength = NewDamageNumberInfo.DamageNumberArray.Num();
            float OffsetAccumulatedValue = (DamageNumberArrayLength * -1.f) + (bShouldShowSign ? 0.f : -1.f);

            const int32 LastIndex = (DamageNumberArrayLength >= 4) ? DamageNumberArrayLength : 4;

            for (int32 NumberIndex = 0; NumberIndex < LastIndex; ++NumberIndex)
            {
                const float NumberYOffset = ((NumberIndex / FMath::Max(1, DamageNumberArrayLength - 1)) - 0.5f) * 2.f;
                const FVector NumberOffset = FVector(0.f, NumberYOffset, 0.f);
                const FVector CameraSpaceDirection = CameraTransform.TransformVectorNoScale(NumberOffset);

                const float SpacingForNumber = ((NumberIndex < DamageNumberArrayLength) && ((NewDamageNumberInfo.DamageNumberArray[NumberIndex] == 1) || ((NumberIndex > 0) && (NewDamageNumberInfo.DamageNumberArray[NumberIndex - 1] == 1)))) ? SpacingPercentageForOnes : 1.f;
                OffsetAccumulatedValue += SpacingForNumber;

                FLinearColor RGBAPositionParameter(CameraSpaceDirection);
                RGBAPositionParameter.A = OffsetAccumulatedValue;

                const FName PositionParameterName = PositionParameterNames[NumberIndex];
                MeshMID->SetVectorParameterValue(PositionParameterName, RGBAPositionParameter);

                const float DistanceFromCameraToNumber = (CameraTransform.GetLocation() - NumberLocation).Size();
                const float DistanceSpriteScale = DistanceFromCameraBeforeDoublingSize == 0.f ? 1.f : FMath::Clamp(DistanceFromCameraToNumber / DistanceFromCameraBeforeDoublingSize, 1.f, 1000000000.f);

                const float ScaleToZeroMultiplier = (NumberIndex < DamageNumberArrayLength) && (((NumberIndex == 0) && bShouldShowSign) || (NumberIndex != 0)) ? 1.f : 0.f;

                const float HitSizeMultiplier = Request.bIsCriticalDamage ? CriticalHitSizeMultiplier : 1.f;
                const float FontSizeMultiplier = HitSizeMultiplier * DistanceSpriteScale * ScaleToZeroMultiplier;

                FLinearColor RGBAScaleRotationParameter;
                RGBAScaleRotationParameter.R = FontXSize * FontSizeMultiplier;
                RGBAScaleRotationParameter.G = FontYSize * FontSizeMultiplier;
                RGBAScaleRotationParameter.B = NewDamageNumberInfo.DamageNumberArray[FMath::Min(DamageNumberArrayLength - 1, NumberIndex)];
                RGBAScaleRotationParameter.A = FMath::Sign(CameraSpaceDirection.X) * NumberOfNumberRotations;

                const FName ScaleRotationAngleParameterName = ScaleRotationAngleParameterNames[NumberIndex];
                MeshMID->SetVectorParameterValue(ScaleRotationAngleParameterName, RGBAScaleRotationParameter);

                FLinearColor RGBADurationParameter;
                RGBADurationParameter.R = RealGameTime + ComponentLifespan;
                RGBADurationParameter.G = FMath::FRand();

                const FName DurationParameterName = DurationParameterNames[NumberIndex];
                MeshMID->SetVectorParameterValue(DurationParameterName, RGBADurationParameter);
            }

            // Non-gameplay cameras while spectating have more cinematic values of aperture as default.
            // This makes damage numbers very blurry as they are brought close to the camera, and away from the point of focus.
            // Disable the shifting of numbers towards the camera here, if in a cinematic spectator camera.
            //@TODO: Determine whether or not we are spectating
            const bool bIsSpectating = false;
            MeshMID->SetScalarParameterValue(MoveToCameraParameterName, bIsSpectating ? 0.0f : 1.0f);
        }
    }
}
