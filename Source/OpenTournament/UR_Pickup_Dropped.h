// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Pickup.h"

#include "UR_Pickup_Dropped.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UProjectileMovementComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(Blueprintable)
class OPENTOURNAMENT_API AUR_Pickup_Dropped : public AUR_Pickup
{
    GENERATED_BODY()

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual bool IsPickupPermitted(const AUR_Character* PickupCharacter) const override;
    virtual void Tick(float DeltaTime) override;

public:
    AUR_Pickup_Dropped(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UProjectileMovementComponent* ProjectileMovementComponent;

    /**
    * Timestamp the weapon was dropped at
    */
    UPROPERTY()
    float CreatedAt;

    /**
    * Remaining life span.
    * Updated sparsely (on server). Replicated initial-only for client.
    */
    UPROPERTY(Replicated, BlueprintReadOnly)
    float RemainingLifeSpan;
    FTimerHandle RefreshRemainingLifeSpanTimerHandle;

    UPROPERTY()
    float ExpireEffectDuration;

    UPROPERTY(BlueprintReadOnly)
    FTimerHandle ExpireTimerHandle;

    UFUNCTION()
    virtual void RefreshRemainingLifeSpan();

    UFUNCTION()
    virtual void OnProjectileStop(const FHitResult& ImpactResult);

    UFUNCTION()
    void PlayExpire();

    /** Expiration internals */
    UPROPERTY()
    bool bExpiring;
    FVector OriginalScale;
    float RotationRate;

    /**
    * Adjusted to work on both server & client
    */
    UFUNCTION(BlueprintPure)
    virtual float GetRemainingLifeSpan();
};
