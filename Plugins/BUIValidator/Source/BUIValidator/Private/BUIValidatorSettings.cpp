#include "BUIValidatorSettings.h"
#include <Engine/Texture2D.h>

#define LOCTEXT_NAMESPACE "BUIEditorValidator"

bool UBUIValidatorSettings::ShouldValidateAsset( UObject* InAsset ) const
{
	UTexture2D* Texture = Cast<UTexture2D>( InAsset );
	if ( Texture )
	{
		const UBUIValidatorSettings& ValidatorSettings = *GetDefault<UBUIValidatorSettings>();
		for ( const auto& Group : ValidatorSettings.ValidationGroups )
		{
			if ( Group.ShouldGroupValidateAsset( InAsset ) )
				return true;
		}
	}

	return false;
}

bool FBUIValidatorGroup::ShouldGroupValidateAsset( UObject* InAsset ) const
{
	UTexture2D* Texture = Cast<UTexture2D>( InAsset );
	if ( !Texture )
		return false;

	const FString AssetPathInUnreal = Texture->GetPathName();

	bool bMatchAnyTextureGroup = MatchConditions.TextureGroups.Num() == 0
		|| MatchConditions.TextureGroups.Contains( Texture->LODGroup );

	bool bMatchAnyPath = MatchConditions.Paths.Num() == 0;
	for ( const auto& Path : MatchConditions.Paths )
	{
		if ( AssetPathInUnreal.StartsWith( Path.Path ) )
		{
			bMatchAnyPath = true;
			break;
		}
	}

	bool bMatchAnyPrefix = MatchConditions.Prefixes.Num() == 0;
	for ( const auto& Prefix : MatchConditions.Prefixes )
	{
		if ( FPaths::GetCleanFilename( AssetPathInUnreal ).StartsWith( Prefix ) )
		{
			bMatchAnyPrefix = true;
			break;
		}
	}

	// Let's apply rules to this texture
	return bMatchAnyTextureGroup && bMatchAnyPath && bMatchAnyPrefix;
}

#undef LOCTEXT_NAMESPACE
