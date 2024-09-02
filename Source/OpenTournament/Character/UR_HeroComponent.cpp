// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HeroComponent.h"

#include "Components/GameFrameworkComponentDelegates.h"
#include "Logging/MessageLog.h"
#include "EnhancedInputSubsystems.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PlayerMappableInputConfig.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputMappingContext.h"

#include "UR_LogChannels.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"
#include "UR_LocalPlayer.h"
#include "Character/UR_PawnExtensionComponent.h"
#include "Character/UR_PawnData.h"
#include "UR_Character.h"
#include "GAS/UR_AbilitySystemComponent.h"
#include "Input/UR_InputConfig.h"
#include "UR_InputComponent.h"
#include "Camera/UR_CameraComponent.h"
#include "UR_GameplayTags.h"
#include "Camera/UR_CameraMode.h"


#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_HeroComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////


namespace GameHero
{
    static const float LookYawRate = 300.0f;
    static const float LookPitchRate = 165.0f;
};

const FName UUR_HeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName UUR_HeroComponent::NAME_ActorFeatureName("Hero");

UUR_HeroComponent::UUR_HeroComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AbilityCameraMode = nullptr;
    bReadyToBindInputs = false;
}

void UUR_HeroComponent::OnRegister()
{
    Super::OnRegister();

    if (!GetPawn<APawn>())
    {
        UE_LOG(LogGame, Error, TEXT("[UUR_HeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
        if (GIsEditor)
        {
            static const FText Message = NSLOCTEXT
                ("UR_HeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
            static const FName HeroMessageLogName = TEXT("UR_HeroComponent");

            FMessageLog(HeroMessageLogName).Error()
                                           ->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
                                           ->AddToken(FTextToken::Create(Message));

            FMessageLog(HeroMessageLogName).Open();
        }
#endif
    }
    else
    {
        // Register with the init state system early, this will only work if this is a game world
        RegisterInitStateFeature();
    }
}

bool UUR_HeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
    check(Manager);

    APawn* Pawn = GetPawn<APawn>();

    if (!CurrentState.IsValid() && DesiredState == URGameplayTags::InitState_Spawned)
    {
        // As long as we have a real pawn, let us transition
        if (Pawn)
        {
            return true;
        }
    }
    else if (CurrentState == URGameplayTags::InitState_Spawned && DesiredState == URGameplayTags::InitState_DataAvailable)
    {
        // The player state is required.
        if (!GetPlayerState<AUR_PlayerState>())
        {
            return false;
        }

        // If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
        if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
        {
            AController* Controller = GetController<AController>();

            const bool bHasControllerPairedWithPS = (Controller != nullptr) &&
                (Controller->PlayerState != nullptr) &&
                (Controller->PlayerState->GetOwner() == Controller);

            if (!bHasControllerPairedWithPS)
            {
                return false;
            }
        }

        const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
        const bool bIsBot = Pawn->IsBotControlled();

        if (bIsLocallyControlled && !bIsBot)
        {
            AUR_PlayerController* PC = GetController<AUR_PlayerController>();

            // The input component and local player is required when locally controlled.
            if (!Pawn->InputComponent || !PC || !PC->GetLocalPlayer())
            {
                return false;
            }
        }

        return true;
    }
    else if (CurrentState == URGameplayTags::InitState_DataAvailable && DesiredState == URGameplayTags::InitState_DataInitialized)
    {
        // Wait for player state and extension component
        AUR_PlayerState* PS = GetPlayerState<AUR_PlayerState>();

        return PS && Manager->HasFeatureReachedInitState(Pawn, UUR_PawnExtensionComponent::NAME_ActorFeatureName, URGameplayTags::InitState_DataInitialized);
    }
    else if (CurrentState == URGameplayTags::InitState_DataInitialized && DesiredState == URGameplayTags::InitState_GameplayReady)
    {
        // TODO add ability initialization checks?
        return true;
    }

    return false;
}

void UUR_HeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
    if (CurrentState == URGameplayTags::InitState_DataAvailable && DesiredState == URGameplayTags::InitState_DataInitialized)
    {
        APawn* Pawn = GetPawn<APawn>();
        AUR_PlayerState* PS = GetPlayerState<AUR_PlayerState>();
        if (!ensure(Pawn && PS))
        {
            return;
        }

        const UUR_PawnData* PawnData = nullptr;

        if (UUR_PawnExtensionComponent* PawnExtComp = UUR_PawnExtensionComponent::FindPawnExtensionComponent(Pawn))
        {
            PawnData = PawnExtComp->GetPawnData<UUR_PawnData>();

            // The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
            // The ability system component and attribute sets live on the player state.
            PawnExtComp->InitializeAbilitySystem(PS->GetGameAbilitySystemComponent(), PS);
        }

        if (AUR_PlayerController* PC = GetController<AUR_PlayerController>())
        {
            if (Pawn->InputComponent != nullptr)
            {
                InitializePlayerInput(Pawn->InputComponent);
            }
        }

        // Hook up the delegate for all pawns, in case we spectate later
        if (PawnData)
        {
            if (UUR_CameraComponent* CameraComponent = UUR_CameraComponent::FindCameraComponent(Pawn))
            {
                CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
            }
        }
    }
}

void UUR_HeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
    if (Params.FeatureName == UUR_PawnExtensionComponent::NAME_ActorFeatureName)
    {
        if (Params.FeatureState == URGameplayTags::InitState_DataInitialized)
        {
            // If the extension component says all all other components are initialized, try to progress to next state
            CheckDefaultInitialization();
        }
    }
}

