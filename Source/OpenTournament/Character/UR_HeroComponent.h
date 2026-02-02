// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Components/PawnComponent.h>
#include <Components/GameFrameworkInitStateInterface.h>

#include <GameplayAbilitySpecHandle.h>
#include <GameFeatures/GameFeatureAction_AddInputContextMapping.h>

#include "InputActionValue.h"
#include "UR_HeroComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace EEndPlayReason
{
    enum Type : int;
}

struct FLoadedMappableConfigPair;
struct FInputMappingContextAndPriority;

class UGameFrameworkComponentManager;
class UInputComponent;
class UUR_CameraMode;
class UUR_InputConfig;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;
struct FInputActionValue;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Component that sets up input and camera handling for player controlled pawns (or bots that simulate players).
 * This depends on a PawnExtensionComponent to coordinate initialization.
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UUR_HeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
    GENERATED_BODY()

public:
    UUR_HeroComponent(const FObjectInitializer& ObjectInitializer);

    /** Returns the hero component if one exists on the specified actor. */
    UFUNCTION(BlueprintPure, Category = "OT|Hero")
    static UUR_HeroComponent* FindHeroComponent(const AActor* Actor)
    {
        return (Actor ? Actor->FindComponentByClass<UUR_HeroComponent>() : nullptr);
    }

    /** Overrides the camera from an active gameplay ability */
    void SetAbilityCameraMode(const TSubclassOf<UUR_CameraMode>& CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

    /** Clears the camera override if it is set */
    void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

    /** Adds mode-specific input config */
    void AddAdditionalInputConfig(const UUR_InputConfig* InputConfig);

    /** Removes a mode-specific input config if it has been added */
    void RemoveAdditionalInputConfig(const UUR_InputConfig* InputConfig);

    /** True if this is controlled by a real player and has progressed far enough in initialization where additional input bindings can be added */
    bool IsReadyToBindInputs() const;

    /** The name of the extension event sent via UGameFrameworkComponentManager when ability inputs are ready to bind */
    static const FName NAME_BindInputsNow;

    /** The name of this component-implemented feature */
    static const FName NAME_ActorFeatureName;

    //~ Begin IGameFrameworkInitStateInterface interface
    virtual FName GetFeatureName() const override
    {
        return NAME_ActorFeatureName;
    }

    virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;

    virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;

    virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;

    virtual void CheckDefaultInitialization() override;

    //~ End IGameFrameworkInitStateInterface interface

//was protected:
public:
    virtual void OnRegister() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);

    void Input_AbilityInputTagPressed(const FGameplayTag InputTag);
    void Input_AbilityInputTagReleased(const FGameplayTag InputTag);

    void Input_Move(const FInputActionValue& InputActionValue);
    void Input_LookMouse(const FInputActionValue& InputActionValue);
    void Input_LookStick(const FInputActionValue& InputActionValue);
    void Input_Crouch(const FInputActionValue& InputActionValue);
    void Input_Dash(const FInputActionValue& InputActionValue);
    void Input_AutoRun(const FInputActionValue& InputActionValue);

    TSubclassOf<UUR_CameraMode> DetermineCameraMode() const;

protected:
    /**
     * Input Configs that should be added to this player when initializing the input. These configs
     * will NOT be registered with the settings because they are added at runtime. If you want the config
     * pair to be in the settings, then add it via the GameFeatureAction_AddInputConfig
     *
     * NOTE: You should only add to this if you do not have a game feature plugin accessible to you.
     * If you do, then use the GameFeatureAction_AddInputConfig instead.
     */
    UPROPERTY(EditAnywhere)
    TArray<FInputMappingContextAndPriority> DefaultInputMappings;

    /** Camera mode set by an ability. */
    UPROPERTY()
    TSubclassOf<UUR_CameraMode> AbilityCameraMode;

    /** Spec handle for the last ability to set a camera mode. */
    FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

    /** True when player input bindings have been applied, will never be true for non - players */
    bool bReadyToBindInputs;
};
