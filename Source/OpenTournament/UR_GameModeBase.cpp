// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameModeBase.h"

#include <TimerManager.h>
#include <Kismet/GameplayStatics.h>
#include <Misc/CommandLine.h>

#include "UR_AssetManager.h"
#include "Development/UR_DeveloperSettings.h"
#include "UR_ExperienceDefinition.h"
#include "UR_ExperienceManagerComponent.h"
#include "UR_WorldSettings.h"
#include "UR_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameModeBase)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameModeBase::AUR_GameModeBase()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameModeBase::RegisterChatComponent(UUR_ChatComponent* InComponent)
{
    if (InComponent)
    {
        ChatComponents.AddUnique(InComponent);
    }
}

void AUR_GameModeBase::UnregisterChatComponent(UUR_ChatComponent* InComponent)
{
    if (ChatComponents.Num() > 0)
    {
        ChatComponents.Remove(InComponent);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // Wait for the next frame to give time to initialize startup settings
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

void AUR_GameModeBase::HandleMatchAssignmentIfNotExpectingOne()
{
    FPrimaryAssetId ExperienceId;
    FString ExperienceIdSource;

    // Precedence order (highest wins)
    //  - Matchmaking assignment (if present)
    //  - URL Options override
    //  - Developer Settings (PIE only)
    //  - Command Line override
    //  - World Settings
    //  - Dedicated server
    //  - Default experience

    UWorld* World = GetWorld();
    const FName ExperienceDefinitionClassName = UUR_ExperienceDefinition::StaticClass()->GetFName();

    if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
    {
        const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));

        FPrimaryAssetType PrimaryAsset = FPrimaryAssetType(ExperienceDefinitionClassName);
        ExperienceId = FPrimaryAssetId(PrimaryAsset, FName(*ExperienceFromOptions));
        ExperienceIdSource = TEXT("OptionsString");
    }

    if (!ExperienceId.IsValid() && World->IsPlayInEditor())
    {
        ExperienceId = GetDefault<UUR_DeveloperSettings>()->ExperienceOverride;
        ExperienceIdSource = TEXT("DeveloperSettings");
    }

    // see if the command line wants to set the experience
    if (!ExperienceId.IsValid())
    {
        FString ExperienceFromCommandLine;
        if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
        {
            ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
            if (!ExperienceId.PrimaryAssetType.IsValid())
            {
                FPrimaryAssetType PrimaryAsset = FPrimaryAssetType(ExperienceDefinitionClassName);
                ExperienceId = FPrimaryAssetId(PrimaryAsset, FName(*ExperienceFromCommandLine));
            }
            ExperienceIdSource = TEXT("CommandLine");
        }
    }

    // see if the world settings has a default experience
    if (!ExperienceId.IsValid())
    {
        if (AUR_WorldSettings* TypedWorldSettings = Cast<AUR_WorldSettings>(GetWorldSettings()))
        {
            ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
            ExperienceIdSource = TEXT("WorldSettings");
        }
    }

    UUR_AssetManager& AssetManager = UUR_AssetManager::Get();
    FAssetData Dummy;
    if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, /*out*/ Dummy))
    {
        UE_LOG(LogGameExperience, Error, TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)"), *ExperienceId.ToString());
        ExperienceId = FPrimaryAssetId();
    }

    // Final fallback to the default experience
    if (!ExperienceId.IsValid())
    {
        if (TryDedicatedServerLogin())
        {
            // This will start to host as a dedicated server
            return;
        }

        FPrimaryAssetType PrimaryAsset = FPrimaryAssetType(ExperienceDefinitionClassName);
        ExperienceId = FPrimaryAssetId(PrimaryAsset, DefaultGameExperienceName);
        ExperienceIdSource = TEXT("Default");
    }

    OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}

bool AUR_GameModeBase::TryDedicatedServerLogin()
{
    // // Some basic code to register as an active dedicated server, this would be heavily modified by the game
    // FString DefaultMap = UGameMapsSettings::GetGameDefaultMap();
    // UWorld* World = GetWorld();
    // UGameInstance* GameInstance = GetGameInstance();
    // if (GameInstance && World && World->GetNetMode() == NM_DedicatedServer && World->URL.Map == DefaultMap)
    // {
    //     // Only register if this is the default map on a dedicated server
    //     UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();
    //
    //     // Dedicated servers may need to do an online login
    //     UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &AUR_GameMode::OnUserInitializedForDedicatedServer);
    //
    //     // There are no local users on dedicated server, but index 0 means the default platform user which is handled by the online login code
    //     if (!UserSubsystem->TryToLoginForOnlinePlay(0))
    //     {
    //         OnUserInitializedForDedicatedServer(nullptr, false, FText(), ECommonUserPrivilege::CanPlayOnline, ECommonUserOnlineContext::Default);
    //     }
    //
    //     return true;
    // }

    return false;
}

void AUR_GameModeBase::OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)
{
    if (ExperienceId.IsValid())
    {
        UE_LOG(LogGameExperience, Log, TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(), *ExperienceIdSource);

        UUR_ExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UUR_ExperienceManagerComponent>();
        check(ExperienceComponent);
        ExperienceComponent->SetCurrentExperience(ExperienceId);
    }
    else
    {
        UE_LOG(LogGameExperience, Error, TEXT("Failed to identify experience, loading screen will stay up forever"));
    }
}
