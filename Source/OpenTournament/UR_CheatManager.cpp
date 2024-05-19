// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CheatManager.h"

#include <AbilitySystemGlobals.h>

#include "UR_AbilitySystemComponent.h"
#include "UR_AssetManager.h"
#include "UR_Character.h"
#include "UR_DeveloperSettings.h"
#include "UR_GameData.h"
#include "UR_GameplayTags.h"
#include "UR_HealthComponent.h"
#include "UR_InventoryComponent.h"
#include "UR_PawnExtensionComponent.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"
#include "UR_Weapon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_CheatManager)

DEFINE_LOG_CATEGORY(LogGameCheat);

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace OpenTournamentCheat
{
    static bool bStartInGodMode = false;
    static FAutoConsoleVariableRef CVarStartInGodMode(
        TEXT("LyraCheat.StartInGodMode"),
        bStartInGodMode,
        TEXT("If true then the God cheat will be applied on begin play"),
        ECVF_Cheat);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_CheatManager::UUR_CheatManager()
{
    //
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_CheatManager::InitCheatManager()
{
    Super::InitCheatManager();

#if WITH_EDITOR
    if (GIsEditor)
    {
        APlayerController* PC = GetOuterAPlayerController();
        for (const FUR_CheatToRun& CheatRow : GetDefault<UUR_DeveloperSettings>()->CheatsToRun)
        {
            if (CheatRow.Phase == ECheatExecutionTime::OnCheatManagerCreated)
            {
                PC->ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
            }
        }
    }
#endif

    if (OpenTournamentCheat::bStartInGodMode)
    {
        God();
    }
}

void UUR_CheatManager::Cheat(const FString& Msg)
{
    if (AUR_PlayerController* PC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        PC->ServerCheat(Msg.Left(128));
    }
}

void UUR_CheatManager::CheatAll(const FString& Msg)
{
    if (AUR_PlayerController* PC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        PC->ServerCheatAll(Msg.Left(128));
    }
}

void UUR_CheatManager::God()
{
    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (GamePC->GetNetMode() == NM_Client)
        {
            // Automatically send cheat to server for convenience.
            GamePC->ServerCheat(FString::Printf(TEXT("God")));
            return;
        }

        if (UUR_AbilitySystemComponent* GameASC = GamePC->GetGameAbilitySystemComponent())
        {
            const FGameplayTag Tag = URGameplayTags::Cheat_GodMode;
            const bool bHasTag = GameASC->HasMatchingGameplayTag(Tag);

            if (bHasTag)
            {
                GameASC->RemoveDynamicTagGameplayEffect(Tag);
            }
            else
            {
                GameASC->AddDynamicTagGameplayEffect(Tag);
            }
        }
    }
}

// void UUR_CheatManager::God()
// {
//     if (AUR_Character* URCharacter = Cast<AUR_Character>(GetOuterAPlayerController()->GetPawn()))
//     {
//         if (URCharacter->CanBeDamaged())
//         {
//             GetOuterAPlayerController()->ClientMessage(TEXT("God mode on"));
//         }
//         else
//         {
//             GetOuterAPlayerController()->ClientMessage(TEXT("God Mode off"));
//         }
//     }
//
//     // This handles the heavy lifting hooking into CanBeDamaged
//     Super::God();
// }

void UUR_CheatManager::Cheat_Loaded()
{
    if (AUR_Character* URCharacter = Cast<AUR_Character>(GetOuterAPlayerController()->GetPawn()))
    {
        if (UUR_InventoryComponent* Inventory = URCharacter->FindComponentByClass<UUR_InventoryComponent>())
        {
            for (const auto& WeaponClass : LoadedWeaponClasses)
            {
                if (WeaponClass)
                {
                    const auto World = URCharacter->GetWorld();
                    auto SpawnedWeapon = World->SpawnActor<AUR_Weapon>(WeaponClass.Get());
                    Inventory->AddWeapon(SpawnedWeapon);
                }
            }
        }
    }
}

void UUR_CheatManager::Cheat_AddScore(int32 InValue)
{
    if (const auto PC = Cast<AUR_PlayerController>(GetOuter()))
    {
        if (const auto PS = Cast<AUR_PlayerState>(PC->PlayerState))
        {
            PS->SetScore(PS->GetScore() + InValue);
        }
    }
}

void UUR_CheatManager::Cheat_HurtPlayer(int32 InDamage)
{
    if (const auto PC = Cast<AUR_PlayerController>(GetOuter()))
    {
        if (const auto Character = Cast<AUR_Character>(PC->GetCharacter()))
        {
            if (auto ASC = Character->GetGameAbilitySystemComponent())
            {
                //FGameplayEffectContextHandle GEContext;
                //auto GESpec = ASC->MakeOutgoingSpec()
                //ASC->ApplyGameplayEffectSpecToSelf()
            }
        }

        // @! TODO : Do we want ASC on PS?
        // if (auto PS = Cast<AUR_PlayerState>(PC->PlayerState))
        // {
        //     PS->SetScore(PS->GetScore() + InValue);
        // }
    }
}

void UUR_CheatManager::Cheat_HealPlayer(int32 InHeal)
{
}

void UUR_CheatManager::DamageSelf(float DamageAmount)
{
    if (UUR_AbilitySystemComponent* GameASC = GetPlayerAbilitySystemComponent())
    {
        ApplySetByCallerDamage(GameASC, DamageAmount);
    }
}

void UUR_CheatManager::DamageTarget(float DamageAmount)
{
    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (GamePC->GetNetMode() == NM_Client)
        {
            // Automatically send cheat to server for convenience.
            GamePC->ServerCheat(FString::Printf(TEXT("DamageTarget %.2f"), DamageAmount));
            return;
        }

        FHitResult TargetHitResult;
        AActor* TargetActor = GetTarget(GamePC, TargetHitResult);

        if (UUR_AbilitySystemComponent* UR_TargetASC = Cast<UUR_AbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor)))
        {
            ApplySetByCallerDamage(UR_TargetASC, DamageAmount);
        }
    }
}

