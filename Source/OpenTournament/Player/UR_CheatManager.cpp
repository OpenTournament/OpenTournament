// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CheatManager.h"

#include <AbilitySystemGlobals.h>
#include <GameFramework/GameplayMessageSubsystem.h>

#include "GameVerbMessage.h"
#include "UR_AbilitySystemComponent.h"
#include "UR_AssetManager.h"
#include "UR_Character.h"
#include "UR_ExperienceManagerComponent.h"
#include "UR_GameData.h"
#include "UR_GameMode.h"
#include "UR_GameplayTags.h"
#include "UR_HealthComponent.h"
#include "UR_HeroComponent.h"
#include "UR_InventoryComponent.h"
#include "UR_LogChannels.h"
#include "UR_PawnData.h"
#include "UR_PawnExtensionComponent.h"
#include "UR_PlayerController.h"
#include "UR_PlayerInput.h"
#include "UR_PlayerState.h"
#include "UR_Weapon.h"
#include "Development/UR_DeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_CheatManager)

/////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(LogGameCheat);

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace OpenTournamentCheat
{
    static bool bStartInGodMode = false;
    static FAutoConsoleVariableRef CVarStartInGodMode
    (
        TEXT("OTCheat.StartInGodMode"),
        bStartInGodMode,
        TEXT("If true then the God cheat will be applied on begin play"),
        ECVF_Cheat
    );
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
    const FString CheatString = FString::Printf(TEXT("God"));

    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (GamePC->GetNetMode() == NM_Client)
        {
            // Automatically send cheat to server for convenience.
            GamePC->ServerCheat(CheatString);
            return;
        }

        if (UUR_AbilitySystemComponent* GameASC = GamePC->GetGameAbilitySystemComponent())
        {
            const FGameplayTag Tag = URGameplayTags::Cheat_GodMode;
            const bool bHasTag = GameASC->HasMatchingGameplayTag(Tag);

            if (bHasTag)
            {
                GameASC->RemoveDynamicTagGameplayEffect(Tag);
                GetOuterAPlayerController()->ClientMessage(TEXT("God Mode off"));
            }
            else
            {
                GameASC->AddDynamicTagGameplayEffect(Tag);
                GetOuterAPlayerController()->ClientMessage(TEXT("God mode on"));
            }
        }
    }
}

void UUR_CheatManager::GetGameExperience()
{
    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (auto World = GamePC->GetWorld())
        {
            if (auto GameState = World->GetGameState())
            {
                if (auto ExperienceComponent = Cast<UUR_ExperienceManagerComponent>(GameState->GetComponentByClass(UUR_ExperienceManagerComponent::StaticClass())))
                {
                    auto Experience = ExperienceComponent->GetCurrentExperience();
                    UE_LOG(LogGameCheat, Log, TEXT("CurrentExperience: %s"), *Experience.GetName());
                }
            }
        }
    }
}

void UUR_CheatManager::GetPawnData()
{
    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (auto PS = GamePC->GetGamePlayerState())
        {
            if (const UUR_PawnData* PawnData = PS->GetPawnData<UUR_PawnData>())
            {
                UE_LOG(LogGameCheat, Log, TEXT("Retrieved PawnData for Current Pawn: %s"), *PawnData->GetName());
            }
            else
            {
                UE_LOG(LogGameCheat, Warning, TEXT("Unable to retrieve PawnData for Current Pawn!"));
            }
        }
    }
}

void UUR_CheatManager::UnlimitedHealth(int32 Enabled)
{
    const FString CheatString = FString::Printf(TEXT("UnlimitedHealth -1"));

    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (GamePC->GetNetMode() == NM_Client)
        {
            // Automatically send cheat to server for convenience.
            GamePC->ServerCheat(CheatString);
            return;
        }

        if (UUR_AbilitySystemComponent* GameASC = GetPlayerAbilitySystemComponent())
        {
            const FGameplayTag Tag = URGameplayTags::Cheat_UnlimitedHealth;
            const bool bHasTag = GameASC->HasMatchingGameplayTag(Tag);

            if ((Enabled == -1) || ((Enabled > 0) && !bHasTag) || ((Enabled == 0) && bHasTag))
            {
                if (bHasTag)
                {
                    GameASC->RemoveDynamicTagGameplayEffect(Tag);
                    GamePC->ClientMessage(TEXT("UnlimitedHealth Mode off"));
                }
                else
                {
                    GameASC->AddDynamicTagGameplayEffect(Tag);
                    GamePC->ClientMessage(TEXT("UnlimitedHealth Mode on"));
                }
            }
        }
    }
}

