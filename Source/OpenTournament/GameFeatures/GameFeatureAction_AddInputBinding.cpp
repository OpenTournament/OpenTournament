// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameFeatureAction_AddInputBinding.h"

#include "EnhancedInputSubsystems.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "Character/UR_HeroComponent.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "Input/UR_InputConfig.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddInputBinding)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "GameFeatures"

/////////////////////////////////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddInputBinding

void UGameFeatureAction_AddInputBinding::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
    FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
    if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
        !ensure(ActiveData.PawnsAddedTo.IsEmpty()))
    {
        Reset(ActiveData);
    }
    Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputBinding::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
    Super::OnGameFeatureDeactivating(Context);
    FPerContextData* ActiveData = ContextData.Find(Context);

    if (ensure(ActiveData))
    {
        Reset(*ActiveData);
    }
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputBinding::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

    int32 Index = 0;

    for (const TSoftObjectPtr<const UUR_InputConfig>& Entry : InputConfigs)
    {
        if (Entry.IsNull())
        {
            Result = EDataValidationResult::Invalid;
            Context.AddError(FText::Format(LOCTEXT("NullInputConfig", "Null InputConfig at index {0}."), Index));
        }
        ++Index;
    }

    return Result;
}
#endif

void UGameFeatureAction_AddInputBinding::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
    UWorld* World = WorldContext.World();
    UGameInstance* GameInstance = WorldContext.OwningGameInstance;
    FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

    if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
    {
        if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
        {
            UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate =
                UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandlePawnExtension, ChangeContext);
            TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle =
                ComponentManager->AddExtensionHandler(APawn::StaticClass(), AddAbilitiesDelegate);

            ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
        }
    }
}

void UGameFeatureAction_AddInputBinding::Reset(FPerContextData& ActiveData)
{
    ActiveData.ExtensionRequestHandles.Empty();

    while (!ActiveData.PawnsAddedTo.IsEmpty())
    {
        TWeakObjectPtr<APawn> PawnPtr = ActiveData.PawnsAddedTo.Top();
        if (PawnPtr.IsValid())
        {
            RemoveInputMapping(PawnPtr.Get(), ActiveData);
        }
        else
        {
            ActiveData.PawnsAddedTo.Pop();
        }
    }
}

void UGameFeatureAction_AddInputBinding::HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
    APawn* AsPawn = CastChecked<APawn>(Actor);
    FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

    if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
    {
        RemoveInputMapping(AsPawn, ActiveData);
    }
    else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == UUR_HeroComponent::NAME_BindInputsNow))
    {
        AddInputMappingForPlayer(AsPawn, ActiveData);
    }
}

void UGameFeatureAction_AddInputBinding::AddInputMappingForPlayer(APawn* Pawn, FPerContextData& ActiveData)
{
    APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());

    if (ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr)
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            UUR_HeroComponent* HeroComponent = Pawn->FindComponentByClass<UUR_HeroComponent>();
            if (HeroComponent && HeroComponent->IsReadyToBindInputs())
            {
                for (const TSoftObjectPtr<const UUR_InputConfig>& Entry : InputConfigs)
                {
                    if (const UUR_InputConfig* BindSet = Entry.Get())
                    {
                        HeroComponent->AddAdditionalInputConfig(BindSet);
                    }
                }
            }
            ActiveData.PawnsAddedTo.AddUnique(Pawn);
        }
        else
        {
            UE_LOG(LogGameFeatures, Error, TEXT("Failed to find `UEnhancedInputLocalPlayerSubsystem` for local player. Input mappings will not be added. Make sure you're set to use the EnhancedInput system via config file."));
        }
    }
}

void UGameFeatureAction_AddInputBinding::RemoveInputMapping(APawn* Pawn, FPerContextData& ActiveData)
{
    APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());

    if (ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr)
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (UUR_HeroComponent* HeroComponent = Pawn->FindComponentByClass<UUR_HeroComponent>())
            {
                for (const TSoftObjectPtr<const UUR_InputConfig>& Entry : InputConfigs)
                {
                    if (const UUR_InputConfig* InputConfig = Entry.Get())
                    {
                        HeroComponent->RemoveAdditionalInputConfig(InputConfig);
                    }
                }
            }
        }
    }

    ActiveData.PawnsAddedTo.Remove(Pawn);
}

#undef LOCTEXT_NAMESPACE
