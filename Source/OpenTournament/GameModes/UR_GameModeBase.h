// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ModularGameMode.h>

#include "UR_GameModeBase.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_ExperienceDefinition;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Base GameMode class for all OpenTournament GameModes
 */
UCLASS()
class OPENTOURNAMENT_API AUR_GameModeBase
    : public AModularGameMode
{
    GENERATED_BODY()

public:
    AUR_GameModeBase();

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Chat components centralizartion
    //TODO: might be better as a subsystem

    TArray<class UUR_ChatComponent*> ChatComponents;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    virtual void RegisterChatComponent(class UUR_ChatComponent* InComponent);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    virtual void UnregisterChatComponent(class UUR_ChatComponent* InComponent);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Experience stuff

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

    void HandleMatchAssignmentIfNotExpectingOne();

    bool TryDedicatedServerLogin();

    void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
    FName DefaultGameExperienceName;

    void OnExperienceLoaded(const UUR_ExperienceDefinition* CurrentExperience);

    bool IsExperienceLoaded() const;

    /////////////////////////////////////////////////////////////////////////////////////////////////
};
