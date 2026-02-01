// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "NativeGameplayTags.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace URGameplayTags
{
    UE_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Countdown);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Match);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Overtime);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_MatchState_InProgress_Warmup);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Jumping);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Dodging);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Crouching);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Movement_Running);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Walking);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Falling);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Swimming);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_States_Physics_Flying);

    //

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Networking);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Behavior_SurvivesDeath);

    //

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_GodMode);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_UnlimitedHealth);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Heal);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Reset);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_RequestReset);

    //

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_AutoRun);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Dash);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Dash_Axis);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Jump);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Drop);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Fire);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Fire_1);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Fire_2);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Fire_3);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Reload);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Switch);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Switch_Next);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Switch_Previous);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Switch_Select);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_Throw);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

    //

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Crouching);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_AutoRunning);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);

    //

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crosshair_Enable);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crosshair_Disable);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crosshair_HitRegister);

    //

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationChord_2x);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationChord_3x);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationChord_4x);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationChord_5x);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationChord_6x);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationStreak_5);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationStreak_10);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationStreak_15);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationStreak_20);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationStreak_25);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationStreak_30);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Announcement_Accolade_EliminationStreak_Ended);

    //

    // These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
    UE_API extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
    UE_API extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);
};

#undef UE_API
