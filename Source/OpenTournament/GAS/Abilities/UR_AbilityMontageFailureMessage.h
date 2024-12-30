// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
//#include "NativeGameplayTags.h"

#include "UR_AbilityMontageFailureMessage.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class APlayerController;
class UAnimMontage;

/////////////////////////////////////////////////////////////////////////////////////////////////

//UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, "Ability.PlayMontageOnActivateFail.Message");

/////////////////////////////////////////////////////////////////////////////////////////////////

/** Failure reason that can be used to play an animation montage when a failure occurs */
USTRUCT(BlueprintType)
struct FGameAbilityMontageFailureMessage
{
    GENERATED_BODY()

public:
    // Player controller that failed to activate the ability, if the AbilitySystemComponent was player owned
    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<APlayerController> PlayerController = nullptr;

    // Avatar actor that failed to activate the ability
    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<AActor> AvatarActor = nullptr;

    // All the reasons why this ability has failed
    UPROPERTY(BlueprintReadWrite)
    FGameplayTagContainer FailureTags;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};