void UUR_HeroComponent::CheckDefaultInitialization()
{
    static const TArray<FGameplayTag> StateChain = { URGameplayTags::InitState_Spawned, URGameplayTags::InitState_DataAvailable, URGameplayTags::InitState_DataInitialized, URGameplayTags::InitState_GameplayReady };

    // This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
    ContinueInitStateChain(StateChain);
}

void UUR_HeroComponent::BeginPlay()
{
    Super::BeginPlay();

    // Listen for when the pawn extension component changes init state
    BindOnActorInitStateChanged(UUR_PawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

    // Notifies that we are done spawning, then try the rest of initialization
    ensure(TryToChangeInitState(URGameplayTags::InitState_Spawned));
    CheckDefaultInitialization();
}

void UUR_HeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterInitStateFeature();

    Super::EndPlay(EndPlayReason);
}

void UUR_HeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);

    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return;
    }

    const APlayerController* PC = GetController<APlayerController>();
    check(PC);

    const UUR_LocalPlayer* LP = Cast<UUR_LocalPlayer>(PC->GetLocalPlayer());
    check(LP);

    UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    check(Subsystem);

    Subsystem->ClearAllMappings();

    if (const UUR_PawnExtensionComponent* PawnExtComp = UUR_PawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        if (const UUR_PawnData* PawnData = PawnExtComp->GetPawnData<UUR_PawnData>())
        {
            if (const UUR_InputConfig* InputConfig = PawnData->InputConfig)
            {
                for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
                {
                    if (UInputMappingContext* IMC = Mapping.InputMapping.Get())
                    {
                        if (Mapping.bRegisterWithSettings)
                        {
                            if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
                            {
                                Settings->RegisterInputMappingContext(IMC);
                            }

                            FModifyContextOptions Options = { };
                            Options.bIgnoreAllPressedKeysUntilRelease = false;
                            // Actually add the config to the local player
                            Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
                        }
                    }
                }

                // The Game Input Component has some additional functions to map Gameplay Tags to an Input Action.
                // If you want this functionality but still want to change your input component class, make it a subclass
                // of the UUR_InputComponent or modify this component accordingly.
                UUR_InputComponent* InputComponent = Cast<UUR_InputComponent>(PlayerInputComponent);
                if (ensureMsgf(InputComponent, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UUR_InputComponent or a subclass of it.")))
                {
                    // Add the key mappings that may have been set by the player
                    InputComponent->AddInputMappings(InputConfig, Subsystem);

                    // This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
                    // be triggered directly by these input actions Triggered events.
                    TArray<uint32> BindHandles;
                    InputComponent->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

                    InputComponent->BindNativeAction(InputConfig, URGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
                    InputComponent->BindNativeAction(InputConfig, URGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
                    InputComponent->BindNativeAction(InputConfig, URGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
                    InputComponent->BindNativeAction(InputConfig, URGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
                    InputComponent->BindNativeAction(InputConfig, URGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
                }
            }
        }
    }

    if (ensure(!bReadyToBindInputs))
    {
        bReadyToBindInputs = true;
    }

    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

void UUR_HeroComponent::AddAdditionalInputConfig(const UUR_InputConfig* InputConfig)
{
    TArray<uint32> BindHandles;

    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return;
    }

    const APlayerController* PC = GetController<APlayerController>();
    check(PC);

    const ULocalPlayer* LP = PC->GetLocalPlayer();
    check(LP);

    UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    check(Subsystem);

    if (const UUR_PawnExtensionComponent* PawnExtComp = UUR_PawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        UUR_InputComponent* InputComponent = Pawn->FindComponentByClass<UUR_InputComponent>();
        if (ensureMsgf(InputComponent, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UUR_InputComponent or a subclass of it.")))
        {
            InputComponent->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
        }
    }
}

void UUR_HeroComponent::RemoveAdditionalInputConfig(const UUR_InputConfig* InputConfig)
{
    //@TODO: Implement me!
}

bool UUR_HeroComponent::IsReadyToBindInputs() const
{
    return bReadyToBindInputs;
}

void UUR_HeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (const APawn* Pawn = GetPawn<APawn>())
    {
        if (const UUR_PawnExtensionComponent* PawnExtComp = UUR_PawnExtensionComponent::FindPawnExtensionComponent(Pawn))
        {
            if (UUR_AbilitySystemComponent* ASC = PawnExtComp->GetGameAbilitySystemComponent())
            {
                ASC->AbilityInputTagPressed(InputTag);
            }
        }
    }
}

void UUR_HeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return;
    }

    if (const UUR_PawnExtensionComponent* PawnExtComp = UUR_PawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        if (UUR_AbilitySystemComponent* ASC = PawnExtComp->GetGameAbilitySystemComponent())
        {
            ASC->AbilityInputTagReleased(InputTag);
        }
    }
}

void UUR_HeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
    APawn* Pawn = GetPawn<APawn>();
    AController* Controller = Pawn ? Pawn->GetController() : nullptr;

    // If the player has attempted to move again then cancel auto running
    if (AUR_PlayerController* GameController = Cast<AUR_PlayerController>(Controller))
    {
        //GameController->SetIsAutoRunning(false);
    }

    if (Controller)
    {
        const FVector2D Value = InputActionValue.Get<FVector2D>();
        const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

        if (Value.X != 0.0f)
        {
            const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
            Pawn->AddMovementInput(MovementDirection, Value.X);
        }

        if (Value.Y != 0.0f)
        {
            const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
            Pawn->AddMovementInput(MovementDirection, Value.Y);
        }
    }
}

void UUR_HeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
    APawn* Pawn = GetPawn<APawn>();

    if (!Pawn)
    {
        return;
    }

    const FVector2D Value = InputActionValue.Get<FVector2D>();

    if (Value.X != 0.0f)
    {
        Pawn->AddControllerYawInput(Value.X);
    }

    if (Value.Y != 0.0f)
    {
        Pawn->AddControllerPitchInput(Value.Y);
    }
}

void UUR_HeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
    APawn* Pawn = GetPawn<APawn>();

    if (!Pawn)
    {
        return;
    }

    const FVector2D Value = InputActionValue.Get<FVector2D>();

    const UWorld* World = GetWorld();
    check(World);

    if (Value.X != 0.0f)
    {
        Pawn->AddControllerYawInput(Value.X * GameHero::LookYawRate * World->GetDeltaSeconds());
    }

    if (Value.Y != 0.0f)
    {
        Pawn->AddControllerPitchInput(Value.Y * GameHero::LookPitchRate * World->GetDeltaSeconds());
    }
}

void UUR_HeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
    if (AUR_Character* Character = GetPawn<AUR_Character>())
    {
        Character->ToggleCrouch();
    }
}

void UUR_HeroComponent::Input_AutoRun(const FInputActionValue& InputActionValue)
{
    if (APawn* Pawn = GetPawn<APawn>())
    {
        if (AUR_PlayerController* Controller = Cast<AUR_PlayerController>(Pawn->GetController()))
        {
            // Toggle auto running
            //Controller->SetIsAutoRunning(!Controller->GetIsAutoRunning());
        }
    }
}

TSubclassOf<UUR_CameraMode> UUR_HeroComponent::DetermineCameraMode() const
{
    if (AbilityCameraMode)
    {
        return AbilityCameraMode;
    }

    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return nullptr;
    }

    if (UUR_PawnExtensionComponent* PawnExtComp = UUR_PawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        if (const UUR_PawnData* PawnData = PawnExtComp->GetPawnData<UUR_PawnData>())
        {
            return PawnData->DefaultCameraMode;
        }
    }

    return nullptr;
}

void UUR_HeroComponent::SetAbilityCameraMode(TSubclassOf<UUR_CameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
    if (CameraMode)
    {
        AbilityCameraMode = CameraMode;
        AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
    }
}

void UUR_HeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
    if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
    {
        AbilityCameraMode = nullptr;
        AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
    }
}
