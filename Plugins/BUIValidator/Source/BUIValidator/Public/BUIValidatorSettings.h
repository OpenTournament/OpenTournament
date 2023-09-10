#pragma once

#include "BUIValidatorSettings.generated.h"

UENUM()
enum class EBUITextureSizeRequirement
{
	MultipleOfFour,
	PowerOfTwo,
};

USTRUCT( meta = ( ToolTip = "All parts of a rule must pass in order for the rule to be applied" ) )
struct FBUIMatchConditions
{
	GENERATED_BODY()
public:
	// Match UTexture2D assets with any of these texture groups
	UPROPERTY( config, EditAnywhere, Category = Custom )
	TArray<TEnumAsByte<TextureGroup>> TextureGroups = { TextureGroup::TEXTUREGROUP_UI };

	// Match UTexture2D assets with any of these prefixes
	UPROPERTY( config, EditAnywhere, Category = Custom ) 
	TArray<FString> Prefixes = { "T_UI_" };

	// Match UTexture2D assets under any of these directories
	UPROPERTY( EditAnywhere, meta = ( ContentDir, TitleProperty = "Path" ) )
	TArray<FDirectoryPath> Paths;
};

USTRUCT()
struct FBUIValidationRule
{
	GENERATED_BODY()
	FBUIValidationRule()
	{
		// Defaults
		if ( Paths.Num() == 0 )
		{
			Paths.Add( FDirectoryPath() );
			Paths[ 0 ].Path = "/Game/UI";
		}
	}
public:

	// Textures must be in one of these Texture Groups
	UPROPERTY( config, EditAnywhere )
	TArray<TEnumAsByte<TextureGroup>> TextureGroups = { TextureGroup::TEXTUREGROUP_UI };

	// Textures must have one of these Compression Settings
	UPROPERTY( config, EditAnywhere )
	TArray<TEnumAsByte<TextureCompressionSettings>> CompressionSettings = { TextureCompressionSettings::TC_Default };

	// Textures must have one of these Pixel Formats
	UPROPERTY( config, EditAnywhere )
	TArray<TEnumAsByte<EPixelFormat>> PixelFormats = { EPixelFormat::PF_DXT5 };

	// Textures must have one of these mip gen settings
	UPROPERTY( config, EditAnywhere )
	TArray<TEnumAsByte<TextureMipGenSettings>> MipGenSettings = { TextureMipGenSettings::TMGS_FromTextureGroup };

	// Textures must have one of these prefixes. Is not applied on import
	UPROPERTY( config, EditAnywhere )
	TArray<FString> Prefixes = { "T_UI_" };

	// Textures must pass these size requirements. Is not applied on import
	UPROPERTY( config, EditAnywhere )
	TSet<EBUITextureSizeRequirement> TextureSizeRequirements = { EBUITextureSizeRequirement::MultipleOfFour };

	// Textures must be within this path in Unreal. Is not applied on import
	UPROPERTY( EditAnywhere, meta = ( ContentDir ) )
	TArray<FDirectoryPath> Paths;

	// Require that the Data Source Folder be set in Editor Preferences, and that assets are imported from there
	UPROPERTY( EditAnywhere )
	bool bRequireDataSourceFolder = true;
};

USTRUCT()
struct FBUIValidatorGroup
{
	GENERATED_BODY()
public:
	UPROPERTY( config, EditAnywhere )
	FString GroupName;

	// Apply this group's rules to newly-imported assets
	UPROPERTY( config, EditAnywhere )
	bool bApplyOnImport = false;
	
	// Apply this group's rules when running BUIValidateAll from the commandlet
	UPROPERTY( config, EditAnywhere )
	bool bRunInCommandlet = true;

	UPROPERTY( config, EditAnywhere, Category = "Validation", meta = ( ShowOnlyInnerProperties ) )
	FBUIMatchConditions MatchConditions;

	UPROPERTY( config, EditAnywhere, Category = "Validation", meta = ( ShowOnlyInnerProperties ) )
	FBUIValidationRule ValidationRule;

	bool ShouldGroupValidateAsset( UObject* InAsset ) const;
};

UCLASS( config = Game, defaultconfig ) //, AutoExpandCategories = "Validation" )
class UBUIValidatorSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY( config, EditAnywhere, Category = "Validation", meta = ( TitleProperty = "GroupName", ShowOnlyInnerProperties ) )
	TArray<FBUIValidatorGroup> ValidationGroups;

	bool ShouldValidateAsset( UObject* InAsset ) const;
};
