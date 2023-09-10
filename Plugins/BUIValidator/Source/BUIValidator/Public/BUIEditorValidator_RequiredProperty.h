#pragma once

#include "EditorValidatorBase.h"
#include "BUIEditorValidator_RequiredProperty.generated.h"

UCLASS( meta = ( DisplayName = "BUI Required Property Validator" ) )
class BUIVALIDATOR_API UBUIEditorValidator_RequiredProperty : public UEditorValidatorBase
{
	GENERATED_BODY()

public:
	UBUIEditorValidator_RequiredProperty();

protected:
	static const FName PropertyName;
	virtual bool CanValidateAsset_Implementation( UObject* InAsset ) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation( UObject* InAsset, TArray<FText>& ValidationErrors ) override;
};
