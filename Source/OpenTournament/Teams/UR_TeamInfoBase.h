// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <GameFramework/Info.h>

#include "GameplayTagStack.h"

#include "UR_TeamInfoBase.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace EEndPlayReason
{
    enum Type : int;
}

class UUR_TeamCreationComponent;
class UUR_TeamSubsystem;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class AUR_TeamInfoBase : public AInfo
{
    GENERATED_BODY()

public:
    AUR_TeamInfoBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    int32 GetTeamId() const
    {
        return TeamId;
    }

    //~AActor interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~End of AActor interface

protected:
    virtual void RegisterWithTeamSubsystem(UUR_TeamSubsystem* Subsystem);
    void TryRegisterWithTeamSubsystem();

private:
    void SetTeamId(int32 NewTeamId);

    UFUNCTION()
    void OnRep_TeamId();

public:
    friend UUR_TeamCreationComponent;

    UPROPERTY(Replicated)
    FGameplayTagStackContainer TeamTags;

private:
    UPROPERTY(ReplicatedUsing=OnRep_TeamId)
    int32 TeamId;
};
