// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_LocalPlayer.h"


UUR_LocalPlayer::UUR_LocalPlayer(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ChatHistory.Reserve(CHAT_HISTORY_MAX);
}
