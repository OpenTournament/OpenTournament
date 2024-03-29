// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayTags.h"

#include <GameplayTagsManager.h>
#include <Engine/EngineTypes.h>

#include "UR_LogChannels.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace URGameplayTags
{
    UE_DEFINE_GAMEPLAY_TAG(TAG_MatchState_InProgress_Countdown ,    "MatchState.InProgress.Countdown");
    UE_DEFINE_GAMEPLAY_TAG(TAG_MatchState_InProgress_Match,         "MatchState.InProgress.Match");
    UE_DEFINE_GAMEPLAY_TAG(TAG_MatchState_InProgress_Overtime,      "MatchState.InProgress.Overtime");
    UE_DEFINE_GAMEPLAY_TAG(TAG_MatchState_InProgress_Warmup,        "MatchState.InProgress.Warmup");

    UE_DEFINE_GAMEPLAY_TAG(TAG_Character_States_Movement_Jumping,   "Character.States.Movement.Jumping");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Character_States_Movement_Dodging,   "Character.States.Movement.Dodging");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Character_States_Movement_Crouching, "Character.States.Movement.Crouching");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Character_States_Movement_Running,   "Character.States.Movement.Running");

    UE_DEFINE_GAMEPLAY_TAG(TAG_Character_States_Physics_Walking,  "Character.States.Movement.Walking");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Character_States_Physics_Falling,  "Character.States.Movement.Falling");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Character_States_Physics_Swimming, "Character.States.Movement.Swimming");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Character_States_Physics_Flying,   "Character.States.Movement.Flying");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_GodMode, "Cheat.GodMode", "GodMode cheat is active on the owner.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_UnlimitedHealth, "Cheat.UnlimitedHealth", "UnlimitedHealth cheat is active on the owner.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Crouching, "Status.Crouching", "Target is crouching.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_AutoRunning, "Status.AutoRunning", "Target is auto-running.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");

	// These are mapped to the movement modes inside GetMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming, "Movement.Mode.Swimming", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying, "Movement.Mode.Flying", "Default Character movement tag");

	// For custom movement modes we need to update GetCustomMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom, "Movement.Mode.Custom", "This is invalid and should be replaced with custom tags.  See URGameplayTags::CustomMovementModeTagMap.");

	// Unreal Movement Modes
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_NavWalking, Movement_Mode_NavWalking },
		{ MOVE_Falling, Movement_Mode_Falling },
		{ MOVE_Swimming, Movement_Mode_Swimming },
		{ MOVE_Flying, Movement_Mode_Flying },
		{ MOVE_Custom, Movement_Mode_Custom }
	};

	// Custom Movement Modes
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		// Fill these in with custom modes
	};

	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogGame, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}
