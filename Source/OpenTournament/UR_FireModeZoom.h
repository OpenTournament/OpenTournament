// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UR_FireModeBase.h"
#include "Interfaces/UR_ActivatableInterface.h"
#include "UR_FireModeZoom.generated.h"

class UUserWidget;

/**
 * 
 */
UCLASS(ClassGroup = (FireMode), Meta = (BlueprintSpawnableComponent), HideCategories = ("FireMode|SpinUp"))
class OPENTOURNAMENT_API UUR_FireModeZoom : public UUR_FireModeBase, public IUR_ActivatableInterface
{
	GENERATED_BODY()

public:
    UUR_FireModeZoom()
    {
        PrimaryComponentTick.bCanEverTick = true;
        PrimaryComponentTick.bStartWithTickEnabled = false;

        Index = 1;
        InitialAmmoCost = 0;

        ZoomFOV = 50;
        ZoomInTime = 0.15f;
        ZoomOutTime = 0.15f;
        bHide1PMeshes = true;

        AnimAlpha = 0.f;
    }

    UPROPERTY(EditAnywhere, Category = "FireMode")
    TSubclassOf<UUserWidget> ZoomWidgetClass;

    //TODO: turn this into a client setting
    UPROPERTY(EditAnywhere, Category = "FireMode")
    float ZoomFOV;

    //TODO: turn this into a client setting
    UPROPERTY(EditAnywhere, Category = "FireMode")
    float ZoomInTime;

    //TODO: turn this into a client setting
    UPROPERTY(EditAnywhere, Category = "FireMode")
    float ZoomOutTime;

    UPROPERTY(EditAnywhere, Category = "FireMode")
    USoundBase* ZoomInSound;

    UPROPERTY(EditAnywhere, Category = "FireMode")
    USoundBase* ZoomOutSound;

    UPROPERTY(EditAnywhere, Category = "FireMode")
    bool bHide1PMeshes;

public:

    virtual void RequestStartFire_Implementation() override;
    virtual void Deactivate() override;

    virtual bool IsIndependentFireMode_Implementation()
    {
        return true;
    }

// ActivatableInterface
protected:
    virtual void AIF_InternalActivate_Implementation() override;
    virtual void AIF_InternalDeactivate_Implementation() override;

protected:

    UPROPERTY()
    UUserWidget* ZoomWidget;

    template<class T> T* GetInstigator();

    // Animation

    float AnimAlpha;
    float AnimTarget;
    float AnimDur;

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION()
    void UpdateZoom();

    // Replication

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(ReplicatedUsing = OnRep_bZooming)
    bool bZooming;

    UFUNCTION(Server, Reliable)
    void ServerSetZoomState(bool bNewZooming);

    UFUNCTION()
    virtual void OnRep_bZooming();

};
