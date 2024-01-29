// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "BUIValidateAllCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCommandletPlugin, Log, All);

UCLASS()
class UBUIValidateAllCommandlet : public UCommandlet
{
	GENERATED_BODY()
public:
	UBUIValidateAllCommandlet();

	// Begin UCommandlet interface
	virtual int32 Main(const FString& Params) override;
	// End UCommandlet interface
};
