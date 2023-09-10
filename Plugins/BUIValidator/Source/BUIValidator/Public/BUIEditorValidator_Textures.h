#pragma once

#include "EditorValidatorBase.h"
#include "BUIEditorValidator_Textures.generated.h"

UCLASS( meta = ( DisplayName = "BUI Texture Validator" ) )
class BUIVALIDATOR_API UBUIEditorValidator_Textures : public UEditorValidatorBase
{
	GENERATED_BODY()

public:
	UBUIEditorValidator_Textures();

protected:
	virtual bool CanValidateAsset_Implementation(UObject* InAsset) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(UObject* InAsset, TArray<FText>& ValidationErrors) override;
};
