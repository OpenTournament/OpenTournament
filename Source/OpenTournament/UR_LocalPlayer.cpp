// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.


#include "UR_LocalPlayer.h"

#include "UR_MessageHistory.h"


UUR_LocalPlayer::UUR_LocalPlayer(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MessageHistory = ObjectInitializer.CreateDefaultSubobject<UUR_MessageHistory>(this, TEXT("MessageHistory"));
}
