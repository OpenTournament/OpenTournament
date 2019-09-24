// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_LocalPlayer.h"

#include "UR_MessageHistory.h"


UUR_LocalPlayer::UUR_LocalPlayer(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MessageHistory = ObjectInitializer.CreateDefaultSubobject<UUR_MessageHistory>(this, TEXT("MessageHistory"));
}
