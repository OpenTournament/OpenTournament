// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModularPlayerController.h"

#include "CommonPlayerController.generated.h"

#define UE_API COMMONGAME_API

class APawn;
class UObject;

UCLASS(MinimalAPI, config=Game)
class ACommonPlayerController : public AModularPlayerController
{
	GENERATED_BODY()

public:
	UE_API ACommonPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void ReceivedPlayer() override;	
	UE_API virtual void SetPawn(APawn* InPawn) override;
	UE_API virtual void OnPossess(class APawn* APawn) override;
	UE_API virtual void OnUnPossess() override;
	
protected:
	UE_API virtual void OnRep_PlayerState() override;
};

#undef UE_API
