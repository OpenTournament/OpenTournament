// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Containers/Ticker.h"
#include "GameUIManagerSubsystem.h"

#include "UR_UIManagerSubsystem.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class FSubsystemCollectionBase;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_UIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:

	UUR_UIManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	bool Tick(float DeltaTime);
	void SyncRootLayoutVisibilityToShowHUD();

	FTSTicker::FDelegateHandle TickHandle;
};
