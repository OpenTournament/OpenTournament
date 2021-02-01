// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_LocalPlayer.h"

#include "UR_MessageHistory.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_LocalPlayer::UUR_LocalPlayer(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    // no need to create a history for the CDO
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        MessageHistory = CreateDefaultSubobject<UUR_MessageHistory>(TEXT("MessageHistory"));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
