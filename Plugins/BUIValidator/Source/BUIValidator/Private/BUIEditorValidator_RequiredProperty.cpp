#include "BUIEditorValidator_RequiredProperty.h"
#include <Engine/Texture2D.h>
#include <Editor/EditorPerProjectUserSettings.h>
#include <EditorFramework/AssetImportData.h>
#include "BUIValidatorSettings.h"
#include <UObject/UnrealType.h>

#define LOCTEXT_NAMESPACE "BUIEditorValidator"

const FName UBUIEditorValidator_RequiredProperty::PropertyName = "BUIRequired";

UBUIEditorValidator_RequiredProperty::UBUIEditorValidator_RequiredProperty()
	: Super()
{
	bIsEnabled = true;
}

bool UBUIEditorValidator_RequiredProperty::CanValidateAsset_Implementation( UObject* InAsset ) const
{
	UClass* AssetClass = nullptr;
	UBlueprint* Bp = Cast<UBlueprint>( InAsset );
	if ( Bp && Bp->ParentClass )
	{
		AssetClass = Bp->ParentClass;
	}
	else
	{
		AssetClass = InAsset->GetClass();
	}
	for ( TFieldIterator<FProperty> PropertyIterator( AssetClass ); PropertyIterator; ++PropertyIterator )
	{
		if ( PropertyIterator->GetBoolMetaData( PropertyName ) )
		{
			return true;
		}
	}
	return false;
}

EDataValidationResult UBUIEditorValidator_RequiredProperty::ValidateLoadedAsset_Implementation( UObject* InAsset, TArray<FText>& ValidationErrors )
{
	bool bAnyFailed = false;
	bool bAnyChecked = false;

	UClass* AssetClass = nullptr;
	UBlueprint* Bp = Cast<UBlueprint>( InAsset );
	if ( Bp && Bp->ParentClass )
	{
		AssetClass = Bp->GeneratedClass;
	}
	else
	{
		AssetClass = InAsset->GetClass();
	}

	UObject* MyCDO = AssetClass->GetDefaultObject( true );
	for ( TFieldIterator<FProperty> PropertyIterator( AssetClass ); PropertyIterator; ++PropertyIterator )
	{
		if ( PropertyIterator->GetBoolMetaData( PropertyName ) )
		{
			bAnyChecked = true;
			FObjectPropertyBase* ObjProp = CastField<FObjectPropertyBase>( *PropertyIterator );
			if ( ObjProp )
			{
				UObject* Something = ObjProp->GetObjectPropertyValue_InContainer( MyCDO );

				if ( Something == nullptr )
				{
					bAnyFailed = true;
					AssetFails( InAsset, FText::FormatNamed(
						LOCTEXT( "BUIValidatorError_NotSet", "Property '{PropertyName}' is not set. All variables marked with '{BUIMetaName}' must be set to non-null" ),
						"PropertyName", ObjProp->GetDisplayNameText(),
						"BUIMetaName", FText::FromName( PropertyName ) ),
						ValidationErrors );
				}
			}
		}
	}

	if ( !bAnyChecked )
		return EDataValidationResult::NotValidated;

	if ( !bAnyFailed )
	{
		AssetPasses( InAsset );
	}
	return bAnyFailed ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}

#undef LOCTEXT_NAMESPACE
