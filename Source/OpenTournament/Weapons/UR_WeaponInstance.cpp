// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_WeaponInstance.h"

#include <Engine/World.h>
#include <GameFramework/InputDeviceProperties.h>
#include <GameFramework/InputDeviceSubsystem.h>
#include <GameFramework/Pawn.h>
#include <Math/UnrealMathUtility.h>
#include <Misc/AssertionMacros.h>

#include "Character/UR_HealthComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_WeaponInstance)

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAnimInstance;
struct FGameplayTagContainer;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_WeaponInstance::UUR_WeaponInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Listen for death of the owning pawn so that any device properties can be removed if we
    // die and can't unequip
    if (APawn* Pawn = GetPawn())
    {
        // We only need to do this for player controlled pawns, since AI and others won't have input devices on the client
        if (Pawn->IsPlayerControlled())
        {
            if (UUR_HealthComponent* HealthComponent = UUR_HealthComponent::FindHealthComponent(GetPawn()))
            {
                HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
            }
        }
    }
}

void UUR_WeaponInstance::OnEquipped()
{
    Super::OnEquipped();

    UWorld* World = GetWorld();
    check(World);
    TimeLastEquipped = World->GetTimeSeconds();

    ApplyDeviceProperties();
}

void UUR_WeaponInstance::OnUnequipped()
{
    Super::OnUnequipped();

    RemoveDeviceProperties();
}

void UUR_WeaponInstance::UpdateFiringTime()
{
    UWorld* World = GetWorld();
    check(World);
    TimeLastFired = World->GetTimeSeconds();
}

float UUR_WeaponInstance::GetTimeSinceLastInteractedWith() const
{
    UWorld* World = GetWorld();
    check(World);
    const double WorldTime = World->GetTimeSeconds();

    double Result = WorldTime - TimeLastEquipped;

    if (TimeLastFired > 0.0)
    {
        const double TimeSinceFired = WorldTime - TimeLastFired;
        Result = FMath::Min(Result, TimeSinceFired);
    }

    return Result;
}

TSubclassOf<UAnimInstance> UUR_WeaponInstance::PickBestAnimLayer(bool bEquipped, const FGameplayTagContainer& CosmeticTags) const
{
    const FUR_AnimLayerSelectionSet& SetToQuery = (bEquipped ? EquippedAnimSet : UnequippedAnimSet);
    return SetToQuery.SelectBestLayer(CosmeticTags);
}

const FPlatformUserId UUR_WeaponInstance::GetOwningUserId() const
{
    if (const APawn* Pawn = GetPawn())
    {
        return Pawn->GetPlatformUserId();
    }
    return PLATFORMUSERID_NONE;
}

void UUR_WeaponInstance::ApplyDeviceProperties()
{
    const FPlatformUserId UserId = GetOwningUserId();

    if (UserId.IsValid())
    {
        if (UInputDeviceSubsystem* InputDeviceSubsystem = UInputDeviceSubsystem::Get())
        {
            for (TObjectPtr<UInputDeviceProperty>& DeviceProp : ApplicableDeviceProperties)
            {
                FActivateDevicePropertyParams Params = { };
                Params.UserId = UserId;

                // By default, the device property will be played on the Platform User's Primary Input Device.
                // If you want to override this and set a specific device, then you can set the DeviceId parameter.
                //Params.DeviceId = <some specific device id>;

                // Don't remove this property it was evaluated. We want the properties to be applied as long as we are holding the
                // weapon, and will remove them manually in OnUnequipped
                Params.bLooping = true;

                DevicePropertyHandles.Emplace(InputDeviceSubsystem->ActivateDeviceProperty(DeviceProp, Params));
            }
        }
    }
}

void UUR_WeaponInstance::RemoveDeviceProperties()
{
    const FPlatformUserId UserId = GetOwningUserId();

    if (UserId.IsValid() && !DevicePropertyHandles.IsEmpty())
    {
        // Remove any device properties that have been applied
        if (UInputDeviceSubsystem* InputDeviceSubsystem = UInputDeviceSubsystem::Get())
        {
            InputDeviceSubsystem->RemoveDevicePropertyHandles(DevicePropertyHandles);
            DevicePropertyHandles.Empty();
        }
    }
}

void UUR_WeaponInstance::OnDeathStarted(AActor* OwningActor)
{
    // Remove any possibly active device properties when we die to make sure that there aren't any lingering around
    RemoveDeviceProperties();
}
