// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "Templates/SubclassOf.h"

#include "UR_GlobalAbilitySystem.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UGameplayAbility;
class UGameplayEffect;
class UUR_AbilitySystemComponent;
class UObject;
struct FActiveGameplayEffectHandle;
struct FFrame;
struct FGameplayAbilitySpecHandle;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UUR_AbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayAbility> Ability, UUR_AbilitySystemComponent* ASC);
	void RemoveFromASC(UUR_AbilitySystemComponent* ASC);
	void RemoveFromAll();
};

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UUR_AbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayEffect> Effect, UUR_AbilitySystemComponent* ASC);
	void RemoveFromASC(UUR_AbilitySystemComponent* ASC);
	void RemoveFromAll();
};

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_GlobalAbilitySystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UUR_GlobalAbilitySystem();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="OT")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="OT")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "OT")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "OT")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/** Register an ASC with global system and apply any active global effects/abilities. */
	void RegisterASC(UUR_AbilitySystemComponent* ASC);

	/** Removes an ASC from the global system, along with any active global effects/abilities. */
	void UnregisterASC(UUR_AbilitySystemComponent* ASC);

private:
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	UPROPERTY()
	TArray<TObjectPtr<UUR_AbilitySystemComponent>> RegisteredASCs;
};
