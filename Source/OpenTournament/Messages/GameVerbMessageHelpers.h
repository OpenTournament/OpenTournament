// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameVerbMessageHelpers.generated.h"

struct FGameplayCueParameters;
struct FGameVerbMessage;

class APlayerController;
class APlayerState;
class UObject;
struct FFrame;


UCLASS()
class OPENTOURNAMENT_API UGameVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "OT")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "OT")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "OT")
	static FGameplayCueParameters VerbMessageToCueParameters(const FGameVerbMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "OT")
	static FGameVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
};
