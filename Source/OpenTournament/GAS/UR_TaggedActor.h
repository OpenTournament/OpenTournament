// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"

#include "UR_TaggedActor.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

// An actor that implements the gameplay tag asset interface
UCLASS()
class AUR_TaggedActor
    : public AActor
    , public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    AUR_TaggedActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~IGameplayTagAssetInterface
    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
    //~End of IGameplayTagAssetInterface

    //~UObject interface
#if WITH_EDITOR
    virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
    //~End of UObject interface

protected:
    // Gameplay-related tags associated with this actor
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Actor)
    FGameplayTagContainer StaticGameplayTags;
};
