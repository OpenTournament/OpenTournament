#include "BUIValidatorModule.h"
#include "BUIValidatorSettings.h"
#include <ISettingsModule.h>
#include <ISettingsSection.h>
#include <ISettingsContainer.h>
#include <Subsystems/ImportSubsystem.h>
#include <Factories/TextureFactory.h>

#define LOCTEXT_NAMESPACE "FBUIValidatorModule"

void FBUIValidatorModule::StartupModule()
{
	if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer( "Project" );
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings( "Project", "Plugins", "BUI Validator",
			LOCTEXT( "RuntimeGeneralSettingsName", "BUI Validator" ),
			LOCTEXT( "RuntimeGeneralSettingsDescription", "Configure UI data asset validation." ),
			GetMutableDefault<UBUIValidatorSettings>()
		);

		if ( SettingsSection.IsValid() )
		{
			SettingsSection->OnModified().BindRaw( this, &FBUIValidatorModule::HandleSettingsSaved );
		}
	}

	FCoreDelegates::OnPostEngineInit.AddRaw( this, &FBUIValidatorModule::OnPostEngineInit );
}

void FBUIValidatorModule::ShutdownModule()
{
	if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
	{
		SettingsModule->UnregisterSettings( "Project", "CustomSettings", "General" );
	}

	if ( GIsEditor )
	{
		if ( GEditor && GEditor->GetEditorSubsystem<UImportSubsystem>() )
			GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.RemoveAll( this );
	}
}

void FBUIValidatorModule::OnPostEngineInit()
{
	if ( GIsEditor )
	{
		GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.AddRaw( this, &FBUIValidatorModule::OnObjectReimported );
	}
}

void FBUIValidatorModule::OnObjectReimported( UFactory* ImportFactory, UObject* InObject )
{
	UTexture2D* Texture = Cast<UTexture2D>( InObject );
	if ( !Texture )
		return;

	// Only apply defaults to newly-imported assets
	UTextureFactory* TextureFactory = Cast<UTextureFactory>( ImportFactory );
	if ( !TextureFactory->bUsingExistingSettings )
	{
		const UBUIValidatorSettings& ValidatorSettings = *GetDefault<UBUIValidatorSettings>();
		for ( const auto& Group : ValidatorSettings.ValidationGroups )
		{
			if ( Group.bApplyOnImport
				&& Group.ShouldGroupValidateAsset( InObject ) )
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
	}
}

bool FBUIValidatorModule::HandleSettingsSaved()
{
	UBUIValidatorSettings* Settings = GetMutableDefault<UBUIValidatorSettings>();
	bool ResaveSettings = false;

	// You can put any validation code in here and resave the settings in case an invalid
	// value has been entered

	if ( ResaveSettings )
	{
		Settings->SaveConfig();
	}

	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FBUIValidatorModule, BUIValidator )

