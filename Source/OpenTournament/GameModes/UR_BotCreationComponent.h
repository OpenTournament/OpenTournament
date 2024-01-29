// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/GameStateComponent.h"

#include "UR_BotCreationComponent.generated.h"

class UUR_ExperienceDefinition;
class AAIController;

UCLASS(Blueprintable, Abstract)
class UUR_BotCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UUR_BotCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void BeginPlay() override;
	//~End of UActorComponent interface

private:
	void OnExperienceLoaded(const UUR_ExperienceDefinition* Experience);

protected:
	UPROPERTY(EditDefaultsOnly, Category=Teams)
	int32 NumBotsToCreate = 5;

	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TSubclassOf<AAIController> BotControllerClass;

	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<FString> RandomBotNames;

	TArray<FString> RemainingBotNames;

protected:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AAIController>> SpawnedBotList;

#if WITH_SERVER_CODE
public:
	void Cheat_AddBot() { SpawnOneBot(); }
	void Cheat_RemoveBot() { RemoveOneBot(); }

protected:
	virtual void ServerCreateBots();

	virtual void SpawnOneBot();
	virtual void RemoveOneBot();

	FString CreateBotName(int32 PlayerIndex);
#endif
};
