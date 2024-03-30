// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "NativeGameplayTags.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace URGameplayTags
{
    OPENTOURNAMENT_API	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Countdown);
    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Match);
    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Overtime);
    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Warmup);

    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Jumping);
    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Dodging);
    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Crouching);
    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Running);

    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Walking);
    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Falling);
    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Swimming);
    OPENTOURNAMENT_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Flying);

    //

    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_GodMode);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_UnlimitedHealth);

    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Heal);

    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);

    //

    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Crouching);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_AutoRunning);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);

    // These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
    OPENTOURNAMENT_API	extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
    OPENTOURNAMENT_API	extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

    OPENTOURNAMENT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);

};
