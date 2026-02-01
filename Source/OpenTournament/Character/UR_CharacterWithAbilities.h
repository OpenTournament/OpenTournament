// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Character/UR_Character.h"

#include "UR_CharacterWithAbilities.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAbilitySystemComponent;
class UUR_AbilitySystemComponent;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

// AUR_Character typically gets the ability system component from the possessing player state
// This represents a character with a self-contained ability system component.
UCLASS(MinimalAPI, Blueprintable)
class AUR_CharacterWithAbilities : public AUR_Character
{
    GENERATED_BODY()

public:
    UE_API AUR_CharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

    UE_API virtual void PostInitializeComponents() override;

    UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
    // @! TODO : Variables commented out as they shadow properties in the parent class

    // // The ability system component sub-object used by player characters.
    // UPROPERTY(VisibleAnywhere, Category = "OT|PlayerState")
    // TObjectPtr<UUR_AbilitySystemComponent> MyAbilitySystemComponent;
    //
    // // Health attribute set used by this actor.
    // UPROPERTY()
    // TObjectPtr<const class UUR_HealthSet> HealthSet;
    // // Combat attribute set used by this actor.
    // UPROPERTY()
    // TObjectPtr<const class UUR_CombatSet> CombatSet;
};

#undef UE_API
