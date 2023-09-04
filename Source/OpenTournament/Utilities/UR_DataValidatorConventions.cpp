// Copyright (c) 2019-2023 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_DataValidatorConventions.h"

#include "Internationalization/Regex.h"

#define LOCTEXT_NAMESPACE "AssetNamingConventions"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_DataValidatorConventions::UUR_DataValidatorConventions()
    : Super()
{
    // Noop
}

bool UUR_DataValidatorConventions::CanValidate_Implementation(const EDataValidationUsecase InUsecase) const
{
    return Super::CanValidate_Implementation(InUsecase);
}

bool UUR_DataValidatorConventions::CanValidateAsset_Implementation(UObject* InAsset) const
{
    return true;
}

EDataValidationResult UUR_DataValidatorConventions::ValidateLoadedAsset_Implementation(UObject* InAsset, TArray<FText>& ValidationErrors)
{
    if (InAsset)
    {
        if (const auto AssetName = InAsset->GetName(); !AssetName.IsEmpty())
        {
            // Force asset names to only use Alphanumerics, Numbers, and - or _
            if (!IsInvalid() && bValidateAssetNames)
            {
                const FRegexPattern Pattern{ FRegexPattern(TEXT("^[A-Za-z0-9_-]+$")) };
                FRegexMatcher Matcher{ FRegexMatcher(Pattern, AssetName) };
                if (!Matcher.FindNext())
                {
                    AssetFails(InAsset, FText::Format(LOCTEXT("IllegalCharacter_AssetName", "Illegal character present in asset {0}"), FText::FromString(AssetName)), ValidationErrors);
                }
            }

            // Ensure Asset NamingConventions
            if (!IsInvalid())
            {
                for (const auto& IterConvention : NamingConventions)
                {
                    // If the Asset class name matches our class...
                    if (IterConvention.ClassName.Equals(InAsset->GetClass()->GetName(), ESearchCase::IgnoreCase))
                    {
                        if (!IterConvention.AssetPrefix.IsEmpty()
                            && !AssetName.StartsWith(IterConvention.AssetPrefix))
                        {
                            AssetFails(InAsset, FText::Format(LOCTEXT("IllegalCharacter_AssetName", "Illegal naming convention for asset {0} of type {1}, expected name to begin with \"{2}\". Please follow project naming conventions!"), FText::FromString(AssetName), FText::FromString(IterConvention.ClassName), FText::FromString(IterConvention.AssetPrefix)), ValidationErrors);
                        }

                        if (!IterConvention.AssetSuffix.IsEmpty()
                            && !AssetName.EndsWith(IterConvention.AssetSuffix))
                        {
                            AssetFails(InAsset, FText::Format(LOCTEXT("IllegalCharacter_AssetName", "Illegal naming convention for asset {0} of type {1}, expected name to end with \"{2}\". Please follow project naming conventions!"), FText::FromString(AssetName), FText::FromString(IterConvention.ClassName), FText::FromString(IterConvention.AssetSuffix)), ValidationErrors);
                        }
                    }
                }
            }
        }
    }

    return GetValidationResult();
}

bool UUR_DataValidatorConventions::IsInvalid() const
{
    return GetValidationResult() == EDataValidationResult::Invalid;
}

#undef LOCTEXT_NAMESPACE
