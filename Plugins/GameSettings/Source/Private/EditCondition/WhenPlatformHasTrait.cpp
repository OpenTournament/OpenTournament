// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditCondition/WhenPlatformHasTrait.h"

#include "CommonUIVisibilitySubsystem.h"

#define LOCTEXT_NAMESPACE "GameSetting"

TSharedRef<FWhenPlatformHasTrait> FWhenPlatformHasTrait::KillIfMissing(FGameplayTag InVisibilityTag, const FString& InKillReason)
{
	check(InVisibilityTag.IsValid());
	check(!InKillReason.IsEmpty());

	TSharedRef<FWhenPlatformHasTrait> Result = MakeShared<FWhenPlatformHasTrait>();
	Result->VisibilityTag = InVisibilityTag;
	Result->KillReason = InKillReason;
	Result->bTagDesired = true;

	return Result;
}

TSharedRef<FWhenPlatformHasTrait> FWhenPlatformHasTrait::DisableIfMissing(FGameplayTag InVisibilityTag, const FText& InDisableReason)
{
	check(InVisibilityTag.IsValid());
	check(!InDisableReason.IsEmpty());

	TSharedRef<FWhenPlatformHasTrait> Result = MakeShared<FWhenPlatformHasTrait>();
	Result->VisibilityTag = InVisibilityTag;
	Result->DisableReason = InDisableReason;
	Result->bTagDesired = true;

	return Result;
}

TSharedRef<FWhenPlatformHasTrait> FWhenPlatformHasTrait::KillIfPresent(FGameplayTag InVisibilityTag, const FString& InKillReason)
{
	check(InVisibilityTag.IsValid());
	check(!InKillReason.IsEmpty());

	TSharedRef<FWhenPlatformHasTrait> Result = MakeShared<FWhenPlatformHasTrait>();
	Result->VisibilityTag = InVisibilityTag;
	Result->KillReason = InKillReason;
	Result->bTagDesired = false;

	return Result;
}

TSharedRef<FWhenPlatformHasTrait> FWhenPlatformHasTrait::DisableIfPresent(FGameplayTag InVisibilityTag, const FText& InDisableReason)
{
	check(InVisibilityTag.IsValid());
	check(!InDisableReason.IsEmpty());

	TSharedRef<FWhenPlatformHasTrait> Result = MakeShared<FWhenPlatformHasTrait>();
	Result->VisibilityTag = InVisibilityTag;
	Result->DisableReason = InDisableReason;
	Result->bTagDesired = false;

	return Result;
}

void FWhenPlatformHasTrait::GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const
{
	if (UCommonUIVisibilitySubsystem::GetChecked(InLocalPlayer)->HasVisibilityTag(VisibilityTag) != bTagDesired)
	{
		if (KillReason.IsEmpty())
		{
			InOutEditState.Disable(DisableReason);
		}
		else
		{
			InOutEditState.Kill(KillReason);
		}
	}
}

#undef LOCTEXT_NAMESPACE
