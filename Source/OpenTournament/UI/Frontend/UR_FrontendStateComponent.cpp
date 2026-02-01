// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_FrontendStateComponent.h"

#include <CommonGameInstance.h>
#include <CommonSessionSubsystem.h>
#include <CommonUserSubsystem.h>
#include <ControlFlowManager.h>
#include <NativeGameplayTags.h>
#include <PrimaryGameLayout.h>
#include <Kismet/GameplayStatics.h>

#include "UR_LogChannels.h"
#include "GameModes/UR_ExperienceManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_FrontendStateComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace FrontendTags
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PLATFORM_TRAIT_SINGLEONLINEUSER, "Platform.Trait.SingleOnlineUser");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MENU, "UI.Layer.Menu");
}

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_FrontendStateComponent::UUR_FrontendStateComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

void UUR_FrontendStateComponent::BeginPlay()
{
    Super::BeginPlay();

    // Listen for the experience load to complete
    AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
    UUR_ExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UUR_ExperienceManagerComponent>();
    check(ExperienceComponent);

    // This delegate is on a component with the same lifetime as this one, so no need to unhook it in
    ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(FOnGameExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void UUR_FrontendStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

bool UUR_FrontendStateComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
    if (bShouldShowLoadingScreen)
    {
        OutReason = TEXT("Frontend Flow Pending...");

        if (FrontEndFlow.IsValid())
        {
            const TOptional<FString> StepDebugName = FrontEndFlow->GetCurrentStepDebugName();
            if (StepDebugName.IsSet())
            {
                OutReason = StepDebugName.GetValue();
            }
        }

        return true;
    }

    return false;
}

void UUR_FrontendStateComponent::OnExperienceLoaded(const UUR_ExperienceDefinition* Experience)
{
    FControlFlow& Flow = FControlFlowStatics::Create(this, TEXT("FrontendFlow"))
                         .QueueStep(TEXT("Wait For User Initialization"), this, &ThisClass::FlowStep_WaitForUserInitialization)
                         .QueueStep(TEXT("Try Show Press Start Screen"), this, &ThisClass::FlowStep_TryShowPressStartScreen)
                         .QueueStep(TEXT("Try Join Requested Session"), this, &ThisClass::FlowStep_TryJoinRequestedSession)
                         .QueueStep(TEXT("Try Show Main Screen"), this, &ThisClass::FlowStep_TryShowMainScreen);

    Flow.ExecuteFlow();

    FrontEndFlow = Flow.AsShared();
}

void UUR_FrontendStateComponent::FlowStep_WaitForUserInitialization(FControlFlowNodeRef SubFlow)
{
    // If this was a hard disconnect, explicitly destroy all user and session state
    // TODO: Refactor the engine disconnect flow so it is more explicit about why it happened
    bool bWasHardDisconnect = false;
    const AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>();
    const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);

    if (ensure(GameMode) && UGameplayStatics::HasOption(GameMode->OptionsString, TEXT("closed")))
    {
        bWasHardDisconnect = true;
    }

    // Only reset users on hard disconnect
    UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();
    if (ensure(UserSubsystem) && bWasHardDisconnect)
    {
        UserSubsystem->ResetUserState();
    }

    // Always reset sessions
    UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
    if (ensure(SessionSubsystem))
    {
        SessionSubsystem->CleanUpSessions();
    }

    SubFlow->ContinueFlow();
}

