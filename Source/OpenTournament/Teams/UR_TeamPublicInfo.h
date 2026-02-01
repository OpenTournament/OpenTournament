// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_TeamInfoBase.h"

#include "UR_TeamPublicInfo.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_TeamCreationComponent;
class UUR_TeamDisplayAsset;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class AUR_TeamPublicInfo : public AUR_TeamInfoBase
{
    GENERATED_BODY()

    friend UUR_TeamCreationComponent;

public:
    AUR_TeamPublicInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UUR_TeamDisplayAsset* GetTeamDisplayAsset() const;

private:
    UFUNCTION()
    void OnRep_TeamDisplayAsset();

    void SetTeamDisplayAsset(TObjectPtr<UUR_TeamDisplayAsset> NewDisplayAsset);

private:
    UPROPERTY(ReplicatedUsing=OnRep_TeamDisplayAsset)
    TObjectPtr<UUR_TeamDisplayAsset> TeamDisplayAsset;
};
