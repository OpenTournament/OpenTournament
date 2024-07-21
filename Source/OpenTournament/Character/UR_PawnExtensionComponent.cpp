// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PawnExtensionComponent.h"

#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

#include "UR_AbilitySystemComponent.h"
#include "UR_GameplayTags.h"
#include "UR_LogChannels.h"
#include "UR_PawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PawnExtensionComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;
class UActorComponent;

const FName UUR_PawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_PawnExtensionComponent::UUR_PawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    PawnData = nullptr;
    AbilitySystemComponent = nullptr;
}

void UUR_PawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, PawnData);
}

void UUR_PawnExtensionComponent::OnRegister()
{
    Super::OnRegister();

    const APawn* Pawn = GetPawn<APawn>();
    ensureAlwaysMsgf((Pawn != nullptr), TEXT("UR_PawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

    TArray<UActorComponent*> PawnExtensionComponents;
    Pawn->GetComponents(ThisClass::StaticClass(), PawnExtensionComponents);
    ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1), TEXT("Only one UR_PawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

    // Register with the init state system early, this will only work if this is a game world
    RegisterInitStateFeature();
}

void UUR_PawnExtensionComponent::BeginPlay()
{
    Super::BeginPlay();

    // Listen for changes to all features
    BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

    // Notifies state manager that we have spawned, then try rest of default initialization
    ensure(TryToChangeInitState(URGameplayTags::InitState_Spawned));
    CheckDefaultInitialization();
}

void UUR_PawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UninitializeAbilitySystem();
    UnregisterInitStateFeature();

    Super::EndPlay(EndPlayReason);
}

void UUR_PawnExtensionComponent::SetPawnData(const UUR_PawnData* InPawnData)
{
    check(InPawnData);

    APawn* Pawn = GetPawnChecked<APawn>();

    if (Pawn->GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    if (PawnData)
    {
        UE_LOG(LogGame, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
        return;
    }

    PawnData = InPawnData;

    Pawn->ForceNetUpdate();

    CheckDefaultInitialization();
}

void UUR_PawnExtensionComponent::OnRep_PawnData()
{
    CheckDefaultInitialization();
}

void UUR_PawnExtensionComponent::InitializeAbilitySystem(UUR_AbilitySystemComponent* InASC, AActor* InOwnerActor)
{
    check(InASC);
    check(InOwnerActor);

    if (AbilitySystemComponent == InASC)
    {
        // The ability system component hasn't changed.
        return;
    }

    if (AbilitySystemComponent)
    {
        // Clean up the old ability system component.
        UninitializeAbilitySystem();
    }

    APawn* Pawn = GetPawnChecked<APawn>();
    AActor* ExistingAvatar = InASC->GetAvatarActor();

    UE_LOG(LogGame, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

    if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
    {
        UE_LOG(LogGame, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

        // There is already a pawn acting as the ASC's avatar, so we need to kick it out
        // This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
        ensure(!ExistingAvatar->HasAuthority());

        if (UUR_PawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
        {
            OtherExtensionComponent->UninitializeAbilitySystem();
        }
    }

    AbilitySystemComponent = InASC;
    AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

    if (ensure(PawnData))
    {
        InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
    }

    OnAbilitySystemInitialized.Broadcast();
}

void UUR_PawnExtensionComponent::UninitializeAbilitySystem()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    // Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
    if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
    {
        FGameplayTagContainer AbilityTypesToIgnore;
        AbilityTypesToIgnore.AddTag(URGameplayTags::Ability_Behavior_SurvivesDeath);

        AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
        AbilitySystemComponent->ClearAbilityInput();
        AbilitySystemComponent->RemoveAllGameplayCues();

        if (AbilitySystemComponent->GetOwnerActor() != nullptr)
        {
            AbilitySystemComponent->SetAvatarActor(nullptr);
        }
        else
        {
            // If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
            AbilitySystemComponent->ClearActorInfo();
        }

        OnAbilitySystemUninitialized.Broadcast();
    }

    AbilitySystemComponent = nullptr;
}

void UUR_PawnExtensionComponent::HandleControllerChanged()
{
    if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
    {
        ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());
        if (AbilitySystemComponent->GetOwnerActor() == nullptr)
        {
            UninitializeAbilitySystem();
        }
        else
        {
            AbilitySystemComponent->RefreshAbilityActorInfo();
        }
    }

    CheckDefaultInitialization();
}

void UUR_PawnExtensionComponent::HandlePlayerStateReplicated()
{
    CheckDefaultInitialization();
}

void UUR_PawnExtensionComponent::SetupPlayerInputComponent()
{
    CheckDefaultInitialization();
}

void UUR_PawnExtensionComponent::CheckDefaultInitialization()
{
    // Before checking our progress, try progressing any other features we might depend on
    CheckDefaultInitializationForImplementers();

    static const TArray<FGameplayTag> StateChain = { URGameplayTags::InitState_Spawned, URGameplayTags::InitState_DataAvailable, URGameplayTags::InitState_DataInitialized, URGameplayTags::InitState_GameplayReady };

    // This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
    ContinueInitStateChain(StateChain);
}

bool UUR_PawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
    check(Manager);

    APawn* Pawn = GetPawn<APawn>();
    if (!CurrentState.IsValid() && DesiredState == URGameplayTags::InitState_Spawned)
    {
        // As long as we are on a valid pawn, we count as spawned
        if (Pawn)
        {
            return true;
        }
    }
    if (CurrentState == URGameplayTags::InitState_Spawned && DesiredState == URGameplayTags::InitState_DataAvailable)
    {
        // Pawn data is required.
        if (!PawnData)
        {
            return false;
        }

        const bool bHasAuthority = Pawn->HasAuthority();
        const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

        if (bHasAuthority || bIsLocallyControlled)
        {
            // Check for being possessed by a controller.
            if (!GetController<AController>())
            {
                return false;
            }
        }

        return true;
    }
    else if (CurrentState == URGameplayTags::InitState_DataAvailable && DesiredState == URGameplayTags::InitState_DataInitialized)
    {
        // Transition to initialize if all features have their data available
        return Manager->HaveAllFeaturesReachedInitState(Pawn, URGameplayTags::InitState_DataAvailable);
    }
    else if (CurrentState == URGameplayTags::InitState_DataInitialized && DesiredState == URGameplayTags::InitState_GameplayReady)
    {
        return true;
    }

    return false;
}

void UUR_PawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
    if (DesiredState == URGameplayTags::InitState_DataInitialized)
    {
        // This is currently all handled by other components listening to this state change
    }
}

void UUR_PawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
    // If another feature is now in DataAvailable, see if we should transition to DataInitialized
    if (Params.FeatureName != NAME_ActorFeatureName)
    {
        if (Params.FeatureState == URGameplayTags::InitState_DataAvailable)
        {
            CheckDefaultInitialization();
        }
    }
}

void UUR_PawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
    if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
    {
        OnAbilitySystemInitialized.Add(Delegate);
    }

    if (AbilitySystemComponent)
    {
        Delegate.Execute();
    }
}

void UUR_PawnExtensionComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
    if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
    {
        OnAbilitySystemUninitialized.Add(Delegate);
    }
}