void UUR_CheatManager::ApplySetByCallerDamage(UUR_AbilitySystemComponent* GameASC, float DamageAmount)
{
    check(GameASC);

    const TSubclassOf<UGameplayEffect> DamageGE = UUR_AssetManager::GetSubclass(UUR_GameData::Get().DamageGameplayEffect_SetByCaller);
    const FGameplayEffectSpecHandle SpecHandle = GameASC->MakeOutgoingSpec(DamageGE, 1.0f, GameASC->MakeEffectContext());

    if (SpecHandle.IsValid())
    {
        SpecHandle.Data->SetSetByCallerMagnitude(URGameplayTags::SetByCaller_Damage, DamageAmount);
        GameASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void UUR_CheatManager::HealSelf(float HealAmount)
{
    if (UUR_AbilitySystemComponent* GameASC = GetPlayerAbilitySystemComponent())
    {
        ApplySetByCallerHeal(GameASC, HealAmount);
    }
}

void UUR_CheatManager::HealTarget(float HealAmount)
{
    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        FHitResult TargetHitResult;
        const AActor* TargetActor = GetTarget(GamePC, TargetHitResult);

        if (UUR_AbilitySystemComponent* UR_TargetASC = Cast<UUR_AbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor)))
        {
            ApplySetByCallerHeal(UR_TargetASC, HealAmount);
        }
    }
}

void UUR_CheatManager::ApplySetByCallerHeal(UUR_AbilitySystemComponent* InASC, float HealAmount)
{
    check(InASC);

    TSubclassOf<UGameplayEffect> HealGE = UUR_AssetManager::GetSubclass(UUR_GameData::Get().HealGameplayEffect_SetByCaller);
    FGameplayEffectSpecHandle SpecHandle = InASC->MakeOutgoingSpec(HealGE, 1.0f, InASC->MakeEffectContext());

    if (SpecHandle.IsValid())
    {
        SpecHandle.Data->SetSetByCallerMagnitude(URGameplayTags::SetByCaller_Heal, HealAmount);
        InASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

UUR_AbilitySystemComponent* UUR_CheatManager::GetPlayerAbilitySystemComponent() const
{
    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        // @! TODO : Move ASC to PlayerState, e.g. as follows...
        //return GamePC->GetGameAbilitySystemComponent();

        if (auto Character = Cast<AUR_Character>(GamePC->GetCharacter()))
        {
            if (auto GameASC = Cast<UUR_AbilitySystemComponent>(Character->GetAbilitySystemComponent()))
            {
                return GameASC;
            }
        }
    }
    return nullptr;
}

void UUR_CheatManager::DamageSelfDestruct()
{
    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (const UUR_PawnExtensionComponent* PawnExtComp = UUR_PawnExtensionComponent::FindPawnExtensionComponent(GamePC->GetPawn()))
        {
            if (PawnExtComp->HasReachedInitState(URGameplayTags::InitState_GameplayReady))
            {
                if (UUR_HealthComponent* HealthComponent = UUR_HealthComponent::FindHealthComponent(GamePC->GetPawn()))
                {
                    HealthComponent->DamageSelfDestruct();
                }
            }
        }
    }
}

void UUR_CheatManager::UnlimitedHealth(int32 Enabled)
{
    if (UUR_AbilitySystemComponent* GameASC = GetPlayerAbilitySystemComponent())
    {
        const FGameplayTag Tag = URGameplayTags::Cheat_UnlimitedHealth;
        const bool bHasTag = GameASC->HasMatchingGameplayTag(Tag);

        if ((Enabled == -1) || ((Enabled > 0) && !bHasTag) || ((Enabled == 0) && bHasTag))
        {
            if (bHasTag)
            {
                GameASC->RemoveDynamicTagGameplayEffect(Tag);
            }
            else
            {
                GameASC->AddDynamicTagGameplayEffect(Tag);
            }
        }
    }
}
