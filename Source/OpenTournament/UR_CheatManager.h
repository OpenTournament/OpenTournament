// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"

#include "UR_CheatManager.generated.h"

#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER (1 && !UE_BUILD_SHIPPING)
#endif // #ifndef USING_CHEAT_MANAGER

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(LogGameCheat, Log, All);

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Weapon;
class UUR_AbilitySystemComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Cheat Manager
 */
UCLASS(Config = Game)
class OPENTOURNAMENT_API UUR_CheatManager : public UCheatManager
{
    GENERATED_BODY()

    UUR_CheatManager();

    virtual void InitCheatManager() override;

    // Runs a cheat on the server for the owning player.
    UFUNCTION(exec)
    void Cheat(const FString& Msg);

    // Runs a cheat on the server for the all players.
    UFUNCTION(exec)
    void CheatAll(const FString& Msg);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Prevents the owning player from taking any damage.
    virtual void God() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Prevents the owning player from dropping below 1 health.
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    virtual void UnlimitedHealth(int32 Enabled = -1);

    UFUNCTION(exec, Category = "Cheat")
    void Cheat_GameReload();

    UFUNCTION(exec, Category = "Cheat")
    void Cheat_Loaded();

    UPROPERTY(Config)
    TArray<TSoftClassPtr<AUR_Weapon>> LoadedWeaponClasses;

    UFUNCTION(exec, Category = "Cheat")
    void Cheat_AddScore(int32 InValue = 1);

    // Applies the specified damage amount to the owning player.
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    virtual void DamageSelf(float DamageAmount);

    // Applies the specified damage amount to the actor that the player is looking at.
    virtual void DamageTarget(float DamageAmount) override;

    // Applies the specified amount of healing to the owning player.
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    virtual void HealSelf(float HealAmount);

    // Applies the specified amount of healing to the actor that the player is looking at.
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    virtual void HealTarget(float HealAmount);

    // Applies enough damage to kill the owning player.
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    virtual void DamageSelfDestruct();

protected:
    void ApplySetByCallerDamage(UUR_AbilitySystemComponent* InASC, float DamageAmount);

    void ApplySetByCallerHeal(UUR_AbilitySystemComponent* InASC, float HealAmount);

    UUR_AbilitySystemComponent* GetPlayerAbilitySystemComponent() const;
};