void UUR_FrontendStateComponent::FlowStep_TryShowPressStartScreen(FControlFlowNodeRef SubFlow)
{
    const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
    if (!IsValid(GameInstance))
    {
        return;
    }
    UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

    // Check to see if the first player is already logged in, if they are, we can skip the press start screen.
    if (const UCommonUserInfo* FirstUser = UserSubsystem->GetUserInfoForLocalPlayerIndex(0))
    {
        if (FirstUser->InitializationState == ECommonUserInitializationState::LoggedInLocalOnly ||
            FirstUser->InitializationState == ECommonUserInitializationState::LoggedInOnline)
        {
            SubFlow->ContinueFlow();
            return;
        }
    }

    // Check to see if the platform actually requires a 'Press Start' screen.  This is only
    // required on platforms where there can be multiple online users where depending on what player's
    // controller presses 'Start' establishes the player to actually login to the game with.
    if (!UserSubsystem->ShouldWaitForStartInput())
    {
        // Start the auto login process, this should finish quickly and will use the default input device id
        InProgressPressStartScreen = SubFlow;
        UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &UUR_FrontendStateComponent::OnUserInitialized);
        UserSubsystem->TryToInitializeForLocalPlay(0, FInputDeviceId(), false);

        return;
    }

    // Add the Press Start screen, move to the next flow when it deactivates.
    if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this))
    {
        constexpr bool bSuspendInputUntilComplete = true;
        RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(FrontendTags::TAG_UI_LAYER_MENU,
            bSuspendInputUntilComplete,
            PressStartScreenClass,
            [this, SubFlow](const EAsyncWidgetLayerState State, const UCommonActivatableWidget* Screen)
            {
                switch (State)
                {
                    case EAsyncWidgetLayerState::AfterPush:
                    {
                        bShouldShowLoadingScreen = false;
                        Screen->OnDeactivated().AddWeakLambda(this,
                            [this, SubFlow]()
                            {
                                SubFlow->ContinueFlow();
                            });
                        break;
                    }
                    case EAsyncWidgetLayerState::Canceled:
                    {
                        bShouldShowLoadingScreen = false;
                        SubFlow->ContinueFlow();
                        return;
                    }
                    default:
                    {
                        UE_LOG(LogGame, Warning, TEXT("Unhandled case in FlowStep_TryShowPressStartScreen"));
                        return;
                    }
                }
            });
    }
}

void UUR_FrontendStateComponent::OnUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
    FControlFlowNodePtr FlowToContinue = InProgressPressStartScreen;
    UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
    UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

    if (ensure(FlowToContinue.IsValid() && UserSubsystem))
    {
        UserSubsystem->OnUserInitializeComplete.RemoveDynamic(this, &UUR_FrontendStateComponent::OnUserInitialized);
        InProgressPressStartScreen.Reset();

        if (bSuccess)
        {
            // On success continue flow normally
            FlowToContinue->ContinueFlow();
        }
        else
        {
            // TODO: Just continue for now, could go to some sort of error screen
            FlowToContinue->ContinueFlow();
        }
    }
}

void UUR_FrontendStateComponent::FlowStep_TryJoinRequestedSession(FControlFlowNodeRef SubFlow)
{
    UCommonGameInstance* GameInstance = Cast<UCommonGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (GameInstance->GetRequestedSession() != nullptr && GameInstance->CanJoinRequestedSession())
    {
        UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
        if (ensure(SessionSubsystem))
        {
            // Bind to session join completion to continue or cancel the flow
            // TODO:  Need to ensure that after session join completes, the server travel completes.
            OnJoinSessionCompleteEventHandle = SessionSubsystem->OnJoinSessionCompleteEvent.AddWeakLambda(this,
                [this, SubFlow, SessionSubsystem](const FOnlineResultInformation& Result)
                {
                    // Unbind delegate. SessionSubsystem is the object triggering this event, so it must still be valid.
                    SessionSubsystem->OnJoinSessionCompleteEvent.Remove(OnJoinSessionCompleteEventHandle);
                    OnJoinSessionCompleteEventHandle.Reset();

                    if (Result.bWasSuccessful)
                    {
                        // No longer transitioning to the main menu
                        SubFlow->CancelFlow();
                    }
                    else
                    {
                        // Proceed to the main menu
                        SubFlow->ContinueFlow();
                        return;
                    }
                });
            GameInstance->JoinRequestedSession();
            return;
        }
    }
    // Skip this step if we didn't start requesting a session join
    SubFlow->ContinueFlow();
}

void UUR_FrontendStateComponent::FlowStep_TryShowMainScreen(FControlFlowNodeRef SubFlow)
{
    if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this))
    {
        constexpr bool bSuspendInputUntilComplete = true;
        RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(FrontendTags::TAG_UI_LAYER_MENU,
            bSuspendInputUntilComplete,
            MainScreenClass,
            [this, SubFlow](EAsyncWidgetLayerState State, UCommonActivatableWidget* Screen)
            {
                switch (State)
                {
                    case EAsyncWidgetLayerState::AfterPush:
                    {
                        bShouldShowLoadingScreen = false;
                        SubFlow->ContinueFlow();
                        return;
                    }
                    case EAsyncWidgetLayerState::Canceled:
                    {
                        bShouldShowLoadingScreen = false;
                        SubFlow->ContinueFlow();
                        return;
                    }
                    default:
                    {
                        UE_LOG(LogGame, Warning, TEXT("Unhandled case in FlowStep_TryShowMainScreen"));
                        return;
                    }
                }
            });
    }
}
