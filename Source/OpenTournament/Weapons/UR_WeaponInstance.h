// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Cosmetics/UR_CosmeticAnimationTypes.h"
#include "Equipment/UR_EquipmentInstance.h"
#include "GameFramework/InputDevicePropertyHandle.h"

#include "UR_WeaponInstance.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

class UAnimInstance;
class UObject;
struct FFrame;
struct FGameplayTagContainer;
class UInputDeviceProperty;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_WeaponInstance
 *
 * A piece of equipment representing a weapon spawned and applied to a pawn
 */
UCLASS(MinimalAPI)
class UUR_WeaponInstance : public UUR_EquipmentInstance
{
    GENERATED_BODY()

public:
    UE_API UUR_WeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~UUR_EquipmentInstance interface
    UE_API virtual void OnEquipped() override;
    UE_API virtual void OnUnequipped() override;
    //~End of UUR_EquipmentInstance interface

    UFUNCTION(BlueprintCallable)
    UE_API void UpdateFiringTime();

    // Returns how long it's been since the weapon was interacted with (fired or equipped)
    UFUNCTION(BlueprintPure)
    UE_API float GetTimeSinceLastInteractedWith() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animation)
    FUR_AnimLayerSelectionSet EquippedAnimSet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animation)
    FUR_AnimLayerSelectionSet UnequippedAnimSet;

    /**
     * Device properties that should be applied while this weapon is equipped.
     * These properties will be played in with the "Looping" flag enabled, so they will
     * play continuously until this weapon is unequipped!
     */
    UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Input Devices")
    TArray<TObjectPtr<UInputDeviceProperty>> ApplicableDeviceProperties;

    // Choose the best layer from EquippedAnimSet or UnequippedAnimSet based on the specified gameplay tags
    UFUNCTION(BlueprintCallable, BlueprintPure=false, Category=Animation)
    UE_API TSubclassOf<UAnimInstance> PickBestAnimLayer(bool bEquipped, const FGameplayTagContainer& CosmeticTags) const;

    /** Returns the owning Pawn's Platform User ID */
    UFUNCTION(BlueprintCallable)
    UE_API const FPlatformUserId GetOwningUserId() const;

    /** Callback for when the owning pawn of this weapon dies. Removes all spawned device properties. */
    UFUNCTION()
    UE_API void OnDeathStarted(AActor* OwningActor);

    /**
     * Apply the ApplicableDeviceProperties to the owning pawn of this weapon.
     * Populate the DevicePropertyHandles so that they can be removed later. This will
     * Play the device properties in Looping mode so that they will share the lifetime of the
     * weapon being Equipped.
     */
    UE_API void ApplyDeviceProperties();

    /** Remove any device properties that were activated in ApplyDeviceProperties. */
    UE_API void RemoveDeviceProperties();

private:
    /** Set of device properties activated by this weapon. Populated by ApplyDeviceProperties */
    UPROPERTY(Transient)
    TSet<FInputDevicePropertyHandle> DevicePropertyHandles;

    double TimeLastEquipped = 0.0;
    double TimeLastFired = 0.0;
};

#undef UE_API
