// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UR_Widget_ChatBox.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Chat Box Widget
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Widget_ChatBox : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;

public:

	/**
	* Function bound to "BeginSay" ActionMapping input.
	* Blueprint implementable.
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void OnBeginSay();

	/**
	* Function bound to "BeginTeamSay" ActionMapping input.
	* Blueprint implementable.
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void OnBeginTeamSay();
};
