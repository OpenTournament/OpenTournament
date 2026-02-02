// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Animation/AnimationAsset.h"

class UAnimationAsset;
class USkeletalMeshComponent;

/// Class to work with animations within a SkeletalMeshComponent
class FOpenTournamentTestsAnimationTestHelper
{
public:
	// Movement animations with the Pistol equipped for Manny
	inline static const FString MannyPistolJogForwardAnimationName = TEXT("MM_Pistol_Jog_Fwd");
	inline static const FString MannyPistolJogBackwardAnimationName = TEXT("MM_Pistol_Jog_Bwd");
	inline static const FString MannyPistolStrafeLeftAnimationName = TEXT("MM_Pistol_Jog_Left");
	inline static const FString MannyPistolStrafeRightAnimationName = TEXT("MM_Pistol_Jog_Right");

	// Movement animations with the Pistol equipped for Quinn
	inline static const FString QuinnPistolJogForwardAnimationName = TEXT("MF_Pistol_Jog_Fwd");
	inline static const FString QuinnPistolJogBackwardAnimationName = TEXT("MF_Pistol_Jog_Bwd");
	inline static const FString QuinnPistolStrafeLeftAnimationName = TEXT("MF_Pistol_Jog_Left");
	inline static const FString QuinnPistolStrafeRightAnimationName = TEXT("MF_Pistol_Jog_Right");

	// Crouching animations with the Pistol equipped
	inline static const FString PistolCrouchIdleAnimationName = TEXT("MM_Pistol_Crouch_Idle");
	inline static const FString PistolCrouchWalkForwardAnimationName = TEXT("MM_Pistol_Crouch_Walk_Fwd");
	inline static const FString PistolCrouchWalkBackwardAnimationName = TEXT("MM_Pistol_Crouch_Walk_Bwd");
	inline static const FString PistolCrouchStrafeLeftAnimationName = TEXT("MM_Pistol_Crouch_Walk_Left");
	inline static const FString PistolCrouchStrafeRightAnimationName = TEXT("MM_Pistol_Crouch_Walk_Right");
	inline static const FString PistolCrouchRotateLeftAnimationName = TEXT("MM_Pistol_Crouch_TurnLeft_90");
	inline static const FString PistolCrouchRotateRightAnimationName = TEXT("MM_Pistol_Crouch_TurnRight_90");

	// Jumping animations with the Pistol equipped
	inline static const FString PistolJumpAnimationName = TEXT("MM_Pistol_Jump_Apex");

	// Equipped weapon melee animations
	inline static const FString PistolMeleeAnimationName = TEXT("AM_MM_Pistol_Melee");
	inline static const FString RifleMeleeAnimationName = TEXT("AM_MM_Rifle_Melee");
	inline static const FString ShotgunMeleeAnimationName = TEXT("AM_MM_Shotgun_Melee");

	/**
	 * Find an animation asset within a SkeletalMesh by name.
	 *
	 * @param SkeletalMeshComponent - SkeletalMeshComponent to be searched against.
	 * @param AnimationName - Name of the UAnimationAsset to find.
	 *
	 * @return Animation asset reference if found, otherwise nullptr.
	 */
	UAnimationAsset* FindAnimationAsset(USkeletalMeshComponent* SkeletalMeshComponent, const FString& AnimationName);

	/**
	 * Check if an animation asset is currently playing.
	 *
	 * @param SkeletalMeshComponent - SkeletalMeshComponent to be checked against.
	 * @param ExpectedAnimation - Expected animation asset.
	 *
	 * @return true if animation asset is playing, otherwise false.
	 */
	bool IsAnimationPlaying(USkeletalMeshComponent* SkeletalMeshComponent, const UAnimationAsset* ExpectedAnimation);
};
