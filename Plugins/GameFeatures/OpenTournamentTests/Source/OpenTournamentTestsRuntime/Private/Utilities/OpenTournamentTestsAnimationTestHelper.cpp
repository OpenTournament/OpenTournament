// Copyright Epic Games, Inc. All Rights Reserved.

#include "OpenTournamentTestsAnimationTestHelper.h"

#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimMontage.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/UObjectIterator.h"

namespace
{

bool IsExpectedAnimationPlaying(const UAnimationAsset* ExpectedAnimation, const TArray<FAnimTickRecord>& Records)
{
	for (int32 PlayerIndex = 0; PlayerIndex < Records.Num(); ++PlayerIndex)
	{
		const FAnimTickRecord& TickRecord = Records[PlayerIndex];
		if (TickRecord.SourceAsset == ExpectedAnimation)
		{
			return true;
		}
	}

	return false;
}

} //anonymous

UAnimationAsset* FOpenTournamentTestsAnimationTestHelper::FindAnimationAsset(USkeletalMeshComponent* SkeletalMeshComponent, const FString& AnimationName)
{
	check(SkeletalMeshComponent);

	if (const USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->GetSkeletalMeshAsset())
	{
		if (const USkeleton* Skeleton = SkeletalMesh->GetSkeleton())
		{
			for (TObjectIterator<UAnimationAsset> Itr; Itr; ++Itr)
			{
				UAnimationAsset* AnimationAsset = (*Itr);
				if (!IsValid(AnimationAsset))
				{
					continue;
				}

				if (Skeleton == AnimationAsset->GetSkeleton() && AnimationAsset->GetName().Equals(AnimationName))
				{
					return AnimationAsset;
				}
			}
		}
	}

	return nullptr;
}

bool FOpenTournamentTestsAnimationTestHelper::IsAnimationPlaying(USkeletalMeshComponent* SkeletalMeshComponent, const UAnimationAsset* ExpectedAnimation)
{
	check(SkeletalMeshComponent);

	bool bIsAnimationPlaying = false;
	SkeletalMeshComponent->ForEachAnimInstance([&ExpectedAnimation, &bIsAnimationPlaying](UAnimInstance* AnimInstance)
		{
			// Early out if we found our animation from a prior AnimInstance
			if (bIsAnimationPlaying)
			{
				return;
			}

			FAnimMontageInstance* AnimMontageInstance = AnimInstance->GetActiveMontageInstance();
			if (AnimMontageInstance && AnimMontageInstance->IsPlaying() && IsValid(AnimMontageInstance->Montage))
			{
				bIsAnimationPlaying = AnimMontageInstance->Montage == ExpectedAnimation;
			}
			else
			{
				const FAnimInstanceProxy::FSyncGroupMap& SyncGroupMap = AnimInstance->GetSyncGroupMapRead();
				const TArray<FAnimTickRecord>& UngroupedActivePlayers = AnimInstance->GetUngroupedActivePlayersRead();
				for (const auto& SyncGroupPair : SyncGroupMap)
				{
					const FAnimGroupInstance& SyncGroup = SyncGroupPair.Value;
					if (SyncGroup.ActivePlayers.Num() > 0)
					{
						if (SyncGroup.GroupLeaderIndex != -1)
						{
							bIsAnimationPlaying = IsExpectedAnimationPlaying(ExpectedAnimation, SyncGroup.ActivePlayers);
							return;
						}
					}
				}

				bIsAnimationPlaying = IsExpectedAnimationPlaying(ExpectedAnimation, UngroupedActivePlayers);
			}
		});

	return bIsAnimationPlaying;
}