void UUR_CheatManager::Cheat_GameReload()
{
    const FString CheatString = FString::Printf(TEXT("Cheat_GameReload"));

    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (GamePC->GetNetMode() == NM_Client)
        {
            // Automatically send cheat to server for convenience.
            GamePC->ServerCheat(CheatString);
            return;
        }

        if (auto World = GamePC->GetWorld())
        {
            if (auto GameMode = Cast<AUR_GameMode>(World->GetAuthGameMode()))
            {
                if (auto GameState = World->GetGameState())
                {
                    if (const UUR_ExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UUR_ExperienceManagerComponent>())
                    {
                        GameMode->OnExperienceLoaded(ExperienceComponent->GetCurrentExperienceChecked());
                    }
                }
            }
        }
    }


}

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

void UUR_CheatManager::Cheat_GetInitState()
{
    const FString CheatString = FString::Printf(TEXT("GetInitState"));

    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (GamePC->GetNetMode() == NM_Client)
        {
            // Automatically send cheat to server for convenience.
            GamePC->ServerCheat(CheatString);
            //return;
        }

        if (auto Character = GamePC->GetPawn())
        {
            if (auto HeroComponent = Character->FindComponentByClass<UUR_HeroComponent>())
            {
                auto InitState = HeroComponent->GetInitState();
                UE_LOG(LogGame, Warning, TEXT("HeroComponent InitState: %s"), *InitState.ToString());
            }
            if (auto PEC = Character->FindComponentByClass<UUR_PawnExtensionComponent>())
            {
                auto InitState = PEC->GetInitState();
                UE_LOG(LogGame, Warning, TEXT("PawnExtensionComponent InitState: %s"), *InitState.ToString());
            }
            if (auto InputComponent = GamePC->GetPlayerInput())
            {
                auto AxisMappings = InputComponent->AxisMappings;
                UE_LOG(LogGame, Warning, TEXT("PlayerInputComponent InitState: %d"), AxisMappings.Num());
            }
        }

    }
}

void UUR_CheatManager::Cheat_Message(FString MessageTag)
{
    const FString CheatString = FString::Printf(TEXT("Cheat_Message %s"), *MessageTag);

    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (GamePC->GetNetMode() == NM_Client)
        {
            // Automatically send cheat to server for convenience.
            GamePC->ServerCheat(CheatString);
            return;
        }

        const FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName(MessageTag), false);
        if (!Tag.IsValid())
        {
            GAME_LOG(LogGameCheat, Error, "Invalid Tag specified: %s", *MessageTag);
            return;
        }

        // @! TODO: Probably need a better way to specify the Message and Message data
        FGameVerbMessage Message;
        Message.Verb = Tag;

        UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
        MessageSystem.BroadcastMessage(Message.Verb, Message);
    }
}

void UUR_CheatManager::DamageSelf(float DamageAmount)
{
    const FString CheatString = FString::Printf(TEXT("DamageSelf %.2f"), DamageAmount);

    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        // Note: Not Server-instigated
        if (UUR_AbilitySystemComponent* GameASC = GetPlayerAbilitySystemComponent())
        {
            ApplySetByCallerDamage(GameASC, DamageAmount);
            GamePC->ClientMessage(CheatString);
        }
    }
}

void UUR_CheatManager::DamageTarget(float DamageAmount)
{
    const FString CheatString = FString::Printf(TEXT("DamageTarget %.2f"), DamageAmount);

    if (AUR_PlayerController* GamePC = Cast<AUR_PlayerController>(GetOuterAPlayerController()))
    {
        if (GamePC->GetNetMode() == NM_Client)
        {
            // Automatically send cheat to server for convenience.
            GamePC->ServerCheat(CheatString);
            return;
        }

        FHitResult TargetHitResult;
        AActor* TargetActor = GetTarget(GamePC, TargetHitResult);

        if (UUR_AbilitySystemComponent* TargetASC = Cast<UUR_AbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor)))
        {
            ApplySetByCallerDamage(TargetASC, DamageAmount);
            GamePC->ClientMessage(CheatString);
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
        //
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
        return GamePC->GetGameAbilitySystemComponent();
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

