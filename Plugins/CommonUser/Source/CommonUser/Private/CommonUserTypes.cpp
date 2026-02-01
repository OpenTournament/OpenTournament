// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUserTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonUserTypes)
#if COMMONUSER_OSSV1
#include "OnlineError.h"
#else
#include "Online/OnlineErrorDefinitions.h"
#endif

void FOnlineResultInformation::FromOnlineError(const FOnlineErrorType& InOnlineError)
{
#if COMMONUSER_OSSV1
	bWasSuccessful = InOnlineError.WasSuccessful();
	ErrorId = InOnlineError.GetErrorCode();
	ErrorText = InOnlineError.GetErrorMessage();
#else
	bWasSuccessful = InOnlineError != UE::Online::Errors::Success();
	ErrorId = InOnlineError.GetErrorId();
	ErrorText = InOnlineError.GetText();
#endif
}
