// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameplayEffectTypes.h"

#include "UR_GameplayEffectContext.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class FArchive;
class IUR_AbilitySourceInterface;
class UObject;
class UPhysicalMaterial;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FUR_GameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_BODY()

    FUR_GameplayEffectContext()
        : FGameplayEffectContext()
    {
    }

    FUR_GameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
        : FGameplayEffectContext(InInstigator, InEffectCauser)
    {
    }

    /** Returns the wrapped FUR_GameplayEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
    static OPENTOURNAMENT_API FUR_GameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

    /** Sets the object used as the ability source */
    void SetAbilitySource(const IUR_AbilitySourceInterface* InObject, float InSourceLevel);

    /** Returns the ability source interface associated with the source object. Only valid on the authority. */
    const IUR_AbilitySourceInterface* GetAbilitySource() const;

    virtual FGameplayEffectContext* Duplicate() const override
    {
        FUR_GameplayEffectContext* NewContext = new FUR_GameplayEffectContext();
        *NewContext = *this;
        if (GetHitResult())
        {
            // Does a deep copy of the hit result
            NewContext->AddHitResult(*GetHitResult(), true);
        }
        return NewContext;
    }

    virtual UScriptStruct* GetScriptStruct() const override
    {
        return FUR_GameplayEffectContext::StaticStruct();
    }

    /** Overridden to serialize new fields */
    virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

    /** Returns the physical material from the hit result if there is one */
    const UPhysicalMaterial* GetPhysicalMaterial() const;

public:
    /** ID to allow the identification of multiple bullets that were part of the same cartridge */
    UPROPERTY()
    int32 CartridgeID = -1;

protected:
    /** Ability Source object (should implement IUR_AbilitySourceInterface). NOT replicated currently */
    UPROPERTY()
    TWeakObjectPtr<const UObject> AbilitySourceObject;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct TStructOpsTypeTraits<FUR_GameplayEffectContext> : public TStructOpsTypeTraitsBase2<FUR_GameplayEffectContext>
{
    enum
    {
        WithNetSerializer = true,
        WithCopy = true
    };
};
