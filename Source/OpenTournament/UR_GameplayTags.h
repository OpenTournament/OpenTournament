// Copyright (c) 2019-2023 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Countdown);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Match);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Overtime);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Warmup);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Jumping);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Dodging);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Crouching);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Running);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Walking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Falling);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Swimming);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Flying);
