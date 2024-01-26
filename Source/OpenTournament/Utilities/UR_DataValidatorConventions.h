// Copyright (c) 2019-2023 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"

#include "UR_DataValidatorConventions.generated.h"


/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FAssetNamingConvention
{
    GENERATED_BODY()

    FAssetNamingConvention()
        : ClassName(FString(TEXT("")))
        , AssetPrefix(FString(TEXT("")))
        , AssetSuffix(FString(TEXT("")))
    {}

    FAssetNamingConvention(const FString& InClassName, const FString& InPrefix, const FString& InSuffix)
        : ClassName(InClassName)
        , AssetPrefix(InPrefix)
        , AssetSuffix(InSuffix)
    {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClassName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssetPrefix;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssetSuffix;

    bool operator==(const FAssetNamingConvention& Right) const
    {
        return (ClassName.Equals(Right.ClassName) && AssetPrefix.Equals(Right.AssetPrefix) && AssetSuffix.Equals(Right.AssetSuffix));
    }
};

/**
* This class validates assets meet naming conventions:
* 1. All assets are named using AlphaNumeric characters, plus "-" or "_" symbols
* 2. All assets follow naming conventions outlined in AssetValidation.ini
*    e.g. Texture2D assets begin with "T_", etc.
*/
UCLASS(Config = "AssetValidation")
class OPENTOURNAMENT_API UUR_DataValidatorConventions : public UEditorValidatorBase
{
    GENERATED_BODY()

public:
    UUR_DataValidatorConventions();

    bool CanValidate_Implementation(const EDataValidationUsecase InUsecase) const override;

    bool CanValidateAsset_Implementation(UObject* InAsset) const override;

    EDataValidationResult ValidateLoadedAsset_Implementation(UObject* InAsset, UPARAM(ref) TArray<FText>& ValidationErrors) override;

    // Returns true if the EDataValidationResult is Invalid
    UFUNCTION()
    bool IsInvalid() const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    bool bValidateAssetNames;

    // Configurable data structure of AssetClass to PrefixConvention
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config)
    TArray<FAssetNamingConvention> NamingConventions;
};
