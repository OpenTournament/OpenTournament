// Copyright Brace Yourself Games. All Rights Reserved.

#include "BUIValidateAllCommandlet.h"

#include "BUIValidatorModule.h"
#include "BUIValidatorSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"

DEFINE_LOG_CATEGORY(LogCommandletPlugin);

UBUIValidateAllCommandlet::UBUIValidateAllCommandlet()
{
	HelpDescription = "Runs existing validation rules on all assets";
	HelpWebLink = "https://github.com/benui-dev/UE4-BUIValidator";
	
	IsClient = false;
	IsEditor = false;
	IsServer = false;
	LogToConsole = true;
}

int32 UBUIValidateAllCommandlet::Main(const FString& Params)
{
#if 0
	UE_LOG(LogCommandletPlugin, Display, TEXT("Hello world!"));

	FBUIValidatorModule& ValidatorModule = FModuleManager::LoadModuleChecked<FBUIValidatorModule>("BUIValidator");
	//AssetRegistryModule.Get().SearchAllAssets(true);
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);
	AssetRegistryModule.Get().SearchAllAssets(true);

	// Load paths from settings
	const UBUIValidatorSettings& ValidatorSettings = *GetDefault<UBUIValidatorSettings>();
	for ( const auto& Group : ValidatorSettings.ValidationGroups )
	{
		if ( Group.bRunInCommandlet
			&& Group.ShouldGroupValidateAsset()
		{
			if ( Group.ValidationRule.TextureGroups.Num() > 0 )
			{
				Texture->LODGroup = Group.ValidationRule.TextureGroups[ 0 ];
			}

			if ( Group.ValidationRule.CompressionSettings.Num() > 0 )
			{
				Texture->CompressionSettings = Group.ValidationRule.CompressionSettings[ 0 ];
			}

			if ( Group.ValidationRule.MipGenSettings.Num() > 0 )
			{
				Texture->MipGenSettings = Group.ValidationRule.MipGenSettings[ 0 ];
			}
		}
	}


	FARFilter Filter;
	Filter.ClassNames.Add(UTexture::StaticClass()->GetFName());

	AssetRegistryModule.Get().EnumerateAssets(Filter, [FoundAssets = &AssetData, InInterfaceClass](const FAssetData& InAssetData)
	{
		if (ImplementsInterface(InAssetData, InInterfaceClass))
		{
			FoundAssets->Add(InAssetData);
		}

		return true;
	});

#endif
	return 0;
}